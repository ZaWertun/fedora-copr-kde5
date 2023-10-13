/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsresource.h"

#include <QDebug>

#include <Akonadi/AttributeFactory>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/SpecialMailCollections>
#include <KMime/Message>
#include <KNotification>

#include <KLocalizedString>

#include "auth/ewsabstractauth.h"
#include "ewsconfigdialog.h"
#include "ewscreatefolderrequest.h"
#include "ewscreateitemjob.h"
#if HAVE_SEPARATE_MTA_RESOURCE
#include "ewscreateitemrequest.h"
#endif
#include "ewsdeletefolderrequest.h"
#include "ewsdeleteitemrequest.h"
#include "ewsfetchfoldersincrjob.h"
#include "ewsfetchfoldersjob.h"
#include "ewsfetchitempayloadjob.h"
#include "ewsgetfolderrequest.h"
#include "ewsgetitemrequest.h"
#include "ewsitemhandler.h"
#include "ewsmodifyitemflagsjob.h"
#include "ewsmodifyitemjob.h"
#include "ewsmovefolderrequest.h"
#include "ewsmoveitemrequest.h"
#include "ewsresource_debug.h"
#include "ewssettings.h"
#include "ewssubscriptionmanager.h"
#include "ewssyncstateattribute.h"
#include "ewsupdatefolderrequest.h"
#include "tags/ewsglobaltagsreadjob.h"
#include "tags/ewsglobaltagswritejob.h"
#include "tags/ewstagstore.h"
#include "tags/ewsupdateitemstagsjob.h"

#include "ewsresourceadaptor.h"
#include "ewssettingsadaptor.h"
#include "ewswalletadaptor.h"

using namespace Akonadi;

struct SpecialFolders {
    EwsDistinguishedId did;
    SpecialMailCollections::Type type;
    QString iconName;
};

static const QVector<SpecialFolders> specialFolderList = {{EwsDIdInbox, SpecialMailCollections::Inbox, QStringLiteral("mail-folder-inbox")},
                                                          {EwsDIdOutbox, SpecialMailCollections::Outbox, QStringLiteral("mail-folder-outbox")},
                                                          {EwsDIdSentItems, SpecialMailCollections::SentMail, QStringLiteral("mail-folder-sent")},
                                                          {EwsDIdDeletedItems, SpecialMailCollections::Trash, QStringLiteral("user-trash")},
                                                          {EwsDIdDrafts, SpecialMailCollections::Drafts, QStringLiteral("document-properties")}};

const QString EwsResource::akonadiEwsPropsetUuid = QStringLiteral("9bf757ae-69b5-4d8a-bf1d-2dd0c0871a28");

const EwsPropertyField EwsResource::globalTagsProperty(EwsResource::akonadiEwsPropsetUuid, QStringLiteral("GlobalTags"), EwsPropTypeStringArray);
const EwsPropertyField EwsResource::globalTagsVersionProperty(EwsResource::akonadiEwsPropsetUuid, QStringLiteral("GlobalTagsVersion"), EwsPropTypeInteger);
const EwsPropertyField EwsResource::tagsProperty(EwsResource::akonadiEwsPropsetUuid, QStringLiteral("Tags"), EwsPropTypeStringArray);
const EwsPropertyField EwsResource::flagsProperty(EwsResource::akonadiEwsPropsetUuid, QStringLiteral("Flags"), EwsPropTypeStringArray);

static constexpr int InitialReconnectTimeout = 15;
static constexpr int MaxReconnectTimeout = 300;

EwsResource::EwsResource(const QString &id)
    : Akonadi::ResourceBase(id)
    , mAuthStage(AuthIdle)
    , mTagsRetrieved(false)
    , mReconnectTimeout(InitialReconnectTimeout)
    , mInitialReconnectTimeout(InitialReconnectTimeout)
    , mSettings(new EwsSettings(winIdForDialogs()))
{
    AttributeFactory::registerAttribute<EwsSyncStateAttribute>();

    mEwsClient.setUserAgent(mSettings->userAgent());
    mEwsClient.setEnableNTLMv2(mSettings->enableNTLMv2());

    changeRecorder()->fetchCollection(true);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(CollectionFetchScope::Parent);
    changeRecorder()->collectionFetchScope().fetchAttribute<EwsSyncStateAttribute>();
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::Parent);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->itemFetchScope().setFetchTags(true);

    mRootCollection.setParentCollection(Collection::root());
    mRootCollection.setName(name());
    mRootCollection.setContentMimeTypes(QStringList() << Collection::mimeType() << KMime::Message::mimeType());
    mRootCollection.setRights(Collection::ReadOnly);

    setScheduleAttributeSyncBeforeItemSync(true);

    // Load the sync state
    QByteArray data = QByteArray::fromBase64(mSettings->folderSyncState().toLatin1());
    if (!data.isEmpty()) {
        data = qUncompress(data);
        if (!data.isEmpty()) {
            mFolderSyncState = QString::fromLatin1(data);
        }
    }

    setHierarchicalRemoteIdentifiersEnabled(true);

    mTagStore = new EwsTagStore(this);

    QMetaObject::invokeMethod(this, &EwsResource::delayedInit, Qt::QueuedConnection);

    connect(this, &AgentBase::reloadConfiguration, this, &EwsResource::reloadConfig);
    connect(this, &ResourceBase::nameChanged, this, &EwsResource::adjustRootCollectionName);
}

EwsResource::~EwsResource() = default;

void EwsResource::delayedInit()
{
    new EwsResourceAdaptor(this);
    new EwsSettingsAdaptor(mSettings.data());
    new EwsWalletAdaptor(mSettings.data());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), mSettings.data(), QDBusConnection::ExportAdaptors);
}

void EwsResource::resetUrl()
{
    Q_EMIT status(Running, i18nc("@info:status", "Connecting to Exchange server"));

    auto req = new EwsGetFolderRequest(mEwsClient, this);
    const EwsId::List folders{EwsId(EwsDIdMsgFolderRoot), EwsId(EwsDIdInbox)};
    req->setFolderIds(folders);
    EwsFolderShape shape(EwsShapeIdOnly);
    shape << EwsPropertyField(QStringLiteral("folder:DisplayName"));
    // Use the opportunity of reading the root folder to read the tag data.
    shape << globalTagsProperty << globalTagsVersionProperty;
    req->setFolderShape(shape);
    connect(req, &EwsRequest::result, this, &EwsResource::rootFolderFetchFinished);
    req->start();
}

