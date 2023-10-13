/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2011 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrieveitemsjob.h"

#include "mixedmaildir_debug.h"
#include "mixedmaildirstore.h"

#include "filestore/itemfetchjob.h"

#include <Akonadi/MessageParts>
#include <Akonadi/MessageStatus>

#include <Akonadi/Collection>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/TransactionSequence>
#include <Akonadi/VectorHelper>

#include "mixedmaildirresource_debug.h"

#include <QDateTime>
#include <QQueue>
#include <QVariant>

using namespace Akonadi;

enum {
    MaxItemCreateJobs = 100,
    MaxItemModifyJobs = 100,
};

class RetrieveItemsJobPrivate
{
    RetrieveItemsJob *const q;

public:
    RetrieveItemsJobPrivate(RetrieveItemsJob *parent, const Collection &collection, MixedMaildirStore *store)
        : q(parent)
        , mCollection(collection)
        , mStore(store)
    {
    }

    TransactionSequence *transaction()
    {
        if (!mTransaction) {
            mTransaction = new TransactionSequence(q);
            mTransaction->setAutomaticCommittingEnabled(false);
            QObject::connect(mTransaction, &TransactionSequence::result, q, [this](KJob *job) {
                transactionResult(job);
            });
        }
        return mTransaction;
    }

public:
    const Collection mCollection;
    MixedMaildirStore *const mStore;
    TransactionSequence *mTransaction = nullptr;

    QHash<QString, Item> mServerItemsByRemoteId;

    QQueue<Item> mNewItems;
    QQueue<Item> mChangedItems;
    Item::List mAvailableItems;
    Item::List mItemsMarkedAsDeleted;
    qint64 mHighestModTime = -1;
    int mNumItemCreateJobs = 0;
    int mNumItemModifyJobs = 0;

public: // slots
    void akonadiFetchResult(KJob *job);
    void transactionResult(KJob *job);
    void storeListResult(KJob *);
    void processNewItem();
    void fetchNewResult(KJob *);
    void processChangedItem();
    void fetchChangedResult(KJob *);
    void itemCreateJobResult(KJob *);
    void itemModifyJobResult(KJob *);
};

void RetrieveItemsJobPrivate::itemCreateJobResult(KJob *job)
{
    if (job->error()) {
        qCCritical(MIXEDMAILDIR_LOG) << "Error running ItemCreateJob: " << job->errorText();
    }

    mNumItemCreateJobs--;
    QMetaObject::invokeMethod(q, "processNewItem", Qt::QueuedConnection);
}

void RetrieveItemsJobPrivate::itemModifyJobResult(KJob *job)
{
    if (job->error()) {
        qCCritical(MIXEDMAILDIR_LOG) << "Error running ItemModifyJob: " << job->errorText();
    }

    mNumItemModifyJobs--;
    QMetaObject::invokeMethod(q, "processChangedItem", Qt::QueuedConnection);
}

void RetrieveItemsJobPrivate::akonadiFetchResult(KJob *job)
{
    if (job->error() != 0) {
        return; // handled by base class
    }

    auto itemFetch = qobject_cast<ItemFetchJob *>(job);
    Q_ASSERT(itemFetch != nullptr);

    Item::List items = itemFetch->items();
    itemFetch->clearItems(); // save memory
    qCDebug(MIXEDMAILDIR_LOG) << "Akonadi fetch got" << items.count() << "items";

    mServerItemsByRemoteId.reserve(items.size());
    for (int i = 0; i < items.count(); ++i) {
        Item &item = items[i];
        // items without remoteId have not been written to the resource yet
        if (!item.remoteId().isEmpty()) {
            // set the parent collection (with all ancestors) in every item
            item.setParentCollection(mCollection);
            mServerItemsByRemoteId.insert(item.remoteId(), item);
        }
    }

    qCDebug(MIXEDMAILDIR_LOG) << "of which" << mServerItemsByRemoteId.count() << "have remoteId";

    FileStore::ItemFetchJob *storeFetch = mStore->fetchItems(mCollection);
    // just basic items, no data

    QObject::connect(storeFetch, &FileStore::ItemFetchJob::result, q, [this](KJob *job) {
        storeListResult(job);
    });
}

