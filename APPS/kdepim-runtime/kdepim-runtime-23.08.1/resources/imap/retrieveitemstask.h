/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIMAP/FetchJob>

#include "resourcetask.h"
#include <QElapsedTimer>
class BatchFetcher;
namespace Akonadi
{
class Session;
}

class RetrieveItemsTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit RetrieveItemsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~RetrieveItemsTask() override;
    void setFetchMissingItemBodies(bool enabled);

public Q_SLOTS:
    void onFetchItemsWithoutBodiesDone(const QVector<qint64> &items);
    void onReadyForNextBatch(int size);

private Q_SLOTS:
    void fetchItemsWithoutBodiesDone(KJob *job);
    void onPreExpungeSelectDone(KJob *job);
    void onExpungeDone(KJob *job);
    void onFinalSelectDone(KJob *job);
    void onStatusDone(KJob *job);
    void onItemsRetrieved(const Akonadi::Item::List &addedItems);
    void onRetrievalDone(KJob *job);
    void onFlagsFetchDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

    virtual BatchFetcher *createBatchFetcher(MessageHelper::Ptr messageHelper,
                                             const KIMAP::ImapSet &set,
                                             const KIMAP::FetchJob::FetchScope &scope,
                                             int batchSize,
                                             KIMAP::Session *session);

private:
    void prepareRetrieval();
    void startRetrievalTasks();
    void triggerPreExpungeSelect(const QString &mailBox);
    void triggerExpunge(const QString &mailBox);
    void triggerFinalSelect(const QString &mailBox);
    void retrieveItems(const KIMAP::ImapSet &set, const KIMAP::FetchJob::FetchScope &scope, bool incremental = false, bool uidBased = false);
    void listFlagsForImapSet(const KIMAP::ImapSet &set);
    void taskComplete();

    KIMAP::Session *m_session = nullptr;
    QVector<qint64> m_messageUidsMissingBody;
    int m_fetchedMissingBodies = -1;
    bool m_fetchMissingBodies = false;
    bool m_incremental = true;
    qint64 m_localHighestModSeq = -1;
    BatchFetcher *m_batchFetcher = nullptr;
    Akonadi::Collection m_modifiedCollection;
    bool m_uidBasedFetch = true;
    bool m_flagsChanged = false;
    QElapsedTimer m_time;

    // Results of SELECT
    QString m_mailBox;
    int m_messageCount = -1;
    int m_uidValidity = -1;
    qint64 m_nextUid = -1;
    qint64 m_highestModSeq = -1;
    QList<QByteArray> m_flags;
};
