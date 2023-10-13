/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ItemFetchJob>

#include "ewsfinditemrequest.h"
#include "ewsjob.h"

namespace Akonadi
{
class Collection;
}
class EwsClient;
class EwsTagStore;
class EwsResource;

class EwsFetchItemsJob : public EwsJob
{
    Q_OBJECT
public:
    struct QueuedUpdate {
        QString id;
        QString changeKey;
        EwsEventType type;
    };

    using QueuedUpdateList = QList<QueuedUpdate>;

    EwsFetchItemsJob(const Akonadi::Collection &collection,
                     EwsClient &client,
                     const QString &syncState,
                     const EwsId::List &itemsToCheck,
                     EwsTagStore *tagStore,
                     EwsResource *parent);
    ~EwsFetchItemsJob() override;

    Akonadi::Item::List newItems() const
    {
        return mNewItems;
    }

    Akonadi::Item::List changedItems() const
    {
        return mChangedItems;
    }

    Akonadi::Item::List deletedItems() const
    {
        return mDeletedItems;
    }

    const QString &syncState() const
    {
        return mSyncState;
    }

    const Akonadi::Collection &collection() const
    {
        return mCollection;
    }

    void setQueuedUpdates(const QueuedUpdateList &updates);

    void start() override;
private Q_SLOTS:
    void localItemFetchDone(KJob *job);
    void remoteItemFetchDone(KJob *job);
    void itemDetailFetchDone(KJob *job);
    void checkedItemsFetchFinished(KJob *job);
    void tagSyncFinished(KJob *job);
Q_SIGNALS:
    void reportStatus(int status, const QString &message = QString());
    void reportPercent(ulong progress);

private:
    void compareItemLists();
    void syncTags();
    bool processIncrementalRemoteItemUpdates(const EwsItem::List &items,
                                             QHash<QString, Akonadi::Item> &itemHash,
                                             QHash<EwsItemType, Akonadi::Item::List> &toFetchItems);

    /*struct QueuedUpdateInt {
        QString changeKey;
        EwsEventType type;
    };*/
    using QueuedUpdateHash = QHash<EwsEventType, QHash<QString, QString>>;

    const Akonadi::Collection mCollection;
    EwsClient &mClient;
    EwsId::List mItemsToCheck;
    Akonadi::Item::List mLocalItems;
    EwsItem::List mRemoteAddedItems;
    EwsItem::List mRemoteChangedItems;
    EwsId::List mRemoteDeletedIds;
    QHash<EwsId, bool> mRemoteFlagChangedIds;
    int mPendingJobs;
    unsigned mTotalItemsToFetch;
    unsigned mTotalItemsFetched;
    QString mSyncState;
    bool mFullSync;
    QueuedUpdateHash mQueuedUpdates;
    EwsTagStore *mTagStore = nullptr;
    bool mTagsSynced;

    Akonadi::Item::List mNewItems;
    Akonadi::Item::List mChangedItems;
    Akonadi::Item::List mDeletedItems;
};
