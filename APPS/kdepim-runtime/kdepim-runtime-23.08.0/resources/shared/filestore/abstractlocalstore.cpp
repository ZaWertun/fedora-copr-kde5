/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractlocalstore.h"

#include "akonadifilestore_debug.h"
#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionmovejob.h"
#include "itemcreatejob.h"
#include "itemdeletejob.h"
#include "itemfetchjob.h"
#include "itemmodifyjob.h"
#include "itemmovejob.h"
#include "sessionimpls_p.h"
#include "storecompactjob.h"

#include <Akonadi/EntityDisplayAttribute>

#include <KLocalizedString>
#include <QFileInfo>

using namespace Akonadi;

class JobProcessingAdaptor : public FileStore::Job::Visitor
{
public:
    explicit JobProcessingAdaptor(FileStore::AbstractJobSession *session)
        : mSession(session)
    {
    }

public: // Job::Visitor interface implementation
    bool visit(FileStore::Job *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::CollectionCreateJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::CollectionDeleteJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::CollectionFetchJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::CollectionModifyJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::CollectionMoveJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::ItemCreateJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::ItemDeleteJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::ItemFetchJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::ItemModifyJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::ItemMoveJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

    bool visit(FileStore::StoreCompactJob *job) override
    {
        Q_UNUSED(job)
        return false;
    }

protected:
    FileStore::AbstractJobSession *mSession = nullptr;
};

class TopLevelCollectionFetcher : public JobProcessingAdaptor
{
public:
    explicit TopLevelCollectionFetcher(FileStore::AbstractJobSession *session)
        : JobProcessingAdaptor(session)
    {
    }

    void setTopLevelCollection(const Collection &collection)
    {
        mTopLevelCollection = collection;
    }

public:
    using JobProcessingAdaptor::visit;

    bool visit(FileStore::CollectionFetchJob *job) override
    {
        if (job->type() == FileStore::CollectionFetchJob::Base && job->collection().remoteId() == mTopLevelCollection.remoteId()) {
            mSession->notifyCollectionsReceived(job, Collection::List() << mTopLevelCollection);
            return true;
        }

        return false;
    }

private:
    Collection mTopLevelCollection;
};

class CollectionsProcessedNotifier : public JobProcessingAdaptor
{
public:
    explicit CollectionsProcessedNotifier(FileStore::AbstractJobSession *session)
        : JobProcessingAdaptor(session)
    {
    }

    void setCollections(const Collection::List &collections)
    {
        mCollections = collections;
    }

public:
    using JobProcessingAdaptor::visit;

    bool visit(FileStore::CollectionCreateJob *job) override
    {
        Q_ASSERT(!mCollections.isEmpty());
        if (mCollections.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing collections for CollectionCreateJob "
                                                "encountered more than one collection. Just processing the first one.";
        }

        mSession->notifyCollectionCreated(job, mCollections[0]);
        return true;
    }

    bool visit(FileStore::CollectionDeleteJob *job) override
    {
        Q_ASSERT(!mCollections.isEmpty());
        if (mCollections.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing collections for CollectionDeleteJob "
                                                "encountered more than one collection. Just processing the first one.";
        }

        mSession->notifyCollectionDeleted(job, mCollections[0]);
        return true;
    }

    bool visit(FileStore::CollectionFetchJob *job) override
    {
        mSession->notifyCollectionsReceived(job, mCollections);
        return true;
    }

    bool visit(FileStore::CollectionModifyJob *job) override
    {
        Q_ASSERT(!mCollections.isEmpty());
        if (mCollections.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing collections for CollectionModifyJob "
                                                "encountered more than one collection. Just processing the first one.";
        }

        mSession->notifyCollectionModified(job, mCollections[0]);
        return true;
    }

    bool visit(FileStore::CollectionMoveJob *job) override
    {
        Q_ASSERT(!mCollections.isEmpty());
        if (mCollections.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing collections for CollectionMoveJob "
                                                "encountered more than one collection. Just processing the first one.";
        }

        mSession->notifyCollectionMoved(job, mCollections[0]);
        return true;
    }

