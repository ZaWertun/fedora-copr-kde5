/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchitemsjob.h"

#include <KLocalizedString>

#include <Akonadi/AgentBase>
#include <Akonadi/ItemFetchScope>

#include "ewsclient.h"
#include "ewsfetchitemdetailjob.h"
#include "ewsitemhandler.h"
#include "ewsresource.h"
#include "ewsresource_debug.h"
#include "ewssyncfolderitemsrequest.h"
#include "tags/ewsakonaditagssyncjob.h"
#include "tags/ewstagstore.h"

using namespace Akonadi;

static constexpr int listBatchSize = 100;
static constexpr int fetchBatchSize = 50;

/**
 * The fetch items job is processed in two stages.
 *
 * The first stage is to query the list of messages on the remote and local sides. For this purpose
 * an EwsSyncFolderItemsRequest is started to retrieve remote items (list of ids only) and an ItemFetchJob
 * is started to fetch local items (from cache only). Both of these jobs are started simultaneously.
 *
 * The second stage begins when both item list query jobs have finished. The goal of this stage is
 * to determine a list of items to fetch more details for. Since the EwsSyncFolderItemsRequest can
 * retrieve incremental information further processing depends on whether the sync request was a
 * full sync (no sync state) or an incremental sync.
 *
 * In case of a full sync both item lists are compared to determine lists of new/changed items and
 * deleted items. For an incremental sync there is no need to compare as the lists of
 * added/changed/deleted items are already given. 'IsRead' flag changes changes are treated
 * specially - the modification is trivial and is performed straight away without fetching item
 * details.
 *
 * The list of new/changed items is then used to perform a second remote request in order to
 * retrieve the details of these items. For e-mail items the second fetch only retrieves the
 * item headers. For other items the full MIME content is fetched.
 *
 * In case of an incremental sync the compare code checks if items marked as 'changed' or 'deleted'
 * exist in Akonadi database. If not an error is raised. This serves as an information to the
 * resource class that an incremental sync has failed due to an out-of-sync state and a full sync
 * is required to bring thing back to order.
 *
 * In addition to a regular sync it is sometimes necessary to check for existence of some specific
 * items. This happens when some operation failed and the resource tries to work its way around to
 * get Akonadi into a synchronous state after the failure. For this purpose the caller can provide
 * a list of item identifiers to look for besides the regular sync. If this list contains elements
 * another request (EwsGetItemRequest) is issued in parallel for the selected items. Once this
 * request returns the list is cross-checked with the server responses. If an item is found on the
 * server side it is added to the "new items" list. Otherwise it is added to the "deleted items"
 * list. In the latter case the code for determining if the deleted item is in the Akonadi database
 * will not raise an error for such deleted item. This helps to avoid unnecessary full syncs.
 *
 * The fetch item job also allows providing a list of expected item changes. This is necessary
 * because the incremental sync will return events for operations made by the resource itself. This
 * can lead to false failures when for example an item was deleted (i.e. does not exist in Akonadi
 * any more) and the incremental sync returns a delete event for this item. Typically this would
 * result in an error and force a full sync. Providing this list allows for this particular error
 * to be safely ignored.
 */

EwsFetchItemsJob::EwsFetchItemsJob(const Collection &collection,
                                   EwsClient &client,
                                   const QString &syncState,
                                   const EwsId::List &itemsToCheck,
                                   EwsTagStore *tagStore,
                                   EwsResource *parent)
    : EwsJob(parent)
    , mCollection(collection)
    , mClient(client)
    , mItemsToCheck(itemsToCheck)
    , mPendingJobs(0)
    , mTotalItemsToFetch(0)
    , mTotalItemsFetched(0)
    , mSyncState(syncState)
    , mFullSync(syncState.isNull())
    , mTagStore(tagStore)
    , mTagsSynced(false)
{
    qRegisterMetaType<EwsId::List>();
}

EwsFetchItemsJob::~EwsFetchItemsJob() = default;