void EwsResource::rootFolderFetchFinished(KJob *job)
{
    auto req = qobject_cast<EwsGetFolderRequest *>(job);
    if (!req) {
        Q_EMIT status(Idle, i18nc("@info:status", "Unable to connect to Exchange server"));
        setTemporaryOffline(reconnectTimeout());
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid EwsGetFolderRequest job object");
        return;
    }

    if (req->error()) {
        Q_EMIT status(Idle, i18nc("@info:status", "Unable to connect to Exchange server"));
        setTemporaryOffline(reconnectTimeout());
        qWarning() << "ERROR" << req->errorString();
        return;
    }

    if (req->responses().size() != 2) {
        Q_EMIT status(Idle, i18nc("@info:status", "Unable to connect to Exchange server"));
        setTemporaryOffline(reconnectTimeout());
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid number of responses received");
        return;
    }

    EwsFolder folder = req->responses()[1].folder();
    auto id = folder[EwsFolderFieldFolderId].value<EwsId>();
    if (id.type() == EwsId::Real) {
        /* Since KDE PIM is heavily based on IMAP philosophy it would only consider for filtering
         * folders with the remote identifier set to "INBOX". While this is true for IMAP/POP3, Exchange
         * uses Base64-encoded strings with data private to the server. In order for mail filtering to work
         * the EWS resource has pretended that the inbox folder's remote name is "INBOX". Since KDE Applications
         * 17.12 this workaround is no longer needed, however in order to clean-up after old Akonadi EWS
         * installations the code below sets the correct Exchange id for the Inbox folder.
         *
         * At some day in the future this part of code can be removed too. */
        Collection c;
        c.setRemoteId(QStringLiteral("INBOX"));
        auto job = new CollectionFetchJob(c, CollectionFetchJob::Base, this);
        job->setFetchScope(changeRecorder()->collectionFetchScope());
        job->fetchScope().setResource(identifier());
        job->fetchScope().setListFilter(CollectionFetchScope::Sync);
        job->setProperty("inboxId", id.id());
        connect(job, &CollectionFetchJob::result, this, &EwsResource::adjustInboxRemoteIdFetchFinished);

        int inboxIdx = mSettings->serverSubscriptionList().indexOf(QLatin1String("INBOX"));
        if (inboxIdx >= 0) {
            QStringList subList = mSettings->serverSubscriptionList();
            subList[inboxIdx] = id.id();
            mSettings->setServerSubscriptionList(subList);
        }
    }

    folder = req->responses().first().folder();
    id = folder[EwsFolderFieldFolderId].value<EwsId>();
    if (id.type() == EwsId::Real) {
        mRootCollection.setRemoteId(id.id());
        mRootCollection.setRemoteRevision(id.changeKey());
        qCDebug(EWSRES_LOG) << "Root folder is " << id;
        emitReadyStatus();
        mReconnectTimeout = mInitialReconnectTimeout;

        if (mSettings->serverSubscription()) {
            mSubManager.reset(new EwsSubscriptionManager(mEwsClient, id, mSettings.data(), this));
            connect(mSubManager.data(), &EwsSubscriptionManager::foldersModified, this, &EwsResource::foldersModifiedEvent);
            connect(mSubManager.data(), &EwsSubscriptionManager::folderTreeModified, this, &EwsResource::folderTreeModifiedEvent);
            connect(mSubManager.data(), &EwsSubscriptionManager::fullSyncRequested, this, &EwsResource::fullSyncRequestedEvent);

            /* Use a queued connection here as the connectionError() method will actually destroy the subscription manager. If this
             * was done with a direct connection this would have ended up with destroying the caller object followed by a crash. */
            connect(mSubManager.data(), &EwsSubscriptionManager::connectionError, this, &EwsResource::connectionError, Qt::QueuedConnection);
            mSubManager->start();
        }

        synchronizeCollectionTree();

        mTagStore->readTags(folder[globalTagsProperty].toStringList(), folder[globalTagsVersionProperty].toInt());
    }
}

void EwsResource::adjustInboxRemoteIdFetchFinished(KJob *job)
{
    if (!job->error()) {
        auto fetchJob = qobject_cast<CollectionFetchJob *>(job);
        Q_ASSERT(fetchJob);
        if (!fetchJob->collections().isEmpty()) {
            Collection c = fetchJob->collections()[0];
            c.setRemoteId(fetchJob->property("inboxId").toString());
            auto modifyJob = new CollectionModifyJob(c, this);
            modifyJob->start();
        }
    }
}

void EwsResource::retrieveCollections()
{
    if (mRootCollection.remoteId().isNull()) {
        cancelTask(i18nc("@info:status", "Root folder id not known."));
        return;
    }

    Q_EMIT status(Running, i18nc("@info:status", "Retrieving collection tree"));

    if (!mFolderSyncState.isEmpty() && !mRootCollection.isValid()) {
        /* When doing an incremental sync the real Akonadi identifier of the root collection must
         * be known, because the retrieved list of changes needs to include all parent folders up
         * to the root. None of the child collections are required to be valid, but the root must
         * be, as it needs to be the anchor point.
         */
        auto fetchJob = new CollectionFetchJob(mRootCollection, CollectionFetchJob::Base);
        connect(fetchJob, &CollectionFetchJob::result, this, &EwsResource::rootCollectionFetched);
        fetchJob->start();
    } else {
        doRetrieveCollections();
    }
    synchronizeTags();
}

void EwsResource::rootCollectionFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(EWSRES_LOG) << "ERROR" << job->errorString();
    } else {
        auto fetchJob = qobject_cast<CollectionFetchJob *>(job);
        if (fetchJob && !fetchJob->collections().isEmpty()) {
            mRootCollection = fetchJob->collections().at(0);
            adjustRootCollectionName(name());
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Root collection fetched: ") << mRootCollection;
        }
    }

    /* If the fetch failed for whatever reason force a full sync, which doesn't require the root
     * collection to be valid. */
    if (!mRootCollection.isValid()) {
        mFolderSyncState.clear();
    }

    doRetrieveCollections();
}

void EwsResource::doRetrieveCollections()
{
    if (mFolderSyncState.isEmpty()) {
        auto job = new EwsFetchFoldersJob(mEwsClient, mRootCollection, this);
        connect(job, &EwsFetchFoldersJob::result, this, &EwsResource::fetchFoldersJobFinished);
        connectStatusSignals(job);
        job->start();
    } else {
        auto job = new EwsFetchFoldersIncrJob(mEwsClient, mFolderSyncState, mRootCollection, this);
        connect(job, &EwsFetchFoldersIncrJob::result, this, &EwsResource::fetchFoldersIncrJobFinished);
        connectStatusSignals(job);
        job->start();
    }
}

void EwsResource::connectionError()
{
    Q_EMIT status(Broken, i18nc("@info:status", "Unable to connect to Exchange server"));
    setTemporaryOffline(reconnectTimeout());
}