    bool visit(FileStore::StoreCompactJob *job) override
    {
        mSession->notifyCollectionsChanged(job, mCollections);
        return true;
    }

private:
    Collection::List mCollections;
};

class ItemsProcessedNotifier : public JobProcessingAdaptor
{
public:
    explicit ItemsProcessedNotifier(FileStore::AbstractJobSession *session)
        : JobProcessingAdaptor(session)
    {
    }

    void setItems(const Item::List &items)
    {
        mItems = items;
    }

    void clearItems()
    {
        mItems.clear();
    }

public:
    using JobProcessingAdaptor::visit;

    bool visit(FileStore::ItemCreateJob *job) override
    {
        Q_ASSERT(!mItems.isEmpty());
        if (mItems.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing items for ItemCreateJob encountered more than one item. "
                                                "Just processing the first one.";
        }

        mSession->notifyItemCreated(job, mItems[0]);
        return true;
    }

    bool visit(FileStore::ItemFetchJob *job) override
    {
        mSession->notifyItemsReceived(job, mItems);
        return true;
    }

    bool visit(FileStore::ItemModifyJob *job) override
    {
        Q_ASSERT(!mItems.isEmpty());
        if (mItems.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing items for ItemModifyJob encountered more than one item. "
                                                "Just processing the first one.";
        }

        mSession->notifyItemModified(job, mItems[0]);
        return true;
    }

    bool visit(FileStore::ItemMoveJob *job) override
    {
        Q_ASSERT(!mItems.isEmpty());
        if (mItems.count() > 1) {
            qCCritical(AKONADIFILESTORE_LOG) << "Processing items for ItemMoveJob encountered more than one item. "
                                                "Just processing the first one.";
        }

        mSession->notifyItemMoved(job, mItems[0]);
        return true;
    }

    bool visit(FileStore::StoreCompactJob *job) override
    {
        mSession->notifyItemsChanged(job, mItems);
        return true;
    }

private:
    Item::List mItems;
};

class FileStore::AbstractLocalStorePrivate
{
    AbstractLocalStore *const q;

public:
    explicit AbstractLocalStorePrivate(FileStore::AbstractLocalStore *parent)
        : q(parent)
        , mSession(new FileStore::FiFoQueueJobSession(q))
        , mCurrentJob(nullptr)
        , mTopLevelCollectionFetcher(mSession)
        , mCollectionsProcessedNotifier(mSession)
        , mItemsProcessedNotifier(mSession)
    {
    }

public:
    QFileInfo mPathFileInfo;
    Collection mTopLevelCollection;

    FileStore::AbstractJobSession *mSession = nullptr;
    FileStore::Job *mCurrentJob = nullptr;

