/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIMAP/ImapSet>
#include <KIMAP/Session>
#include <KJob>
#include <KMime/Message>

/**
 * This job appends a message, marks the old one as deleted, and returns the uid of the appended message.
 */
class ReplaceMessageJob : public KJob
{
    Q_OBJECT
public:
    ReplaceMessageJob(const KMime::Message::Ptr &msg,
                      KIMAP::Session *session,
                      const QString &mailbox,
                      qint64 uidNext = -1,
                      const KIMAP::ImapSet &oldUids = KIMAP::ImapSet(),
                      QObject *parent = nullptr);

    qint64 newUid() const;

    void start() override;

private:
    void triggerSearchJob();
    void triggerDeleteJobIfNecessary();

private Q_SLOTS:
    void onAppendMessageDone(KJob *job);
    void onSelectDone(KJob *job);
    void onSearchDone(KJob *job);
    void onDeleteDone(KJob *job);

private:
    KIMAP::Session *mSession = nullptr;
    const KMime::Message::Ptr mMessage;
    const QString mMailbox;
    qint64 mUidNext;
    KIMAP::ImapSet mOldUids;
    qint64 mNewUid = -1;
    const QByteArray mMessageId;
};