void RetrieveItemsJobPrivate::storeListResult(KJob *job)
{
    qCDebug(MIXEDMAILDIRRESOURCE_LOG) << "storeList->error=" << job->error();
    auto storeList = qobject_cast<FileStore::ItemFetchJob *>(job);
    Q_ASSERT(storeList != nullptr);

    if (storeList->error() != 0) {
        q->setError(storeList->error());
        q->setErrorText(storeList->errorText());
        q->emitResult();
        return;
    }

    // if some items have tags, we need to complete the retrieval and schedule tagging
    // to a later time so we can then fetch the items to get their Akonadi URLs
    // forward the property to this instance so the resource can take care of that
    const QVariant var = storeList->property("remoteIdToTagList");
    if (var.isValid()) {
        q->setProperty("remoteIdToTagList", var);
    }

    const qint64 collectionTimestamp = mCollection.remoteRevision().toLongLong();

    const Item::List storedItems = storeList->items();
    for (const Item &item : storedItems) {
        // messages marked as deleted have been deleted from mbox files but never got purged
        Akonadi::MessageStatus status;
        status.setStatusFromFlags(item.flags());
        if (status.isDeleted()) {
            mItemsMarkedAsDeleted << item;
            continue;
        }

        mAvailableItems << item;

        const QHash<QString, Item>::iterator it = mServerItemsByRemoteId.find(item.remoteId());
        if (it == mServerItemsByRemoteId.end()) {
            // item not in server items -> new
            mNewItems << item;
        } else {
            // item both on server and in store, check modification time
            const QDateTime modTime = item.modificationTime();
            if (!modTime.isValid() || modTime.toMSecsSinceEpoch() > collectionTimestamp) {
                mChangedItems << it.value();
            }

            // remove from hash so only no longer existing items remain
            mServerItemsByRemoteId.erase(it);
        }
    }

    qCDebug(MIXEDMAILDIR_LOG) << "Store fetch got" << storedItems.count() << "items"
                              << "of which" << mNewItems.count() << "are new and" << mChangedItems.count() << "are changed and"
                              << mServerItemsByRemoteId.count() << "need to be removed";

    // all items remaining in mServerItemsByRemoteId are no longer in the store

    if (!mServerItemsByRemoteId.isEmpty()) {
        auto deleteJob = new ItemDeleteJob(Akonadi::valuesToVector(mServerItemsByRemoteId), transaction());
        transaction()->setIgnoreJobFailure(deleteJob);
    }

    processNewItem();
}

void RetrieveItemsJobPrivate::processNewItem()
{
    if (mNewItems.isEmpty()) {
        processChangedItem();
        return;
    }

    const Item item = mNewItems.dequeue();
    FileStore::ItemFetchJob *storeFetch = mStore->fetchItem(item);
    storeFetch->fetchScope().fetchPayloadPart(MessagePart::Envelope);

    QObject::connect(storeFetch, &FileStore::ItemFetchJob::result, q, [this](KJob *job) {
        fetchNewResult(job);
    });
}

void RetrieveItemsJobPrivate::fetchNewResult(KJob *job)
{
    auto fetchJob = qobject_cast<FileStore::ItemFetchJob *>(job);
    Q_ASSERT(fetchJob != nullptr);

    if (fetchJob->items().count() != 1) {
        const Item item = fetchJob->item();
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Store fetch for new item" << item.remoteId() << "in collection" << item.parentCollection().id() << ","
                                            << item.parentCollection().remoteId() << "did not return the expected item. error=" << fetchJob->error() << ","
                                            << fetchJob->errorText();
        processNewItem();
        return;
    }

    const Item item = fetchJob->items().at(0);
    const QDateTime modTime = item.modificationTime();
    if (modTime.isValid()) {
        mHighestModTime = qMax(modTime.toMSecsSinceEpoch(), mHighestModTime);
    }

    auto itemCreate = new ItemCreateJob(item, mCollection, transaction());
    mNumItemCreateJobs++;
    QObject::connect(itemCreate, &ItemCreateJob::result, q, [this](KJob *job) {
        itemCreateJobResult(job);
    });

    if (mNumItemCreateJobs < MaxItemCreateJobs) {
        QMetaObject::invokeMethod(q, "processNewItem", Qt::QueuedConnection);
    }
}

