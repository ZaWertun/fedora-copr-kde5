/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabremovetagtask.h"
#include "kolabresource_debug.h"
#include "kolabresource_trace.h"
#include <imapflags.h>

#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KIMAP/StoreJob>

KolabRemoveTagTask::KolabRemoveTagTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : KolabRelationResourceTask(resource, parent)
{
}

void KolabRemoveTagTask::startRelationTask(KIMAP::Session *session)
{
    // The imap specs do not allow for a single message to be deleted. We can only
    // set the \Deleted flag. The message will actually be deleted when EXPUNGE will
    // be issued on the next retrieveItems().

    const QString mailBox = mailBoxForCollection(relationCollection());

    qCDebug(KOLABRESOURCE_LOG) << "Deleting tag " << resourceState()->tag().name() << " from " << mailBox;

    if (session->selectedMailBox() != mailBox) {
        auto select = new KIMAP::SelectJob(session);
        select->setMailBox(mailBox);

        connect(select, &KJob::result, this, &KolabRemoveTagTask::onSelectDone);

        select->start();
    } else {
        triggerStoreJob(session);
    }
}

void KolabRemoveTagTask::triggerStoreJob(KIMAP::Session *session)
{
    KIMAP::ImapSet set;
    set.add(resourceState()->tag().remoteId().toLong());
    qCDebug(KOLABRESOURCE_TRACE) << set.toImapSequenceSet();

    auto store = new KIMAP::StoreJob(session);
    store->setUidBased(true);
    store->setSequenceSet(set);
    store->setFlags(QList<QByteArray>() << ImapFlags::Deleted);
    store->setMode(KIMAP::StoreJob::AppendFlags);
    connect(store, &KJob::result, this, &KolabRemoveTagTask::onStoreFlagsDone);
    store->start();
}

void KolabRemoveTagTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to select mailbox: " << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto select = static_cast<KIMAP::SelectJob *>(job);
        triggerStoreJob(select->session());
    }
}

void KolabRemoveTagTask::onStoreFlagsDone(KJob *job)
{
    qCDebug(KOLABRESOURCE_TRACE);
    // TODO use UID EXPUNGE if available
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to append flags: " << job->errorString();
        cancelTask(job->errorString());
    } else {
        changeProcessed();
    }
}