void EwsResource::retrieveItems(const Collection &collection)
{
    queueFetchItemsJob(collection, RetrieveItems, [this](EwsFetchItemsJob *fetchJob) {
        auto col = fetchJob->collection();
        if (fetchJob->error()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Item fetch error:") << fetchJob->errorString() << fetchJob->error() << fetchJob->ewsResponseCode();
            if (!isEwsResponseCodeTemporaryError(fetchJob->ewsResponseCode())) {
                const auto syncState = getCollectionSyncState(fetchJob->collection());
                if (!syncState.isEmpty()) {
                    qCDebugNC(EWSRES_LOG) << QStringLiteral("Retrying with empty state.");
                    // Retry with a clear sync state.
                    saveCollectionSyncState(col, QString());
                    retrieveItems(col);
                } else {
                    qCDebugNC(EWSRES_LOG) << QStringLiteral("Clean sync failed.");
                    // No more hope
                    cancelTask(i18nc("@info:status", "Failed to retrieve items"));
                    return;
                }
            } else {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Sync failed due to temporary error - not clearing state");
                cancelTask(i18nc("@info:status", "Failed to retrieve items"));
                setTemporaryOffline(reconnectTimeout());
                return;
            }
        } else {
            saveCollectionSyncState(col, fetchJob->syncState());
            itemsRetrievedIncremental(fetchJob->newItems() + fetchJob->changedItems(), fetchJob->deletedItems());
        }
        saveState();
        mItemsToCheck.remove(fetchJob->collection().remoteId());
        emitReadyStatus();
    });
}

void EwsResource::queueFetchItemsJob(const Akonadi::Collection &col, QueuedFetchItemsJobType type, const std::function<void(EwsFetchItemsJob *)> &startFn)
{
    qCDebugNC(EWSRES_LOG) << QStringLiteral("Enqueuing sync for collection ") << col << col.id();

    const auto queueEmpty = mFetchItemsJobQueue.empty();
    if (mFetchItemsJobQueue.count() > 1) {
        // Don't enqueue the same collection id, type pair twice, except for the first element,
        // which belongs to the collection being synced right now.
        for (const auto &item : std::as_const(mFetchItemsJobQueue).mid(1)) {
            if ((item.col == col) && (item.type == type)) {
                qCDebugNC(EWSRES_LOG) << QStringLiteral("Sync already queued - skipping");
                return;
            }
        }
    }

    mFetchItemsJobQueue.enqueue({col, type, startFn});

    qCDebugNC(EWSRES_LOG) << QStringLiteral("Sync queue state: ") << dumpResourceToString().replace(QLatin1Char('\n'), QLatin1Char(' '));

    if (queueEmpty) {
        startFetchItemsJob(col, startFn);
    }
}

void EwsResource::dequeueFetchItemsJob()
{
    qCDebugNC(EWSRES_LOG) << QStringLiteral("Finished queued sync ") << mFetchItemsJobQueue.head().col << mFetchItemsJobQueue.head().col.id();

    mFetchItemsJobQueue.dequeue();

    if (!mFetchItemsJobQueue.empty()) {
        const auto &head = mFetchItemsJobQueue.head();
        startFetchItemsJob(head.col, head.startFn);
    }
}

void EwsResource::startFetchItemsJob(const Akonadi::Collection &col, std::function<void(EwsFetchItemsJob *)> startFn)
{
    qCDebugNC(EWSRES_LOG) << QStringLiteral("Starting queued sync for collection ") << col;

    auto fetchJob = new EwsFetchItemsJob(col, mEwsClient, getCollectionSyncState(col), mItemsToCheck.value(col.remoteId()), mTagStore, this);
    connect(fetchJob, &EwsFetchItemsJob::result, this, [this, startFn, fetchJob](KJob *) {
        startFn(fetchJob);
        dequeueFetchItemsJob();
    });
    connectStatusSignals(fetchJob);
    fetchJob->start();
}

bool EwsResource::retrieveItems(const Item::List &items, const QSet<QByteArray> &parts)
{
    qCDebugNC(EWSRES_AGENTIF_LOG) << "retrieveItems: start " << items << parts;

    Q_EMIT status(Running, i18nc("@info:status", "Retrieving items"));

    auto job = new EwsFetchItemPayloadJob(mEwsClient, this, items);
    connect(job, &EwsGetItemRequest::result, this, &EwsResource::getItemsRequestFinished);
    connectStatusSignals(job);
    job->start();

    return true;
}

void EwsResource::getItemsRequestFinished(KJob *job)
{
    emitReadyStatus();

    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto *fetchJob = qobject_cast<EwsFetchItemPayloadJob *>(job);
    if (!fetchJob) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid EwsFetchItemPayloadJob job object");
        cancelTask(i18nc("@info:status", "Failed to retrieve items - internal error"));
        return;
    }

    qCDebugNC(EWSRES_AGENTIF_LOG) << "retrieveItems: done";
    itemsRetrieved(fetchJob->items());
}

void EwsResource::reloadConfig()
{
    mSubManager.reset(nullptr);
    mEwsClient.setUrl(mSettings->baseUrl());
    setUpAuth();
    mEwsClient.setAuth(mAuth.data());
}

void EwsResource::configure(WId windowId)
{
    QPointer<EwsConfigDialog> dlg = new EwsConfigDialog(this, mEwsClient, windowId, mSettings.data());
    if (dlg->exec()) {
        reloadConfig();
        Q_EMIT configurationDialogAccepted();
    } else {
        Q_EMIT configurationDialogRejected();
    }
    delete dlg;
}

void EwsResource::fetchFoldersJobFinished(KJob *job)
{
    emitReadyStatus();
    auto req = qobject_cast<EwsFetchFoldersJob *>(job);
    if (!req) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid EwsFetchFoldersJob job object");
        cancelTask(i18nc("@info:status", "Failed to retrieve folders - internal error"));
        return;
    }

    if (req->error()) {
        qWarning() << "ERROR" << req->errorString();
        cancelTask(i18nc("@info:status", "Failed to process folders retrieval request"));
        return;
    }

    mFolderSyncState = req->syncState();
    saveState();
    collectionsRetrieved(req->folders());

    fetchSpecialFolders();
}

void EwsResource::fetchFoldersIncrJobFinished(KJob *job)
{
    emitReadyStatus();
    auto req = qobject_cast<EwsFetchFoldersIncrJob *>(job);
    if (!req) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid EwsFetchFoldersIncrJob job object");
        cancelTask(i18nc("@info:status", "Invalid incremental folders retrieval request job object"));
        return;
    }

    if (req->error()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("ERROR") << req->errorString();

        /* Retry with a full sync. */
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Retrying with a full sync.");
        mFolderSyncState.clear();
        doRetrieveCollections();
        return;
    }

    mFolderSyncState = req->syncState();
    saveState();
    collectionsRetrievedIncremental(req->changedFolders(), req->deletedFolders());

    if (!req->changedFolders().isEmpty() || !req->deletedFolders().isEmpty()) {
        fetchSpecialFolders();
    }
}

