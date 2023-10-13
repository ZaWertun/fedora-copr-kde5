/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "expungecollectiontask.h"

#include "imapresource_debug.h"

#include <KIMAP/ExpungeJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>

#include "noselectattribute.h"

ExpungeCollectionTask::ExpungeCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
{
}

ExpungeCollectionTask::~ExpungeCollectionTask() = default;

void ExpungeCollectionTask::doStart(KIMAP::Session *session)
{
    // Prevent expunging items from noselect folders.
    if (collection().hasAttribute("noselect")) {
        NoSelectAttribute *noselect = static_cast<NoSelectAttribute *>(collection().attribute("noselect"));
        if (noselect->noSelect()) {
            qCDebug(IMAPRESOURCE_LOG) << "No Select folder";
            taskDone();
            return;
        }
    }

    const QString mailBox = mailBoxForCollection(collection());

    if (session->selectedMailBox() != mailBox) {
        auto select = new KIMAP::SelectJob(session);
        select->setMailBox(mailBox);

        connect(select, &KIMAP::SelectJob::result, this, &ExpungeCollectionTask::onSelectDone);

        select->start();
    } else {
        triggerExpungeJob(session);
    }
}

void ExpungeCollectionTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else {
        auto select = static_cast<KIMAP::SelectJob *>(job);
        if (select->isOpenReadOnly()) {
            qCDebug(IMAPRESOURCE_LOG) << "Mailbox is opened readonly, not expunging";
            taskDone();
        } else {
            triggerExpungeJob(select->session());
        }
    }
}

void ExpungeCollectionTask::triggerExpungeJob(KIMAP::Session *session)
{
    auto expunge = new KIMAP::ExpungeJob(session);

    connect(expunge, &KIMAP::ExpungeJob::result, this, &ExpungeCollectionTask::onExpungeDone);

    expunge->start();
}

void ExpungeCollectionTask::onExpungeDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else {
        taskDone();
    }
}