    TopLevelCollectionFetcher mTopLevelCollectionFetcher;
    CollectionsProcessedNotifier mCollectionsProcessedNotifier;
    ItemsProcessedNotifier mItemsProcessedNotifier;

public:
    void processJobs(const QList<FileStore::Job *> &jobs);
};

void FileStore::AbstractLocalStorePrivate::processJobs(const QList<FileStore::Job *> &jobs)
{
    for (FileStore::Job *job : jobs) {
        mCurrentJob = job;

        if (job->error() == 0) {
            if (!job->accept(&mTopLevelCollectionFetcher)) {
                q->processJob(job);
            }
        }
        mSession->emitResult(job);
        mCurrentJob = nullptr;
    }
}

FileStore::AbstractLocalStore::AbstractLocalStore()
    : QObject()
    , d(new AbstractLocalStorePrivate(this))
{
    connect(d->mSession, &AbstractJobSession::jobsReady, this, [this](const QList<FileStore::Job *> &jobs) {
        d->processJobs(jobs);
    });
}

FileStore::AbstractLocalStore::~AbstractLocalStore() = default;

void FileStore::AbstractLocalStore::setPath(const QString &path)
{
    QFileInfo pathFileInfo(path);
    if (pathFileInfo.fileName().isEmpty()) {
        pathFileInfo = QFileInfo(pathFileInfo.path());
    }
    pathFileInfo.makeAbsolute();

    if (pathFileInfo.absoluteFilePath() == d->mPathFileInfo.absoluteFilePath()) {
        return;
    }

    d->mPathFileInfo = pathFileInfo;

    Collection collection;
    collection.setRemoteId(d->mPathFileInfo.absoluteFilePath());
    collection.setName(d->mPathFileInfo.fileName());

    auto attribute = collection.attribute<EntityDisplayAttribute>();
    if (attribute) {
        attribute->setDisplayName(d->mPathFileInfo.fileName());
    }

    setTopLevelCollection(collection);
}

QString FileStore::AbstractLocalStore::path() const
{
    return d->mPathFileInfo.absoluteFilePath();
}

Collection FileStore::AbstractLocalStore::topLevelCollection() const
{
    return d->mTopLevelCollection;
}

FileStore::CollectionCreateJob *FileStore::AbstractLocalStore::createCollection(const Collection &collection, const Collection &targetParent)
{
    auto job = new FileStore::CollectionCreateJob(collection, targetParent, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (targetParent.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((targetParent.rights() & Collection::CanCreateCollection) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits folder creation in folder %1", targetParent.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkCollectionCreate(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::CollectionDeleteJob *FileStore::AbstractLocalStore::deleteCollection(const Collection &collection)
{
    auto job = new FileStore::CollectionDeleteJob(collection, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty() || collection.parentCollection().remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((collection.rights() & Collection::CanDeleteCollection) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits folder deletion in folder %1", collection.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkCollectionDelete(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::CollectionFetchJob *FileStore::AbstractLocalStore::fetchCollections(const Collection &collection, FileStore::CollectionFetchJob::Type type) const
{
    auto job = new FileStore::CollectionFetchJob(collection, type, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << "FetchType=" << type;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << "FetchType=" << type;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkCollectionFetch(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::CollectionModifyJob *FileStore::AbstractLocalStore::modifyCollection(const Collection &collection)
{
    auto job = new FileStore::CollectionModifyJob(collection, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((collection.rights() & Collection::CanChangeCollection) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits folder modification in folder %1", collection.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkCollectionModify(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::CollectionMoveJob *FileStore::AbstractLocalStore::moveCollection(const Collection &collection, const Collection &targetParent)
{
    auto job = new FileStore::CollectionMoveJob(collection, targetParent, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty() || collection.parentCollection().remoteId().isEmpty() || targetParent.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((targetParent.rights() & Collection::CanCreateCollection) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits folder creation in folder %1", targetParent.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkCollectionMove(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemFetchJob *FileStore::AbstractLocalStore::fetchItems(const Collection &collection) const
{
    auto job = new FileStore::ItemFetchJob(collection, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkItemFetch(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemFetchJob *FileStore::AbstractLocalStore::fetchItems(const Item::List &items) const
{
    auto job = new FileStore::ItemFetchJob(items, d->mSession);

    if (items.size() == 1) {
        const Akonadi::Item &item = items[0];
        if (d->mTopLevelCollection.remoteId().isEmpty()) {
            const QString message = i18nc("@info:status", "Configured storage location is empty");
            qCCritical(AKONADIFILESTORE_LOG) << message;
            qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                             << ", parentCollection=" << item.parentCollection().remoteId() << ")";
            d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
        } else if (item.remoteId().isEmpty()) {
            const QString message = i18nc("@info:status", "Given item identifier is empty");
            qCCritical(AKONADIFILESTORE_LOG) << message;
            qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                             << ", parentCollection=" << item.parentCollection().remoteId() << ")";
            d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
        }
    }

    int errorCode = 0;
    QString errorText;
    checkItemFetch(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemFetchJob *FileStore::AbstractLocalStore::fetchItem(const Item &item) const
{
    return fetchItems({item});
}

FileStore::ItemCreateJob *FileStore::AbstractLocalStore::createItem(const Item &item, const Collection &collection)
{
    auto job = new FileStore::ItemCreateJob(item, collection, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (collection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((collection.rights() & Collection::CanCreateItem) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits item creation in folder %1", collection.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << collection << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkItemCreate(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemModifyJob *FileStore::AbstractLocalStore::modifyItem(const Item &item)
{
    auto job = new FileStore::ItemModifyJob(item, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (item.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given item identifier is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((item.parentCollection().rights() & Collection::CanChangeItem) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits item modification in folder %1", item.parentCollection().name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkItemModify(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemDeleteJob *FileStore::AbstractLocalStore::deleteItem(const Item &item)
{
    auto job = new FileStore::ItemDeleteJob(item, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (item.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given item identifier is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((item.parentCollection().rights() & Collection::CanDeleteItem) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits item deletion in folder %1", item.parentCollection().name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")";
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkItemDelete(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::ItemMoveJob *FileStore::AbstractLocalStore::moveItem(const Item &item, const Collection &targetParent)
{
    auto job = new FileStore::ItemMoveJob(item, targetParent, d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")" << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    } else if (item.parentCollection().remoteId().isEmpty() || targetParent.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given folder name is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")" << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((targetParent.rights() & Collection::CanCreateItem) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits item creation in folder %1", targetParent.name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")" << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if ((item.parentCollection().rights() & Collection::CanDeleteItem) == 0) {
        const QString message = i18nc("@info:status", "Access control prohibits item deletion in folder %1", item.parentCollection().name());
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")" << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    } else if (item.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Given item identifier is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        qCCritical(AKONADIFILESTORE_LOG) << "Item(remoteId=" << item.remoteId() << ", mimeType=" << item.mimeType()
                                         << ", parentCollection=" << item.parentCollection().remoteId() << ")" << targetParent;
        d->mSession->setError(job, FileStore::Job::InvalidJobContext, message);
    }

    int errorCode = 0;
    QString errorText;
    checkItemMove(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::StoreCompactJob *FileStore::AbstractLocalStore::compactStore()
{
    auto job = new FileStore::StoreCompactJob(d->mSession);

    if (d->mTopLevelCollection.remoteId().isEmpty()) {
        const QString message = i18nc("@info:status", "Configured storage location is empty");
        qCCritical(AKONADIFILESTORE_LOG) << message;
        d->mSession->setError(job, FileStore::Job::InvalidStoreState, message);
    }

    int errorCode = 0;
    QString errorText;
    checkStoreCompact(job, errorCode, errorText);
    if (errorCode != 0) {
        d->mSession->setError(job, errorCode, errorText);
    }

    return job;
}

FileStore::Job *FileStore::AbstractLocalStore::currentJob() const
{
    return d->mCurrentJob;
}

void FileStore::AbstractLocalStore::notifyError(int errorCode, const QString &errorText) const
{
    Q_ASSERT(d->mCurrentJob != nullptr);

    d->mSession->setError(d->mCurrentJob, errorCode, errorText);
}

void FileStore::AbstractLocalStore::notifyCollectionsProcessed(const Collection::List &collections) const
{
    Q_ASSERT(d->mCurrentJob != nullptr);

    d->mCollectionsProcessedNotifier.setCollections(collections);
    d->mCurrentJob->accept(&(d->mCollectionsProcessedNotifier));
}

void FileStore::AbstractLocalStore::notifyItemsProcessed(const Item::List &items) const
{
    Q_ASSERT(d->mCurrentJob != nullptr);

    d->mItemsProcessedNotifier.setItems(items);
    d->mCurrentJob->accept(&(d->mItemsProcessedNotifier));
    d->mItemsProcessedNotifier.clearItems(); // save memory
}

void FileStore::AbstractLocalStore::setTopLevelCollection(const Collection &collection)
{
    d->mTopLevelCollection = collection;
    d->mTopLevelCollectionFetcher.setTopLevelCollection(collection);
}

void FileStore::AbstractLocalStore::checkCollectionCreate(FileStore::CollectionCreateJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkCollectionDelete(FileStore::CollectionDeleteJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkCollectionFetch(FileStore::CollectionFetchJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkCollectionModify(FileStore::CollectionModifyJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkCollectionMove(FileStore::CollectionMoveJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkItemCreate(FileStore::ItemCreateJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkItemDelete(FileStore::ItemDeleteJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkItemFetch(FileStore::ItemFetchJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkItemModify(FileStore::ItemModifyJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkItemMove(FileStore::ItemMoveJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

void FileStore::AbstractLocalStore::checkStoreCompact(FileStore::StoreCompactJob *job, int &errorCode, QString &errorText) const
{
    Q_UNUSED(job)
    Q_UNUSED(errorCode)
    Q_UNUSED(errorText)
}

#include "moc_abstractlocalstore.cpp"