void EwsResource::itemFetchJobFinished(KJob *job)
{
    auto fetchJob = qobject_cast<EwsFetchItemsJob *>(job);

    if (!fetchJob) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Invalid EwsFetchItemsJobjob object");
        cancelTask(i18nc("@info:status", "Failed to retrieve items - internal error"));
        return;
    }
    auto col = fetchJob->collection();
    if (job->error()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Item fetch error:") << job->errorString();
        const auto syncState = getCollectionSyncState(fetchJob->collection());
        if (!syncState.isEmpty()) {
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Retrying with empty state.");
            // Retry with a clear sync state.
            saveCollectionSyncState(col, QString());
            retrieveItems(col);
        } else {
            qCDebugNC(EWSRES_LOG) << QStringLiteral("Clean sync failed.");
            // No more hope
            cancelTask(i18nc("@info:status", "Failed to retrieve items"));
            return;
        }
    } else {
        saveCollectionSyncState(col, fetchJob->syncState());
        itemsRetrievedIncremental(fetchJob->newItems() + fetchJob->changedItems(), fetchJob->deletedItems());
    }
    saveState();
    mItemsToCheck.remove(fetchJob->collection().remoteId());
    emitReadyStatus();
}

void EwsResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers)
{
    qCDebugNC(EWSRES_AGENTIF_LOG) << "itemChanged: start " << item << partIdentifiers;

    EwsItemType type = EwsItemHandler::mimeToItemType(item.mimeType());
    if (isEwsMessageItemType(type)) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemChanged: Item type not supported for changing";
        cancelTask(i18nc("@info:status", "Item type not supported for changing"));
    } else {
        EwsModifyItemJob *job = EwsItemHandler::itemHandler(type)->modifyItemJob(mEwsClient, Item::List() << item, partIdentifiers, this);
        connect(job, &KJob::result, this, &EwsResource::itemChangeRequestFinished);
        connectStatusSignals(job);
        job->start();
    }
}

void EwsResource::itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    qCDebug(EWSRES_AGENTIF_LOG) << "itemsFlagsChanged: start" << items << addedFlags << removedFlags;

    Q_EMIT status(Running, i18nc("@info:status", "Updating item flags"));

    auto job = new EwsModifyItemFlagsJob(mEwsClient, this, items, addedFlags, removedFlags);
    connect(job, &EwsModifyItemFlagsJob::result, this, &EwsResource::itemModifyFlagsRequestFinished);
    connectStatusSignals(job);
    job->start();
}

void EwsResource::itemModifyFlagsRequestFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(EWSRES_AGENTIF_LOG) << "itemsFlagsChanged:" << job->errorString();
        cancelTask(i18nc("@info:status", "Failed to process item flags update request"));
        return;
    }

    auto req = qobject_cast<EwsModifyItemFlagsJob *>(job);
    if (!req) {
        qCWarning(EWSRES_AGENTIF_LOG) << "itemsFlagsChanged: Invalid EwsModifyItemFlagsJob job object";
        cancelTask(i18nc("@info:status", "Failed to update item flags - internal error"));
        return;
    }

    emitReadyStatus();

    qCDebug(EWSRES_AGENTIF_LOG) << "itemsFlagsChanged: done";
    changesCommitted(req->items());
}

void EwsResource::itemChangeRequestFinished(KJob *job)
{
    if (job->error()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemChanged: " << job->errorString();
        cancelTask(i18nc("@info:status", "Failed to process item update request"));
        return;
    }

    auto req = qobject_cast<EwsModifyItemJob *>(job);
    if (!req) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemChanged: Invalid EwsModifyItemJob job object";
        cancelTask(i18nc("@info:status", "Failed to update item - internal error"));
        return;
    }

    qCDebugNC(EWSRES_AGENTIF_LOG) << "itemChanged: done";
    changesCommitted(req->items());
}

void EwsResource::itemsMoved(const Item::List &items, const Collection &sourceCollection, const Collection &destinationCollection)
{
    qCDebug(EWSRES_AGENTIF_LOG) << "itemsMoved: start" << items << sourceCollection << destinationCollection;

    EwsId::List ids;

    ids.reserve(items.count());
    for (const Item &item : items) {
        EwsId id(item.remoteId(), item.remoteRevision());
        ids.append(id);
    }

    auto req = new EwsMoveItemRequest(mEwsClient, this);
    req->setItemIds(ids);
    EwsId destId(destinationCollection.remoteId(), QString());
    req->setDestinationFolderId(destId);
    req->setProperty("items", QVariant::fromValue<Item::List>(items));
    req->setProperty("sourceCollection", QVariant::fromValue<Collection>(sourceCollection));
    req->setProperty("destinationCollection", QVariant::fromValue<Collection>(destinationCollection));
    connect(req, &KJob::result, this, &EwsResource::itemMoveRequestFinished);
    req->start();
}

void EwsResource::itemMoveRequestFinished(KJob *job)
{
    if (job->error()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsMoved: " << job->errorString();
        cancelTask(i18nc("@info:status", "Failed to process item move request"));
        return;
    }

    auto req = qobject_cast<EwsMoveItemRequest *>(job);
    if (!req) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsMoved: Invalid EwsMoveItemRequest job object";
        cancelTask(i18nc("@info:status", "Failed to move item - internal error"));
        return;
    }
    auto items = job->property("items").value<Item::List>();

    if (items.count() != req->responses().count()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsMoved: Invalid number of responses received from server";
        cancelTask(i18nc("@info:status", "Failed to move item - invalid number of responses received from server"));
        return;
    }

    /* When moving a batch of items it is possible that the operation will fail for some of them.
     * Unfortunately Akonadi doesn't provide a way to report such partial success/failure. In order
     * to work around this in case of partial failure the source and destination folders will be
     * resynchronised. In order to avoid doing a full sync a hint will be provided in order to
     * indicate the item(s) to check.
     */

    Item::List movedItems;
    EwsId::List failedIds;

    auto srcCol = req->property("sourceCollection").value<Collection>();
    auto dstCol = req->property("destinationCollection").value<Collection>();
    Item::List::iterator it = items.begin();
    const auto reqResponses{req->responses()};
    for (const EwsMoveItemRequest::Response &resp : reqResponses) {
        Item &item = *it;
        if (resp.isSuccess()) {
            qCDebugNC(EWSRES_AGENTIF_LOG)
                << QStringLiteral("itemsMoved: succeeded for item %1 (new id: %2)").arg(ewsHash(item.remoteId()), ewsHash(resp.itemId().id()));
            if (item.isValid()) {
                item.setRemoteId(resp.itemId().id());
                item.setRemoteRevision(resp.itemId().changeKey());
                movedItems.append(item);
            }
        } else {
            Q_EMIT warning(QStringLiteral("Move failed for item %1").arg(item.remoteId()));
            qCDebugNC(EWSRES_AGENTIF_LOG) << QStringLiteral("itemsMoved: failed for item %1").arg(ewsHash(item.remoteId()));
            failedIds.append(EwsId(item.remoteId(), QString()));
        }
        ++it;
    }

    if (!failedIds.isEmpty()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to move %1 items. Forcing src & dst folder sync.").arg(failedIds.size());
        mItemsToCheck[srcCol.remoteId()] += failedIds;
        foldersModifiedEvent(EwsId::List({EwsId(srcCol.remoteId(), QString())}));
        mItemsToCheck[dstCol.remoteId()] += failedIds;
        foldersModifiedEvent(EwsId::List({EwsId(dstCol.remoteId(), QString())}));
    }

    qCDebugNC(EWSRES_AGENTIF_LOG) << "itemsMoved: done";
    changesCommitted(movedItems);
}

