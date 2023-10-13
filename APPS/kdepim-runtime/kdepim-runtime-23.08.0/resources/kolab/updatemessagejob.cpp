/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "updatemessagejob.h"

#include "kolabresource_debug.h"
#include <KIMAP/ImapSet>
#include <KIMAP/SearchJob>
#include <KIMAP/SelectJob>
#include <replacemessagejob.h>

#include "imapflags.h"

// Check if the expected uid message is still there => no modification, replace message.
// otherwise search for up-to-date message by subject containing UID, merge contents, and replace message

UpdateMessageJob::UpdateMessageJob(const KMime::Message::Ptr &msg,
                                   KIMAP::Session *session,
                                   const QByteArray &kolabUid,
                                   const QSharedPointer<Merger> &merger,
                                   const QString &mailbox,
                                   qint64 uidNext,
                                   qint64 oldUid,
                                   QObject *parent)
    : KJob(parent)
    , mSession(session)
    , mMessage(msg)
    , mMailbox(mailbox)
    , mUidNext(uidNext)
    , mOldUid(oldUid)
    , mMessageId(msg->messageID()->asUnicodeString().toUtf8())
    , mKolabUid(kolabUid)
    , mMerger(merger)
{
    mOldUids.add(oldUid);
}

void UpdateMessageJob::start()
{
    if (mSession->selectedMailBox() != mMailbox) {
        auto select = new KIMAP::SelectJob(mSession);
        select->setMailBox(mMailbox);
        connect(select, &KIMAP::SelectJob::result, this, [this](KJob *job) {
            onSelectDone(job);
        });
        select->start();
    } else {
        fetchHeaders();
    }
}

void UpdateMessageJob::onSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
    } else {
        fetchHeaders();
    }
}

void UpdateMessageJob::fetchHeaders()
{
    auto fetchJob = new KIMAP::FetchJob(mSession);

    fetchJob->setSequenceSet(KIMAP::ImapSet(mOldUid));
    fetchJob->setUidBased(true);

    KIMAP::FetchJob::FetchScope scope;
    scope.parts.clear();
    scope.mode = KIMAP::FetchJob::FetchScope::Headers;
    fetchJob->setScope(scope);
    connect(fetchJob, &KIMAP::FetchJob::messagesAvailable, this, &UpdateMessageJob::onMessagesAvailable);
    connect(fetchJob, &KJob::result, this, &UpdateMessageJob::onHeadersFetchDone);
    fetchJob->start();
}

void UpdateMessageJob::onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages)
{
    // Filter deleted messages
    for (auto it = messages.cbegin(), end = messages.cend(); it != end; ++it) {
        // const KMime::Message::Ptr msg = messages[number];
        if (!it->flags.contains(ImapFlags::Deleted)) {
            mFoundUids << it->uid;
        }
    }
}

void UpdateMessageJob::onHeadersFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to fetch message: " << job->errorString();
    }
    if (mFoundUids.size() >= 1) {
        qCDebug(KOLABRESOURCE_LOG) << "Fast-forward update, replacing message.";
        appendMessage();
    } else {
        searchForLatestVersion();
    }
}

void UpdateMessageJob::searchForLatestVersion()
{
    auto search = new KIMAP::SearchJob(mSession);
    search->setUidBased(true);
    search->setTerm(KIMAP::Term(KIMAP::Term::Subject, QString::fromLatin1(mKolabUid)));
    connect(search, &KJob::result, this, &UpdateMessageJob::onSearchDone);
    search->start();
}

void UpdateMessageJob::onSearchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    auto search = static_cast<KIMAP::SearchJob *>(job);

    if (search->results().count() >= 1) {
        mOldUids = KIMAP::ImapSet();
        const auto results = search->results();
        for (qint64 id : results) {
            mOldUids.add(id);
        }

        auto fetchJob = new KIMAP::FetchJob(mSession);
        fetchJob->setSequenceSet(mOldUids);
        fetchJob->setUidBased(true);

        KIMAP::FetchJob::FetchScope scope;
        scope.parts.clear();
        scope.mode = KIMAP::FetchJob::FetchScope::Full;
        fetchJob->setScope(scope);
        connect(fetchJob, &KIMAP::FetchJob::messagesAvailable, this, &UpdateMessageJob::onConflictingMessagesReceived);
        connect(fetchJob, &KJob::result, this, &UpdateMessageJob::onConflictingMessageFetchDone);
        fetchJob->start();
    } else {
        qCWarning(KOLABRESOURCE_LOG) << "failed to find latest version of object";
        appendMessage();
    }
}

void UpdateMessageJob::onConflictingMessagesReceived(const QMap<qint64, KIMAP::Message> &messages)
{
    for (auto it = messages.cbegin(), end = messages.cend(); it != end; ++it) {
        if (!it->flags.contains(ImapFlags::Deleted)) {
            mMessagesToMerge << it->message;
        }
    }
}

void UpdateMessageJob::onConflictingMessageFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to retrieve messages to merge: " << job->errorString();
    }
    mMessage = mMerger->merge(mMessage, mMessagesToMerge);
    appendMessage();
}

void UpdateMessageJob::appendMessage()
{
    const qint64 uidNext = -1;
    auto replace = new ReplaceMessageJob(mMessage, mSession, mMailbox, uidNext, mOldUids, this);
    connect(replace, &KJob::result, this, &UpdateMessageJob::onReplaceDone);
    replace->start();
}

void UpdateMessageJob::onReplaceDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    auto replaceJob = static_cast<ReplaceMessageJob *>(job);
    mNewUid = replaceJob->newUid();
    emitResult();
}

qint64 UpdateMessageJob::newUid() const
{
    return mNewUid;
}