void EwsFetchItemsJob::start()
{
    Q_EMIT reportStatus(AgentBase::Running, i18nc("@info:status", "Retrieving %1 item list", mCollection.name()));
    Q_EMIT reportPercent(0);

    /* Begin stage 1 - query item list from local and remote side. */
    auto syncItemsReq = new EwsSyncFolderItemsRequest(mClient, this);
    syncItemsReq->setFolderId(EwsId(mCollection.remoteId(), mCollection.remoteRevision()));
    EwsItemShape shape(EwsShapeIdOnly);
    shape << EwsResource::tagsProperty;
    syncItemsReq->setItemShape(shape);
    if (!mSyncState.isNull()) {
        syncItemsReq->setSyncState(mSyncState);
    }
    syncItemsReq->setMaxChanges(listBatchSize);
    connect(syncItemsReq, &EwsSyncFolderItemsRequest::result, this, &EwsFetchItemsJob::remoteItemFetchDone);
    addSubjob(syncItemsReq);

    auto itemJob = new ItemFetchJob(mCollection);
    ItemFetchScope itemScope;
    itemScope.setCacheOnly(true);
    itemScope.fetchFullPayload(false);
    itemJob->setFetchScope(itemScope);
    connect(itemJob, &ItemFetchJob::result, this, &EwsFetchItemsJob::localItemFetchDone);
    addSubjob(itemJob);

    mPendingJobs = 2;
    syncItemsReq->start();
    itemJob->start();

    if (!mItemsToCheck.isEmpty()) {
        auto getItemReq = new EwsGetItemRequest(mClient, this);
        getItemReq->setItemIds(mItemsToCheck);
        getItemReq->setItemShape(EwsItemShape(EwsShapeIdOnly));
        connect(getItemReq, &EwsGetItemRequest::result, this, &EwsFetchItemsJob::checkedItemsFetchFinished);
        ++mPendingJobs;
        getItemReq->start();
    }
}

void EwsFetchItemsJob::localItemFetchDone(KJob *job)
{
    auto fetchJob = qobject_cast<ItemFetchJob *>(job);

    qCDebug(EWSRES_LOG) << "EwsFetchItemsJob::localItemFetchDone";
    if (!fetchJob) {
        setErrorMsg(QStringLiteral("Invalid item fetch job pointer."));
        doKill();
        emitResult();
        return;
    }

    if (!fetchJob->error()) {
        removeSubjob(job);
        mLocalItems = fetchJob->items();
        --mPendingJobs;
        if (mPendingJobs == 0) {
            compareItemLists();
        }
    }
}