void EwsResource::itemsRemoved(const Item::List &items)
{
    qCDebugNC(EWSRES_AGENTIF_LOG) << "itemsRemoved: start" << items;

    EwsId::List ids;
    ids.reserve(items.count());
    for (const Item &item : items) {
        EwsId id(item.remoteId(), item.remoteRevision());
        ids.append(id);
    }

    auto req = new EwsDeleteItemRequest(mEwsClient, this);
    req->setItemIds(ids);
    req->setProperty("items", QVariant::fromValue<Item::List>(items));
    connect(req, &EwsDeleteItemRequest::result, this, &EwsResource::itemDeleteRequestFinished);
    req->start();
}

void EwsResource::itemDeleteRequestFinished(KJob *job)
{
    if (job->error()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsRemoved: " << job->errorString();
        cancelTask(i18nc("@info:status", "Failed to process item delete request"));
        return;
    }

    auto req = qobject_cast<EwsDeleteItemRequest *>(job);
    if (!req) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsRemoved: Invalid EwsDeleteItemRequest job object";
        cancelTask(i18nc("@info:status", "Failed to delete item - internal error"));
        return;
    }
    auto items = job->property("items").value<Item::List>();

    if (items.count() != req->responses().count()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << "itemsRemoved: Invalid number of responses received from server";
        cancelTask(i18nc("@info:status", "Failed to delete item - invalid number of responses received from server"));
        return;
    }

    /* When removing a batch of items it is possible that the operation will fail for some of them.
     * Unfortunately Akonadi doesn't provide a way to report such partial success/failure. In order
     * to work around this in case of partial failure the original folder(s) will be resynchronised.
     * In order to avoid doing a full sync a hint will be provided in order to indicate the item(s)
     * to check.
     */

    EwsId::List foldersToSync;

    Item::List::iterator it = items.begin();

    const auto reqResponses{req->responses()};
    for (const EwsDeleteItemRequest::Response &resp : reqResponses) {
        Item &item = *it;
        if (resp.isSuccess()) {
            qCDebugNC(EWSRES_AGENTIF_LOG) << QStringLiteral("itemsRemoved: succeeded for item %1").arg(ewsHash(item.remoteId()));
        } else {
            Q_EMIT warning(QStringLiteral("Delete failed for item %1").arg(item.remoteId()));
            qCWarningNC(EWSRES_AGENTIF_LOG) << QStringLiteral("itemsRemoved: failed for item %1").arg(ewsHash(item.remoteId()));
            EwsId colId = EwsId(item.parentCollection().remoteId(), QString());
            mItemsToCheck[colId.id()].append(EwsId(item.remoteId(), QString()));
            if (!foldersToSync.contains(colId)) {
                foldersToSync.append(colId);
            }
        }
        ++it;
    }

    if (!foldersToSync.isEmpty()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Need to force sync for %1 folders.").arg(foldersToSync.size());
        foldersModifiedEvent(foldersToSync);
    }

    qCDebug(EWSRES_AGENTIF_LOG) << "itemsRemoved: done";
    changeProcessed();
}

void EwsResource::itemAdded(const Item &item, const Collection &collection)
{
    EwsItemType type = EwsItemHandler::mimeToItemType(item.mimeType());
    if (isEwsMessageItemType(type)) {
        cancelTask(i18nc("@info:status", "Item type not supported for creation"));
    } else {
        EwsCreateItemJob *job = EwsItemHandler::itemHandler(type)->createItemJob(mEwsClient, item, collection, mTagStore, this);
        connect(job, &EwsCreateItemJob::result, this, &EwsResource::itemCreateRequestFinished);
        job->start();
    }
}

void EwsResource::itemCreateRequestFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process item create request"));
        return;
    }

    auto req = qobject_cast<EwsCreateItemJob *>(job);
    if (!req) {
        cancelTask(i18nc("@info:status", "Failed to create item - internal error"));
        return;
    }

    changeCommitted(req->item());
}

void EwsResource::collectionAdded(const Collection &collection, const Collection &parent)
{
    EwsFolderType type;
    QStringList mimeTypes = collection.contentMimeTypes();
    if (mimeTypes.contains(EwsItemHandler::itemHandler(EwsItemTypeCalendarItem)->mimeType())) {
        type = EwsFolderTypeCalendar;
    } else if (mimeTypes.contains(EwsItemHandler::itemHandler(EwsItemTypeContact)->mimeType())) {
        type = EwsFolderTypeContacts;
    } else if (mimeTypes.contains(EwsItemHandler::itemHandler(EwsItemTypeTask)->mimeType())) {
        type = EwsFolderTypeTasks;
    } else if (mimeTypes.contains(EwsItemHandler::itemHandler(EwsItemTypeMessage)->mimeType())) {
        type = EwsFolderTypeMail;
    } else {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Cannot determine EWS folder type.");
        cancelTask(i18nc("@info:status", "Failed to add collection - cannot determine EWS folder type"));
        return;
    }

    EwsFolder folder;
    folder.setType(type);
    folder.setField(EwsFolderFieldDisplayName, collection.name());

    auto req = new EwsCreateFolderRequest(mEwsClient, this);
    req->setParentFolderId(EwsId(parent.remoteId()));
    req->setFolders(EwsFolder::List() << folder);
    req->setProperty("collection", QVariant::fromValue<Collection>(collection));
    connect(req, &EwsCreateFolderRequest::result, this, &EwsResource::folderCreateRequestFinished);
    req->start();
}

void EwsResource::folderCreateRequestFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process folder create request"));
        return;
    }

    auto req = qobject_cast<EwsCreateFolderRequest *>(job);
    if (!req) {
        cancelTask(i18nc("@info:status", "Failed to create folder - internal error"));
        return;
    }
    auto col = job->property("collection").value<Collection>();

    EwsCreateFolderRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        const EwsId &id = resp.folderId();
        col.setRemoteId(id.id());
        col.setRemoteRevision(id.changeKey());
        changeCommitted(col);
    } else {
        cancelTask(i18nc("@info:status", "Failed to create folder"));
    }
}

