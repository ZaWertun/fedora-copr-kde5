/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "removecollectionrecursivetask.h"

#include "imapresource_debug.h"
#include <Akonadi/MessageFlags>
#include <KIMAP/CloseJob>
#include <KIMAP/DeleteJob>
#include <KIMAP/SelectJob>
#include <KIMAP/StoreJob>
#include <KLocalizedString>

Q_DECLARE_METATYPE(KIMAP::DeleteJob *)

RemoveCollectionRecursiveTask::RemoveCollectionRecursiveTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

RemoveCollectionRecursiveTask::~RemoveCollectionRecursiveTask() = default;

void RemoveCollectionRecursiveTask::doStart(KIMAP::Session *session)
{
    mSession = session;

    mFolderFound = false;
    auto listJob = new KIMAP::ListJob(session);
    listJob->setIncludeUnsubscribed(!isSubscriptionEnabled());
    listJob->setQueriedNamespaces(serverNamespaces());
    connect(listJob, &KIMAP::ListJob::mailBoxesReceived, this, &RemoveCollectionRecursiveTask::onMailBoxesReceived);
    connect(listJob, &KIMAP::ListJob::result, this, &RemoveCollectionRecursiveTask::onJobDone);
    listJob->start();
}

void RemoveCollectionRecursiveTask::onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &)
{
    const QString mailBox = mailBoxForCollection(collection());

    // We have to delete the deepest-nested folders first, so
    // we use a map here that has the level of nesting as key.
    QMultiMap<int, KIMAP::MailBoxDescriptor> foldersToDelete;

    for (int i = 0, total = descriptors.size(); i < total; ++i) {
        const KIMAP::MailBoxDescriptor descriptor = descriptors[i];

        if (descriptor.name == mailBox || descriptor.name.startsWith(mailBox + descriptor.separator)) { // a sub folder to delete
            const QStringList pathParts = descriptor.name.split(descriptor.separator);
            foldersToDelete.insert(pathParts.count(), descriptor);
        }
    }

    if (foldersToDelete.isEmpty()) {
        return;
    }

    mFolderFound = true;

    // Now start the actual deletion work
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mFolderIterator.reset(new QMapIterator<int, KIMAP::MailBoxDescriptor>(foldersToDelete));
#else
    mFolderIterator.reset(new QMultiMapIterator<int, KIMAP::MailBoxDescriptor>(foldersToDelete));
#endif
    mFolderIterator->toBack(); // we start with largest nesting value first

    deleteNextMailbox();
}

void RemoveCollectionRecursiveTask::deleteNextMailbox()
{
    if (!mFolderIterator->hasPrevious()) {
        changeProcessed(); // finish the job
        return;
    }

    mFolderIterator->previous();
    const KIMAP::MailBoxDescriptor &descriptor = mFolderIterator->value();
    qCDebug(IMAPRESOURCE_LOG) << descriptor.name;

    // first select the mailbox
    auto selectJob = new KIMAP::SelectJob(mSession);
    selectJob->setMailBox(descriptor.name);
    connect(selectJob, &KIMAP::SelectJob::result, this, &RemoveCollectionRecursiveTask::onJobDone);
    selectJob->start();

    // mark all items as deleted
    // This step shouldn't be required, but apparently some servers don't allow deleting, non empty mailboxes (although they should).
    KIMAP::ImapSet allItems;
    allItems.add(KIMAP::ImapInterval(1, 0)); // means 1:*
    auto storeJob = new KIMAP::StoreJob(mSession);
    storeJob->setSequenceSet(allItems);
    storeJob->setFlags(KIMAP::MessageFlags() << Akonadi::MessageFlags::Deleted);
    storeJob->setMode(KIMAP::StoreJob::AppendFlags);
    // The result is explicitly ignored, since this can fail in the case of an empty folder
    storeJob->start();

    // Some IMAP servers don't allow deleting an opened mailbox, so make sure
    // it's not opened (https://bugs.kde.org/show_bug.cgi?id=324932). CLOSE will
    // also trigger EXPUNGE to take care of the messages deleted above
    auto closeJob = new KIMAP::CloseJob(mSession);
    closeJob->setProperty("folderDescriptor", descriptor.name);
    connect(closeJob, &KIMAP::CloseJob::result, this, &RemoveCollectionRecursiveTask::onCloseJobDone);
    closeJob->start();
}

void RemoveCollectionRecursiveTask::onCloseJobDone(KJob *job)
{
    if (job->error()) {
        changeProcessed();
        qCDebug(IMAPRESOURCE_LOG) << "Failed to close the folder, resync the folder tree";
        emitWarning(i18n("Failed to delete the folder, restoring folder list."));
        synchronizeCollectionTree();
    } else {
        auto deleteJob = new KIMAP::DeleteJob(mSession);
        deleteJob->setMailBox(job->property("folderDescriptor").toString());
        connect(deleteJob, &KIMAP::DeleteJob::result, this, &RemoveCollectionRecursiveTask::onDeleteJobDone);
        deleteJob->start();
    }
}

void RemoveCollectionRecursiveTask::onDeleteJobDone(KJob *job)
{
    if (job->error()) {
        changeProcessed();

        qCDebug(IMAPRESOURCE_LOG) << "Failed to delete the folder, resync the folder tree";
        emitWarning(i18n("Failed to delete the folder, restoring folder list."));
        synchronizeCollectionTree();
    } else {
        deleteNextMailbox();
    }
}

void RemoveCollectionRecursiveTask::onJobDone(KJob *job)
{
    if (job->error()) {
        changeProcessed();

        qCDebug(IMAPRESOURCE_LOG) << "Failed to delete the folder, resync the folder tree";
        emitWarning(i18n("Failed to delete the folder, restoring folder list."));
        synchronizeCollectionTree();
    } else if (!mFolderFound) {
        changeProcessed();
        qCDebug(IMAPRESOURCE_LOG) << "Failed to find the folder to be deleted, resync the folder tree";
        emitWarning(i18n("Failed to find the folder to be deleted, restoring folder list."));
        synchronizeCollectionTree();
    }
}