void EwsFetchItemsJob::remoteItemFetchDone(KJob *job)
{
    auto itemReq = qobject_cast<EwsSyncFolderItemsRequest *>(job);

    qCDebug(EWSRES_LOG) << "EwsFetchItemsJob::remoteItemFetchDone";
    if (!itemReq) {
        setErrorMsg(QStringLiteral("Invalid find item request pointer."));
        doKill();
        emitResult();
        return;
    }

    if (!itemReq->error()) {
        removeSubjob(job);
        const auto reqChanges{itemReq->changes()};
        for (const EwsSyncFolderItemsRequest::Change &change : reqChanges) {
            switch (change.type()) {
            case EwsSyncFolderItemsRequest::Create:
                mRemoteAddedItems.append(change.item());
                break;
            case EwsSyncFolderItemsRequest::Update:
                mRemoteChangedItems.append(change.item());
                break;
            case EwsSyncFolderItemsRequest::Delete:
                mRemoteDeletedIds.append(change.itemId());
                break;
            case EwsSyncFolderItemsRequest::ReadFlagChange:
                mRemoteFlagChangedIds.insert(change.itemId(), change.isRead());
                break;
            default:
                break;
            }
        }

        if (!itemReq->includesLastItem()) {
            auto syncItemsReq = new EwsSyncFolderItemsRequest(mClient, this);
            syncItemsReq->setFolderId(EwsId(mCollection.remoteId(), mCollection.remoteRevision()));
            EwsItemShape shape(EwsShapeIdOnly);
            shape << EwsResource::tagsProperty;
            syncItemsReq->setItemShape(shape);
            syncItemsReq->setSyncState(itemReq->syncState());
            syncItemsReq->setMaxChanges(listBatchSize);
            connect(syncItemsReq, &KJob::result, this, &EwsFetchItemsJob::remoteItemFetchDone);
            addSubjob(syncItemsReq);
            syncItemsReq->start();
        } else {
            mSyncState = itemReq->syncState();
            --mPendingJobs;
            if (mPendingJobs == 0) {
                compareItemLists();
            }
        }
        const auto totalItems = mRemoteAddedItems.size() + mRemoteChangedItems.size() + mRemoteDeletedIds.size() + mRemoteFlagChangedIds.size();
        if (!mLocalItems.empty()) {
            Q_EMIT reportPercent(qMin(totalItems * 50 / mLocalItems.size(), 50));
        }
        Q_EMIT reportStatus(AgentBase::Running, i18nc("@info:status", "Retrieving %1 item list (%2 items)", mCollection.name(), totalItems));
    } else {
        setEwsResponseCode(itemReq->ewsResponseCode());
        qCWarningNC(EWSRES_LOG) << QStringLiteral("EwsFetchItemsJob: Failed remote item sync");
    }
}

void EwsFetchItemsJob::checkedItemsFetchFinished(KJob *job)
{
    auto req = qobject_cast<EwsGetItemRequest *>(job);

    if (!req) {
        setErrorMsg(QStringLiteral("Invalid item fetch job pointer."));
        doKill();
        emitResult();
        return;
    }

    if (!req->error()) {
        removeSubjob(job);

        Q_ASSERT(mItemsToCheck.size() == req->responses().size());

        EwsId::List::const_iterator it = mItemsToCheck.cbegin();
        const auto reqResponses{req->responses()};
        for (const EwsGetItemRequest::Response &resp : reqResponses) {
            if (resp.isSuccess()) {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Checked item %1 found - readding").arg(ewsHash(it->id()));
                mRemoteAddedItems.append(resp.item());
            } else {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Checked item %1 not found - removing").arg(ewsHash(it->id()));
                mRemoteDeletedIds.append(*it);
            }
            ++it;
        }
        --mPendingJobs;
        if (mPendingJobs == 0) {
            compareItemLists();
        }
    }
}

bool EwsFetchItemsJob::processIncrementalRemoteItemUpdates(const EwsItem::List &items,
                                                           QHash<QString, Item> &itemHash,
                                                           QHash<EwsItemType, Item::List> &toFetchItems)
{
    for (const EwsItem &ewsItem : items) {
        auto id(ewsItem[EwsItemFieldItemId].value<EwsId>());
        auto it = itemHash.find(id.id());
        if (it == itemHash.end()) {
            setErrorMsg(QStringLiteral("Got update for item %1, but item not found in local store.").arg(ewsHash(id.id())));
            emitResult();
            return false;
        }
        const auto qitup = mQueuedUpdates[EwsModifiedEvent].find(id.id());
        if (qitup != mQueuedUpdates[EwsModifiedEvent].end() && *qitup == id.changeKey()) {
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Match for queued modification of item %1").arg(ewsHash(id.id()));
            continue;
        }
        Item &item = *it;
        if (item.remoteRevision() == id.changeKey()) {
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Matching change key for item %1 - not syncing").arg(ewsHash(id.id()));
            continue;
        }
        item.clearPayload();
        item.setRemoteRevision(id.changeKey());
        if (!mTagStore->readEwsProperties(item, ewsItem, mTagsSynced)) {
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Missing tags encountered - forcing sync");
            syncTags();
            return false;
        }
        EwsItemType type = ewsItem.internalType();
        toFetchItems[type].append(item);
        ++mTotalItemsToFetch;
    }

    return true;
}