void EwsResource::collectionMoved(const Collection &collection, const Collection &collectionSource, const Collection &collectionDestination)
{
    Q_UNUSED(collectionSource)

    EwsId::List ids;
    ids.append(EwsId(collection.remoteId(), collection.remoteRevision()));

    auto req = new EwsMoveFolderRequest(mEwsClient, this);
    req->setFolderIds(ids);
    EwsId destId(collectionDestination.remoteId());
    req->setDestinationFolderId(destId);
    req->setProperty("collection", QVariant::fromValue<Collection>(collection));
    connect(req, &EwsMoveFolderRequest::result, this, &EwsResource::folderMoveRequestFinished);
    req->start();
}

void EwsResource::folderMoveRequestFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process folder move request"));
        return;
    }

    auto req = qobject_cast<EwsMoveFolderRequest *>(job);
    if (!req) {
        cancelTask(i18nc("@info:status", "Failed to move folder - internal error"));
        return;
    }
    auto col = job->property("collection").value<Collection>();

    if (req->responses().count() != 1) {
        cancelTask(i18nc("@info:status", "Failed to move folder - invalid number of responses received from server"));
        return;
    }

    EwsMoveFolderRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        const EwsId &id = resp.folderId();
        col.setRemoteId(id.id());
        col.setRemoteRevision(id.changeKey());
        changeCommitted(col);
    } else {
        cancelTask(i18nc("@info:status", "Failed to move folder"));
    }
}

void EwsResource::collectionChanged(const Collection &collection, const QSet<QByteArray> &changedAttributes)
{
    if (changedAttributes.contains("NAME")) {
        auto req = new EwsUpdateFolderRequest(mEwsClient, this);
        EwsUpdateFolderRequest::FolderChange fc(EwsId(collection.remoteId(), collection.remoteRevision()), EwsFolderTypeMail);
        EwsUpdateFolderRequest::Update *upd = new EwsUpdateFolderRequest::SetUpdate(EwsPropertyField(QStringLiteral("folder:DisplayName")), collection.name());
        fc.addUpdate(upd);
        req->addFolderChange(fc);
        req->setProperty("collection", QVariant::fromValue<Collection>(collection));
        connect(req, &EwsUpdateFolderRequest::finished, this, &EwsResource::folderUpdateRequestFinished);
        req->start();
    } else {
        changeCommitted(collection);
    }
}

void EwsResource::collectionChanged(const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)
}

void EwsResource::folderUpdateRequestFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process folder update request"));
        return;
    }

    auto req = qobject_cast<EwsUpdateFolderRequest *>(job);
    if (!req) {
        cancelTask(i18nc("@info:status", "Failed to update folder - internal error"));
        return;
    }
    auto col = job->property("collection").value<Collection>();

    if (req->responses().count() != 1) {
        cancelTask(i18nc("@info:status", "Failed to update folder - invalid number of responses received from server"));
        return;
    }

    EwsUpdateFolderRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        const EwsId &id = resp.folderId();
        col.setRemoteId(id.id());
        col.setRemoteRevision(id.changeKey());
        changeCommitted(col);
    } else {
        cancelTask(i18nc("@info:status", "Failed to update folder"));
    }
}

void EwsResource::collectionRemoved(const Collection &collection)
{
    auto req = new EwsDeleteFolderRequest(mEwsClient, this);
    EwsId::List ids;
    ids.append(EwsId(collection.remoteId(), collection.remoteRevision()));
    req->setFolderIds(ids);
    connect(req, &EwsDeleteFolderRequest::result, this, &EwsResource::folderDeleteRequestFinished);
    req->start();
}

void EwsResource::folderDeleteRequestFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process folder delete request"));
        return;
    }

    auto req = qobject_cast<EwsDeleteFolderRequest *>(job);
    if (!req) {
        cancelTask(i18nc("@info:status", "Failed to delete folder - internal error"));
        return;
    }

    EwsDeleteFolderRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        changeProcessed();
    } else {
        cancelTask(i18nc("@info:status", "Failed to delete folder"));
        mFolderSyncState.clear();
        synchronizeCollectionTree();
    }
}

void EwsResource::sendItem(const Akonadi::Item &item)
{
    EwsItemType type = EwsItemHandler::mimeToItemType(item.mimeType());
    if (isEwsMessageItemType(type)) {
        itemSent(item, TransportFailed, i18nc("@info:status", "Item type not supported for creation"));
    } else {
        EwsCreateItemJob *job = EwsItemHandler::itemHandler(type)->createItemJob(mEwsClient, item, Collection(), mTagStore, this);
        job->setSend(true);
        job->setProperty("item", QVariant::fromValue<Item>(item));
        connect(job, &EwsCreateItemJob::result, this, &EwsResource::itemSendRequestFinished);
        job->start();
    }
}

void EwsResource::itemSendRequestFinished(KJob *job)
{
    Item item = job->property("item").value<Item>();
    if (job->error()) {
        itemSent(item, TransportFailed, i18nc("@info:status", "Failed to process item send request"));
        return;
    }

    auto req = qobject_cast<EwsCreateItemJob *>(job);
    if (!req) {
        itemSent(item, TransportFailed, i18nc("@info:status", "Failed to send item - internal error"));
        return;
    }

    itemSent(item, TransportSucceeded);
}

void EwsResource::sendMessage(const QString &id, const QByteArray &content)
{
#if HAVE_SEPARATE_MTA_RESOURCE
    auto req = new EwsCreateItemRequest(mEwsClient, this);

    EwsItem item;
    item.setType(EwsItemTypeMessage);
    item.setField(EwsItemFieldMimeContent, content);
    req->setItems(EwsItem::List() << item);
    req->setMessageDisposition(EwsDispSendOnly);
    req->setProperty("requestId", id);
    connect(req, &EwsCreateItemRequest::finished, this, &EwsResource::messageSendRequestFinished);
    req->start();
#endif
}

#if HAVE_SEPARATE_MTA_RESOURCE
void EwsResource::messageSendRequestFinished(KJob *job)
{
    QString id = job->property("requestId").toString();
    if (job->error()) {
        Q_EMIT messageSent(id, i18nc("@info:status", "Failed to process item send request"));
        return;
    }

    auto req = qobject_cast<EwsCreateItemRequest *>(job);
    if (!req) {
        Q_EMIT messageSent(id, i18nc("@info:status", "Failed to send item - internal error"));
        return;
    }

    if (req->responses().count() != 1) {
        Q_EMIT messageSent(id, i18nc("@info:status", "Invalid number of responses received from server"));
        return;
    }

    EwsCreateItemRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        Q_EMIT messageSent(id, QString());
    } else {
        Q_EMIT messageSent(id, resp.responseMessage());
    }
}

#endif

