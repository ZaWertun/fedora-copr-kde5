/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIMAP/FetchJob>
#include <KJob>
#include <KMime/Message>

struct Merger {
    virtual ~Merger() = default;

    virtual KMime::Message::Ptr merge(const KMime::Message::Ptr &newMessage, const QList<KMime::Message::Ptr> &conflictingMessages) const = 0;
};

/**
 * This job appends a message, marks the old one as deleted, and returns the uid of the appended message.
 */
class UpdateMessageJob : public KJob
{
    Q_OBJECT
public:
    UpdateMessageJob(const KMime::Message::Ptr &msg,
                     KIMAP::Session *session,
                     const QByteArray &kolabUid,
                     const QSharedPointer<Merger> &merger,
                     const QString &mailbox,
                     qint64 uidNext = -1,
                     qint64 oldUid = -1,
                     QObject *parent = nullptr);

    qint64 newUid() const;

    void start() override;

private:
    void fetchHeaders();
    void searchForLatestVersion();
    void appendMessage();

private Q_SLOTS:
    void onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages);
    void onHeadersFetchDone(KJob *job);
    void onSelectDone(KJob *job);
    void onSearchDone(KJob *job);
    void onConflictingMessagesReceived(const QMap<qint64, KIMAP::Message> &message);
    void onConflictingMessageFetchDone(KJob *job);
    void onReplaceDone(KJob *job);

private:
    KIMAP::Session *mSession = nullptr;
    KMime::Message::Ptr mMessage;
    const QString mMailbox;
    qint64 mUidNext;
    qint64 mOldUid;
    KIMAP::ImapSet mOldUids;
    qint64 mNewUid = -1;
    const QByteArray mMessageId;
    const QByteArray mKolabUid;
    QList<qint64> mFoundUids;
    QList<KIMAP::MessagePtr> mMessagesToMerge;
    QSharedPointer<Merger> mMerger;
};