void EwsFetchItemsJob::compareItemLists()
{
    /* Begin stage 2 - determine list of new/changed items and fetch details about them. */

    QHash<EwsItemType, Item::List> toFetchItems;

    QHash<QString, Item> itemHash;
    for (const Item &item : std::as_const(mLocalItems)) {
        itemHash.insert(item.remoteId(), item);
    }

    for (const EwsItem &ewsItem : std::as_const(mRemoteAddedItems)) {
        /* In case of a full sync all existing items appear as added on the remote side. Therefore
         * look for the item in the local list before creating a new copy. */
        auto id(ewsItem[EwsItemFieldItemId].value<EwsId>());
        QHash<QString, Item>::iterator it = itemHash.find(id.id());
        EwsItemType type = ewsItem.internalType();
        if (type == EwsItemTypeUnknown) {
            /* Ignore unknown items. */
            continue;
        }
        auto handler(EwsItemHandler::itemHandler(type));
        if (!handler) {
            /* Ignore items where no handler exists. */
            continue;
        }
        QString mimeType = handler->mimeType();
        if (it == itemHash.end()) {
            Item item(mimeType);
            item.setParentCollection(mCollection);
            auto id = ewsItem[EwsItemFieldItemId].value<EwsId>();
            item.setRemoteId(id.id());
            item.setRemoteRevision(id.changeKey());
            if (!mTagStore->readEwsProperties(item, ewsItem, mTagsSynced)) {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Missing tags encountered - forcing sync");
                syncTags();
                return;
            }
            toFetchItems[type].append(item);
            ++mTotalItemsToFetch;
        } else {
            Item &item = *it;
            /* In case of a full sync even unchanged items appear as new. Compare the change keys
             * to determine if a fetch is needed. */
            if (item.remoteRevision() != id.changeKey()) {
                item.clearPayload();
                item.setRemoteRevision(id.changeKey());
                if (!mTagStore->readEwsProperties(item, ewsItem, mTagsSynced)) {
                    qCDebugNC(EWSRES_LOG) << QStringLiteral("Missing tags encountered - forcing sync");
                    syncTags();
                    return;
                }
                toFetchItems[type].append(item);
                ++mTotalItemsToFetch;
            } else {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Matching change key for item %1 - not syncing").arg(ewsHash(id.id()));
            }
            itemHash.erase(it);
        }
    }

    if (mFullSync) {
        /* In case of a full sync all items that are still on the local item list do not exist
         * remotely and need to be deleted locally. */
        const QHash<QString, Item>::iterator end(itemHash.end());
        for (QHash<QString, Item>::iterator it = itemHash.begin(); it != end; ++it) {
            mDeletedItems.append(it.value());
        }
    } else {
        if (!processIncrementalRemoteItemUpdates(mRemoteChangedItems, itemHash, toFetchItems)) {
            return;
        }

        // In case of an incremental sync deleted items will be given explicitly. */
        for (const EwsId &id : std::as_const(mRemoteDeletedIds)) {
            QHash<QString, Item>::iterator it = itemHash.find(id.id());
            if (it == itemHash.end()) {
                /* If an item is not found locally, it can mean two things:
                 *  1. The item got deleted earlier without the resource being told about it.
                 *  2. The item was never known by Akonadi due to a sync problem.
                 * Either way the item doesn't exist any more and there is no point crying about it. */
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Got delete for item %1, but item not found in local store.").arg(ewsHash(id.id()));
            } else {
                mDeletedItems.append(*it);
            }
        }

        QHash<EwsId, bool>::const_iterator it;
        EwsItemHandler *handler = EwsItemHandler::itemHandler(EwsItemTypeMessage);
        for (it = mRemoteFlagChangedIds.cbegin(); it != mRemoteFlagChangedIds.cend(); ++it) {
            QHash<QString, Item>::iterator iit = itemHash.find(it.key().id());
            if (iit == itemHash.end()) {
                setErrorMsg(QStringLiteral("Got read flag change for item %1, but item not found in local store.").arg(it.key().id()));
                emitResult();
                return;
            }
            Item &item = *iit;
            handler->setSeenFlag(item, it.value());
            mChangedItems.append(item);
            itemHash.erase(iit);
        }
    }

    qCDebugNC(EWSRES_LOG)
        << QStringLiteral("Changed %2, deleted %3, new %4").arg(mRemoteChangedItems.size()).arg(mDeletedItems.size()).arg(mRemoteAddedItems.size());

    Q_EMIT reportStatus(AgentBase::Running, i18nc("@info:status", "Retrieving %1 items", mCollection.name()));

    bool fetch = false;
    for (const auto &iType : toFetchItems.keys()) {
        for (int i = 0; i < toFetchItems[iType].size(); i += fetchBatchSize) {
            EwsItemHandler *handler = EwsItemHandler::itemHandler(static_cast<EwsItemType>(iType));
            if (!handler) {
                // TODO: Temporarily ignore unsupported item types.
                qCWarning(EWSRES_LOG) << QStringLiteral("Unable to initialize fetch for item type %1").arg(iType);
                /*setErrorMsg(QStringLiteral("Unable to initialize fetch for item type %1").arg(iType));
                emitResult();
                return;*/
            } else {
                EwsFetchItemDetailJob *job = handler->fetchItemDetailJob(mClient, this, mCollection);
                Item::List itemList = toFetchItems[iType].mid(i, fetchBatchSize);
                job->setItemLists(itemList, &mDeletedItems);
                connect(job, &KJob::result, this, &EwsFetchItemsJob::itemDetailFetchDone);
                addSubjob(job);
                fetch = true;
            }
        }
    }
    if (!fetch) {
        // Nothing to fetch - we're done here.
        emitResult();
    } else {
        subjobs().first()->start();
    }
}