void EwsResource::foldersModifiedEvent(const EwsId::List &folders)
{
    for (const EwsId &id : folders) {
        Collection c;
        c.setRemoteId(id.id());
        auto job = new CollectionFetchJob(c, CollectionFetchJob::Base);
        job->setFetchScope(changeRecorder()->collectionFetchScope());
        job->fetchScope().setResource(identifier());
        job->fetchScope().setListFilter(CollectionFetchScope::Sync);
        connect(job, &KJob::result, this, &EwsResource::foldersModifiedCollectionSyncFinished);
    }
}

void EwsResource::foldersModifiedCollectionSyncFinished(KJob *job)
{
    if (job->error()) {
        qCDebug(EWSRES_LOG) << QStringLiteral("Failed to fetch collection tree for sync.");
        return;
    }

    auto fetchColJob = qobject_cast<CollectionFetchJob *>(job);
    const auto collection = fetchColJob->collections().at(0);
    queueFetchItemsJob(collection, SubscriptionSync, [this](EwsFetchItemsJob *fetchJob) {
        auto collection = fetchJob->collection();
        if (fetchJob->error()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Item fetch error:") << fetchJob->errorString() << fetchJob->error();
            synchronizeCollection(collection.id());
        } else {
            const auto newItems = fetchJob->newItems();
            for (const auto &newItem : newItems) {
                new ItemCreateJob(newItem, collection, this);
            }
            if (!fetchJob->changedItems().isEmpty()) {
                new ItemModifyJob(fetchJob->changedItems());
            }
            if (!fetchJob->deletedItems().isEmpty()) {
                new ItemDeleteJob(fetchJob->deletedItems());
            }
            saveCollectionSyncState(collection, fetchJob->syncState());
            emitReadyStatus();
        }
    });
}

void EwsResource::folderTreeModifiedEvent()
{
    synchronizeCollectionTree();
}

void EwsResource::fullSyncRequestedEvent()
{
    synchronize();
}

void EwsResource::clearCollectionSyncState(int collectionId)
{
    Collection col(collectionId);
    auto attr = col.attribute<EwsSyncStateAttribute>();
    col.addAttribute(attr);
    auto job = new CollectionModifyJob(col);
    job->start();
}

void EwsResource::clearFolderTreeSyncState()
{
    mFolderSyncState.clear();
    saveState();
}

void EwsResource::fetchSpecialFolders()
{
    auto job = new CollectionFetchJob(mRootCollection, CollectionFetchJob::Recursive, this);
    connect(job, &CollectionFetchJob::collectionsReceived, this, &EwsResource::specialFoldersCollectionsRetrieved);
    connect(job, &CollectionFetchJob::result, this, [](KJob *job) {
        if (job->error()) {
            qCWarningNC(EWSRES_LOG) << "Special folders fetch failed:" << job->errorString();
        }
    });
    job->start();
}

void EwsResource::specialFoldersCollectionsRetrieved(const Collection::List &folders)
{
    EwsId::List queryItems;

    queryItems.reserve(specialFolderList.count());
    for (const SpecialFolders &sf : std::as_const(specialFolderList)) {
        queryItems.append(EwsId(sf.did));
    }

    if (!queryItems.isEmpty()) {
        auto req = new EwsGetFolderRequest(mEwsClient, this);
        req->setFolderShape(EwsFolderShape(EwsShapeIdOnly));
        req->setFolderIds(queryItems);
        req->setProperty("collections", QVariant::fromValue<Collection::List>(folders));
        connect(req, &EwsGetFolderRequest::finished, this, &EwsResource::specialFoldersFetchFinished);
        req->start();
    }
}

void EwsResource::specialFoldersFetchFinished(KJob *job)
{
    if (job->error()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Special collection fetch failed:") << job->errorString();
        return;
    }

    auto req = qobject_cast<EwsGetFolderRequest *>(job);
    if (!req) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Special collection fetch failed:") << QStringLiteral("Invalid EwsGetFolderRequest job object");
        return;
    }

    const auto collections = req->property("collections").value<Collection::List>();

    if (req->responses().size() != specialFolderList.size()) {
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Special collection fetch failed:") << QStringLiteral("Invalid number of responses received");
        return;
    }

    QMap<QString, Collection> map;
    for (const Collection &col : collections) {
        map.insert(col.remoteId(), col);
    }

    auto it = specialFolderList.cbegin();
    const auto responses{req->responses()};
    for (const EwsGetFolderRequest::Response &resp : responses) {
        if (resp.isSuccess()) {
            auto fid = resp.folder()[EwsFolderFieldFolderId].value<EwsId>();
            QMap<QString, Collection>::iterator mapIt = map.find(fid.id());
            if (mapIt != map.end()) {
                qCDebugNC(EWSRES_LOG)
                    << QStringLiteral("Registering folder %1(%2) as special collection %3").arg(ewsHash(mapIt->remoteId())).arg(mapIt->id()).arg(it->type);
                SpecialMailCollections::self()->registerCollection(it->type, *mapIt);
                if (!mapIt->hasAttribute<EntityDisplayAttribute>()) {
                    auto attr = mapIt->attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
                    attr->setIconName(it->iconName);
                    auto modJob = new CollectionModifyJob(*mapIt, this);
                    modJob->start();
                }
            }
        }
        it++;
    }
}

void EwsResource::saveState()
{
    QByteArray str;
    QDataStream dataStream(&str, QIODevice::WriteOnly);
    mSettings->setFolderSyncState(QString::fromLatin1(qCompress(mFolderSyncState.toLatin1(), 9).toBase64()));
    mSettings->save();
}

void EwsResource::doSetOnline(bool online)
{
    if (online) {
        reloadConfig();
    } else {
        mSubManager.reset(nullptr);
    }
}

int EwsResource::reconnectTimeout()
{
    int timeout = mReconnectTimeout;
    if (mReconnectTimeout < MaxReconnectTimeout) {
        mReconnectTimeout *= 2;
    }
    return timeout;
}

void EwsResource::itemsTagsChanged(const Item::List &items, const QSet<Tag> &addedTags, const QSet<Tag> &removedTags)
{
    Q_UNUSED(addedTags)
    Q_UNUSED(removedTags)

    Q_EMIT status(Running, i18nc("@info:status", "Updating item tags"));

    auto job = new EwsUpdateItemsTagsJob(items, mTagStore, mEwsClient, this);
    connect(job, &EwsUpdateItemsTagsJob::result, this, &EwsResource::itemsTagChangeFinished);
    connectStatusSignals(job);
    job->start();
}

void EwsResource::itemsTagChangeFinished(KJob *job)
{
    emitReadyStatus();

    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process item tags update request"));
        return;
    }

    auto updJob = qobject_cast<EwsUpdateItemsTagsJob *>(job);
    if (!updJob) {
        cancelTask(i18nc("@info:status", "Failed to update item tags - internal error"));
        return;
    }

    changesCommitted(updJob->items());
}