void RetrieveItemsJobPrivate::processChangedItem()
{
    if (mChangedItems.isEmpty()) {
        if (!mTransaction) {
            // no jobs created here -> done
            q->emitResult();
            return;
        }

        if (mHighestModTime > -1) {
            Collection collection(mCollection);
            collection.setRemoteRevision(QString::number(mHighestModTime));
            auto job = new CollectionModifyJob(collection, transaction());
            transaction()->setIgnoreJobFailure(job);
        }
        transaction()->commit();
        return;
    }

    const Item item = mChangedItems.dequeue();
    FileStore::ItemFetchJob *storeFetch = mStore->fetchItem(item);
    storeFetch->fetchScope().fetchPayloadPart(MessagePart::Envelope);

    QObject::connect(storeFetch, &FileStore::ItemFetchJob::result, q, [this](KJob *job) {
        fetchChangedResult(job);
    });
}

void RetrieveItemsJobPrivate::fetchChangedResult(KJob *job)
{
    auto fetchJob = qobject_cast<FileStore::ItemFetchJob *>(job);
    Q_ASSERT(fetchJob != nullptr);

    if (fetchJob->items().count() != 1) {
        const Item item = fetchJob->item();
        qCWarning(MIXEDMAILDIRRESOURCE_LOG) << "Store fetch for changed item" << item.remoteId() << "in collection" << item.parentCollection().id() << ","
                                            << item.parentCollection().remoteId() << "did not return the expected item. error=" << fetchJob->error() << ","
                                            << fetchJob->errorText();
        processChangedItem();
        return;
    }

    const Item item = fetchJob->items().at(0);
    const QDateTime modTime = item.modificationTime();
    if (modTime.isValid()) {
        mHighestModTime = qMax(modTime.toMSecsSinceEpoch(), mHighestModTime);
    }

    auto itemModify = new ItemModifyJob(item, transaction());
    QObject::connect(itemModify, &ItemModifyJob::result, q, [this](KJob *job) {
        itemModifyJobResult(job);
    });
    mNumItemModifyJobs++;
    if (mNumItemModifyJobs < MaxItemModifyJobs) {
        QMetaObject::invokeMethod(q, "processChangedItem", Qt::QueuedConnection);
    }
}

void RetrieveItemsJobPrivate::transactionResult(KJob *job)
{
    if (job->error() != 0) {
        return; // handled by base class
    }

    q->emitResult();
}

RetrieveItemsJob::RetrieveItemsJob(const Akonadi::Collection &collection, MixedMaildirStore *store, QObject *parent)
    : Job(parent)
    , d(new RetrieveItemsJobPrivate(this, collection, store))
{
    Q_ASSERT(d->mCollection.isValid());
    Q_ASSERT(!d->mCollection.remoteId().isEmpty());
    Q_ASSERT(d->mStore != nullptr);
}

RetrieveItemsJob::~RetrieveItemsJob() = default;

Collection RetrieveItemsJob::collection() const
{
    return d->mCollection;
}

Item::List RetrieveItemsJob::availableItems() const
{
    return d->mAvailableItems;
}

Item::List RetrieveItemsJob::itemsMarkedAsDeleted() const
{
    return d->mItemsMarkedAsDeleted;
}

void RetrieveItemsJob::doStart()
{
    auto job = new Akonadi::ItemFetchJob(d->mCollection, this);
    connect(job, &ItemFetchJob::result, this, [this](KJob *job) {
        d->akonadiFetchResult(job);
    });
}

#include "moc_retrieveitemsjob.cpp"