void EwsFetchItemsJob::itemDetailFetchDone(KJob *job)
{
    const auto detailJob = qobject_cast<EwsFetchItemDetailJob *>(job);
    if (detailJob) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("itemDetailFetchDone: ") << detailJob->error();
        if (!detailJob->error()) {
            removeSubjob(job);

            const auto changedItems = detailJob->changedItems();
            for (const auto &item : changedItems) {
                if (item.isValid()) {
                    mChangedItems.append(item);
                } else {
                    mNewItems.append(item);
                }
            }

            mTotalItemsFetched = mChangedItems.size();
            Q_EMIT reportPercent(50 + (mTotalItemsFetched * 50) / mTotalItemsToFetch);

            if (subjobs().isEmpty()) {
                emitResult();
            } else {
                subjobs().first()->start();
            }
        } else {
            setEwsResponseCode(detailJob->ewsResponseCode());
        }
    }
}

void EwsFetchItemsJob::setQueuedUpdates(const QueuedUpdateList &updates)
{
    mQueuedUpdates.clear();
    for (const QueuedUpdate &upd : updates) {
        mQueuedUpdates[upd.type].insert(upd.id, upd.changeKey);
        qCDebugNC(EWSRES_LOG) << QStringLiteral("Queued update %1 for item %2").arg(upd.type).arg(ewsHash(upd.id));
    }
}

void EwsFetchItemsJob::syncTags()
{
    if (mTagsSynced) {
        setErrorMsg(QStringLiteral("Missing tags encountered despite previous sync."));
        emitResult();
    } else {
        auto job = new EwsAkonadiTagsSyncJob(mTagStore, mClient, qobject_cast<EwsResource *>(parent())->rootCollection(), this);
        connect(job, &EwsAkonadiTagsSyncJob::result, this, &EwsFetchItemsJob::tagSyncFinished);
        job->start();
        mTagsSynced = true;
    }
}

void EwsFetchItemsJob::tagSyncFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorText());
        emitResult();
    } else {
        compareItemLists();
    }
}