void EwsResource::tagAdded(const Tag &tag)
{
    mTagStore->addTag(tag);

    auto job = new EwsGlobalTagsWriteJob(mTagStore, mEwsClient, mRootCollection, this);
    connect(job, &EwsGlobalTagsWriteJob::result, this, &EwsResource::globalTagChangeFinished);
    job->start();
}

void EwsResource::tagChanged(const Tag &tag)
{
    mTagStore->addTag(tag);

    auto job = new EwsGlobalTagsWriteJob(mTagStore, mEwsClient, mRootCollection, this);
    connect(job, &EwsGlobalTagsWriteJob::result, this, &EwsResource::globalTagChangeFinished);
    job->start();
}

void EwsResource::tagRemoved(const Tag &tag)
{
    mTagStore->removeTag(tag);

    auto job = new EwsGlobalTagsWriteJob(mTagStore, mEwsClient, mRootCollection, this);
    connect(job, &EwsGlobalTagsWriteJob::result, this, &EwsResource::globalTagChangeFinished);
    job->start();
}

void EwsResource::globalTagChangeFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process global tag update request"));
    } else {
        changeProcessed();
    }
}

void EwsResource::retrieveTags()
{
    auto job = new EwsGlobalTagsReadJob(mTagStore, mEwsClient, mRootCollection, this);
    connect(job, &EwsGlobalTagsReadJob::result, this, &EwsResource::globalTagsRetrievalFinished);
    job->start();
}

void EwsResource::globalTagsRetrievalFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18nc("@info:status", "Failed to process global tags retrieval request"));
    } else {
        auto readJob = qobject_cast<EwsGlobalTagsReadJob *>(job);
        Q_ASSERT(readJob);
        tagsRetrieved(readJob->tags(), QHash<QString, Item::List>());
    }
}

void EwsResource::setUpAuth()
{
    EwsAbstractAuth *auth = mSettings->loadAuth(this);

    /* Use queued connections here to avoid stack overflow when the reauthentication proceeds through all stages. */
    connect(auth, &EwsAbstractAuth::authSucceeded, this, &EwsResource::authSucceeded, Qt::QueuedConnection);
    connect(auth, &EwsAbstractAuth::authFailed, this, &EwsResource::authFailed, Qt::QueuedConnection);
    connect(auth, &EwsAbstractAuth::requestAuthFailed, this, &EwsResource::requestAuthFailed, Qt::QueuedConnection);

    qCDebugNC(EWSRES_LOG) << QStringLiteral("Initializing authentication");

    mAuth.reset(auth);

    auth->init();
}

void EwsResource::authSucceeded()
{
    if (mAuthStage != AuthIdle) {
        setOnline(true);
    }

    mAuthStage = AuthIdle;

    resetUrl();
}

void EwsResource::reauthNotificationDismissed(bool accepted)
{
    if (mReauthNotification) {
        mReauthNotification.clear();
        if (accepted) {
            mAuth->authenticate(true);
        } else {
            authFailed(QStringLiteral("Interactive authentication request denied"));
        }
    }
}

void EwsResource::authFailed(const QString &error)
{
    qCWarningNC(EWSRES_LOG) << "Authentication failed: " << error;

    reauthenticate();
}

void EwsResource::reauthenticate()
{
    switch (mAuthStage) {
    case AuthIdle:
        mAuthStage = AuthRefreshToken;
        qCWarningNC(EWSRES_LOG) << "reauthenticate: trying to refresh";
        if (mAuth->authenticate(false)) {
            break;
        }
    /* fall through */
    case AuthRefreshToken: {
        mAuthStage = AuthAccessToken;
        const auto reauthPrompt = mAuth->reauthPrompt();
        if (!reauthPrompt.isNull()) {
            mReauthNotification = new KNotification(QStringLiteral("auth-expired"), KNotification::Persistent, this);

            mReauthNotification->setText(reauthPrompt.arg(name()));
            mReauthNotification->setActions(QStringList(i18nc("@action:button", "Authenticate")));
            mReauthNotification->setComponentName(QStringLiteral("akonadi_ews_resource"));
            auto acceptedFn = std::bind(&EwsResource::reauthNotificationDismissed, this, true);
            auto rejectedFn = std::bind(&EwsResource::reauthNotificationDismissed, this, false);
            connect(mReauthNotification.data(), &KNotification::action1Activated, this, acceptedFn);
            connect(mReauthNotification.data(), &KNotification::closed, this, rejectedFn);
            connect(mReauthNotification.data(), &KNotification::ignored, this, rejectedFn);
            mReauthNotification->sendEvent();
            break;
        }
    }
    /* fall through */
    case AuthAccessToken:
        mAuthStage = AuthFailure;
        Q_EMIT status(Broken, i18nc("@info:status", "Authentication failed"));
        break;
    case AuthFailure:
        break;
    }
}

void EwsResource::requestAuthFailed()
{
    qCWarningNC(EWSRES_LOG) << "requestAuthFailed - going offline";

    if (mAuthStage == AuthIdle) {
        QTimer::singleShot(0, this, [&]() {
            setTemporaryOffline(reconnectTimeout());
        });
        Q_EMIT status(Broken, i18nc("@info:status", "Authentication failed"));

        reauthenticate();
    }
}

void EwsResource::emitReadyStatus()
{
    Q_EMIT status(Idle, i18nc("@info:status Resource is ready", "Ready"));
    Q_EMIT percent(0);
}

void EwsResource::adjustRootCollectionName(const QString &newName)
{
    if (mRootCollection.isValid()) {
        auto attr = mRootCollection.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
        if (attr->displayName() != newName) {
            attr->setDisplayName(newName);
            new CollectionModifyJob(mRootCollection);
        }
    }
}

void EwsResource::setInitialReconnectTimeout(int timeout)
{
    mInitialReconnectTimeout = mReconnectTimeout = timeout;
}

template<class Job>
void EwsResource::connectStatusSignals(Job *job)
{
    connect(job, &Job::reportStatus, this, [this](int s, const QString &message) {
        Q_EMIT status(s, message);
    });
    connect(job, &Job::reportPercent, this, [this](int p) {
        Q_EMIT percent(p);
    });
}

QString EwsResource::getCollectionSyncState(const Akonadi::Collection &col)
{
    auto attr = col.attribute<EwsSyncStateAttribute>();
    return attr ? attr->syncState() : QString();
}

void EwsResource::saveCollectionSyncState(Akonadi::Collection &col, const QString &state)
{
    col.addAttribute(new EwsSyncStateAttribute(state));
    auto job = new CollectionModifyJob(col);
    job->start();
}

QString EwsResource::dumpResourceToString() const
{
    QString dump = QStringLiteral("item sync queue (%1):\n").arg(mFetchItemsJobQueue.count());

    for (const auto &item : std::as_const(mFetchItemsJobQueue)) {
        dump += QStringLiteral(" %1:%2\n").arg(item.col.id()).arg(item.type);
    }

    return dump;
}

AKONADI_RESOURCE_MAIN(EwsResource)
