/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "additemtask.h"

#include <QUuid>

#include "imapresource_debug.h"
#include <KLocalizedString>

#include <KIMAP/AppendJob>
#include <KIMAP/ImapSet>
#include <KIMAP/SearchJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>

#include <KMime/Message>

#include "uidnextattribute.h"

AddItemTask::AddItemTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

AddItemTask::~AddItemTask() = default;

void AddItemTask::doStart(KIMAP::Session *session)
{
    if (!item().hasPayload<KMime::Message::Ptr>()) {
        changeProcessed();
        return;
    }

    const QString mailBox = mailBoxForCollection(collection());
    if (mailBox.isEmpty()) {
        qCWarning(IMAPRESOURCE_LOG) << "Trying to append message to invalid mailbox, this will fail. Id: " << parentCollection().id();
    }

    qCDebug(IMAPRESOURCE_LOG) << "Got notification about item added for local id " << item().id() << " and remote id " << item().remoteId();

    // save message to the server.
    auto msg = item().payload<KMime::Message::Ptr>();
    m_messageId = msg->messageID()->asUnicodeString().toUtf8();

    auto job = new KIMAP::AppendJob(session);
    job->setMailBox(mailBox);
    job->setContent(msg->encodedContent(true));
    job->setFlags(fromAkonadiToSupportedImapFlags(item().flags().values(), collection()));
    job->setInternalDate(msg->date()->dateTime());
    connect(job, &KIMAP::AppendJob::result, this, &AddItemTask::onAppendMessageDone);
    job->start();
}

void AddItemTask::onAppendMessageDone(KJob *job)
{
    auto append = qobject_cast<KIMAP::AppendJob *>(job);

    if (append->error()) {
        qCWarning(IMAPRESOURCE_LOG) << append->errorString();
        cancelTask(append->errorString());
        return;
    }

    const qint64 uid = append->uid();

    if (uid > 0) {
        // We got it directly if UIDPLUS is supported...
        applyFoundUid(uid);
    } else {
        // ... otherwise prepare searching for the message
        KIMAP::Session *session = append->session();
        const QString mailBox = append->mailBox();

        if (session->selectedMailBox() != mailBox) {
            auto select = new KIMAP::SelectJob(session);
            select->setMailBox(mailBox);

            connect(select, &KJob::result, this, &AddItemTask::onPreSearchSelectDone);

            select->start();
        } else {
            triggerSearchJob(session);
        }
    }
}

void AddItemTask::onPreSearchSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto select = static_cast<KIMAP::SelectJob *>(job);
        triggerSearchJob(select->session());
    }
}

void AddItemTask::triggerSearchJob(KIMAP::Session *session)
{
    auto search = new KIMAP::SearchJob(session);

    search->setUidBased(true);

    if (!m_messageId.isEmpty()) {
        search->setTerm(KIMAP::Term(QStringLiteral("Message-ID"), QString::fromLatin1(m_messageId)));
    } else {
        Akonadi::Collection c = collection();
        auto uidNext = c.attribute<UidNextAttribute>();
        if (!uidNext) {
            cancelTask(i18n("Could not determine the UID for the newly created message on the server"));
            search->deleteLater();
            return;
        }
        search->setTerm(KIMAP::Term(KIMAP::Term::And, {KIMAP::Term(KIMAP::Term::New), KIMAP::Term(KIMAP::Term::Uid, KIMAP::ImapSet(uidNext->uidNext(), 0))}));
    }

    connect(search, &KJob::result, this, &AddItemTask::onSearchDone);

    search->start();
}

void AddItemTask::onSearchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    auto search = static_cast<KIMAP::SearchJob *>(job);

    qint64 uid = 0;
    if (search->results().count() == 1) {
        uid = search->results().at(0);
    }

    applyFoundUid(uid);
}

void AddItemTask::applyFoundUid(qint64 uid)
{
    Akonadi::Item i = item();

    // if we didn't manage to get a valid UID from the server, use a random RID instead
    // this will make ItemSync clean up the mess during the next sync (while empty RIDs are protected as not yet existing on the server)
    if (uid > 0) {
        i.setRemoteId(QString::number(uid));
    } else {
        i.setRemoteId(QUuid::createUuid().toString());
    }
    qCDebug(IMAPRESOURCE_LOG) << "Setting remote ID to " << i.remoteId() << " for item with local id " << i.id();

    changeCommitted(i);

    Akonadi::Collection c = collection();

    // Get the current uid next value and store it
    UidNextAttribute *uidAttr = nullptr;
    int oldNextUid = 0;
    if (c.hasAttribute("uidnext")) {
        uidAttr = static_cast<UidNextAttribute *>(c.attribute("uidnext"));
        oldNextUid = uidAttr->uidNext();
    }

    // If the uid we just got back is the expected next one of the box
    // then update the property to the probable next uid to keep the cache in sync.
    // If not something happened in our back, so we don't update and a refetch will
    // happen at some point.
    if (uid == oldNextUid) {
        if (uidAttr == nullptr) {
            uidAttr = new UidNextAttribute(uid + 1);
            c.addAttribute(uidAttr);
        } else {
            uidAttr->setUidNext(uid + 1);
        }

        applyCollectionChanges(c);
    }
}
