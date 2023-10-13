/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "movecollectiontask.h"

#include <KLocalizedString>

#include <KIMAP/RenameJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KIMAP/SubscribeJob>

#include <QUuid>

MoveCollectionTask::MoveCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

MoveCollectionTask::~MoveCollectionTask() = default;

void MoveCollectionTask::doStart(KIMAP::Session *session)
{
    if (collection().remoteId().isEmpty()) {
        emitError(i18n("Cannot move IMAP folder '%1', it does not exist on the server.", collection().name()));
        changeProcessed();
        return;
    }

    if (sourceCollection().remoteId().isEmpty()) {
        emitError(i18n("Cannot move IMAP folder '%1' out of '%2', '%2' does not exist on the server.", collection().name(), sourceCollection().name()));
        changeProcessed();
        return;
    }

    if (targetCollection().remoteId().isEmpty()) {
        emitError(i18n("Cannot move IMAP folder '%1' to '%2', '%2' does not exist on the server.", collection().name(), sourceCollection().name()));
        changeProcessed();
        return;
    }

    if (session->selectedMailBox() != mailBoxForCollection(collection())) {
        doRename(session);
        return;
    }

    // Some IMAP servers don't allow moving an opened mailbox, so make sure
    // it's not opened (https://bugs.kde.org/show_bug.cgi?id=324932) by examining
    // a non-existent mailbox. We don't use CLOSE in order not to trigger EXPUNGE
    auto examine = new KIMAP::SelectJob(session);
    examine->setOpenReadOnly(true); // use EXAMINE instead of SELECT
    examine->setMailBox(QStringLiteral("IMAP Resource non existing folder %1").arg(QUuid::createUuid().toString()));
    connect(examine, &KIMAP::SelectJob::result, this, &MoveCollectionTask::onExamineDone);
    examine->start();
}

void MoveCollectionTask::onExamineDone(KJob *job)
{
    // We deliberately ignore any error here, because the SelectJob will always fail
    // when examining a non-existent mailbox

    auto examine = static_cast<KIMAP::SelectJob *>(job);
    doRename(examine->session());
}

QString MoveCollectionTask::mailBoxForCollections(const Akonadi::Collection &parent, const Akonadi::Collection &child) const
{
    const QString parentMailbox = mailBoxForCollection(parent);
    if (parentMailbox.isEmpty()) {
        return child.remoteId().mid(1); // Strip separator on toplevel mailboxes
    }
    return parentMailbox + child.remoteId();
}

void MoveCollectionTask::doRename(KIMAP::Session *session)
{
    // collection.remoteId() already includes the separator
    const QString oldMailBox = mailBoxForCollections(sourceCollection(), collection());
    const QString newMailBox = mailBoxForCollections(targetCollection(), collection());

    if (oldMailBox != newMailBox) {
        auto job = new KIMAP::RenameJob(session);
        job->setSourceMailBox(oldMailBox);
        job->setDestinationMailBox(newMailBox);

        connect(job, &KIMAP::RenameJob::result, this, &MoveCollectionTask::onRenameDone);

        job->start();
    } else {
        changeProcessed();
    }
}

void MoveCollectionTask::onRenameDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else {
        // Automatically subscribe to the new mailbox name
        auto rename = static_cast<KIMAP::RenameJob *>(job);

        auto subscribe = new KIMAP::SubscribeJob(rename->session());
        subscribe->setMailBox(rename->destinationMailBox());

        connect(subscribe, &KIMAP::SubscribeJob::result, this, &MoveCollectionTask::onSubscribeDone);

        subscribe->start();
    }
}

void MoveCollectionTask::onSubscribeDone(KJob *job)
{
    if (job->error()) {
        emitWarning(
            i18n("Failed to subscribe to the folder '%1' on the IMAP server. "
                 "It will disappear on next sync. Use the subscription dialog to overcome that",
                 collection().name()));
    }

    changeCommitted(collection());
}
