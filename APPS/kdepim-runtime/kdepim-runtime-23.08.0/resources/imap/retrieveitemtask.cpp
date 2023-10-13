/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrieveitemtask.h"
#include "messagehelper.h"

#include "imapresource_debug.h"

#include <KLocalizedString>

#include <Akonadi/MessageFlags>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>

RetrieveItemTask::RetrieveItemTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
{
}

RetrieveItemTask::~RetrieveItemTask() = default;

void RetrieveItemTask::doStart(KIMAP::Session *session)
{
    m_session = session;

    const QString mailBox = mailBoxForCollection(item().parentCollection());
    m_uid = item().remoteId().toLongLong();

    if (m_uid <= 0) {
        qCWarning(IMAPRESOURCE_LOG) << "Remote id is " << item().remoteId();
        cancelTask(i18n("Remote id is empty or invalid"));
        return;
    }

    if (session->selectedMailBox() != mailBox) {
        auto select = new KIMAP::SelectJob(m_session);
        select->setMailBox(mailBox);
        connect(select, &KJob::result, this, &RetrieveItemTask::onSelectDone);
        select->start();
    } else {
        triggerFetchJob();
    }
}

void RetrieveItemTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else {
        triggerFetchJob();
    }
}

void RetrieveItemTask::triggerFetchJob()
{
    auto fetch = new KIMAP::FetchJob(m_session);
    KIMAP::FetchJob::FetchScope scope;
    fetch->setUidBased(true);
    fetch->setSequenceSet(KIMAP::ImapSet(m_uid));
    scope.parts.clear(); // = parts.toList();
    scope.mode = KIMAP::FetchJob::FetchScope::Content;
    fetch->setScope(scope);
    connect(fetch, &KIMAP::FetchJob::messagesAvailable, this, &RetrieveItemTask::onMessagesReceived);
    connect(fetch, &KJob::result, this, &RetrieveItemTask::onContentFetchDone);
    fetch->start();
}

void RetrieveItemTask::onMessagesReceived(const QMap<qint64, KIMAP::Message> &messages)
{
    auto fetch = qobject_cast<KIMAP::FetchJob *>(sender());
    Q_ASSERT(fetch != nullptr);
    if (messages.size() == 0) {
        qCDebug(IMAPRESOURCE_LOG) << "Requested message does not exist on the server anymore";
        cancelTask(i18n("No message retrieved, failed to read the message."));
        return;
    }

    Akonadi::Item i = item();

    qCDebug(IMAPRESOURCE_LOG) << "MESSAGE from Imap server" << item().remoteId();
    Q_ASSERT(item().isValid());

    const auto message = messages.cbegin();
    const qint64 number = message->uid;
    bool ok;
    const Akonadi::Item remoteItem =
        resourceState()->messageHelper()->createItemFromMessage(message->message, number, 0, message->attributes, {}, fetch->scope(), ok);
    if (!ok) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to retrieve message " << number;
        cancelTask(i18n("No message retrieved, failed to read the message."));
        return;
    }
    i.setMimeType(remoteItem.mimeType());
    i.setPayload(remoteItem.payload<KMime::Message::Ptr>());
    const auto flags{remoteItem.flags()};
    for (const QByteArray &flag : flags) {
        i.setFlag(flag);
    }

    qCDebug(IMAPRESOURCE_LOG) << "Has Payload: " << i.hasPayload();

    m_messageReceived = true;
    itemRetrieved(i);
}

void RetrieveItemTask::onContentFetchDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else if (!m_messageReceived) {
        cancelTask(i18n("No message retrieved, server reply was empty."));
    }
}
