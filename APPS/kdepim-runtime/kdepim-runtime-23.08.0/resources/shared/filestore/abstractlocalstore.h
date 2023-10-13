/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "storeinterface.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QObject>

#include <memory>

namespace Akonadi
{
namespace FileStore
{
class AbstractLocalStorePrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT AbstractLocalStore : public QObject, public StoreInterface
{
    Q_OBJECT

public:
    AbstractLocalStore();
    ~AbstractLocalStore() override;

    virtual void setPath(const QString &path);
    QString path() const;

    Collection topLevelCollection() const override;

    CollectionCreateJob *createCollection(const Collection &collection, const Collection &targetParent) override;

    CollectionFetchJob *fetchCollections(const Collection &collection, CollectionFetchJob::Type type = CollectionFetchJob::FirstLevel) const override;

    CollectionDeleteJob *deleteCollection(const Collection &collection) override;

    CollectionModifyJob *modifyCollection(const Collection &collection) override;

    CollectionMoveJob *moveCollection(const Collection &collection, const Collection &targetParent) override;

    ItemFetchJob *fetchItems(const Collection &collection) const override;

    ItemFetchJob *fetchItems(const Item::List &items) const override;

    ItemFetchJob *fetchItem(const Item &item) const override;

    ItemCreateJob *createItem(const Item &item, const Collection &collection) override;

    ItemModifyJob *modifyItem(const Item &item) override;

    ItemDeleteJob *deleteItem(const Item &item) override;

    ItemMoveJob *moveItem(const Item &item, const Collection &targetParent) override;

    StoreCompactJob *compactStore() override;

protected: // job processing
    virtual void processJob(Job *job) = 0;

    Job *currentJob() const;

    void notifyError(int errorCode, const QString &errorText) const;

    void notifyCollectionsProcessed(const Collection::List &collections) const;

    void notifyItemsProcessed(const Item::List &items) const;

protected: // template methods
    void setTopLevelCollection(const Collection &collection) override;

    virtual void checkCollectionCreate(CollectionCreateJob *job, int &errorCode, QString &errorText) const;

    virtual void checkCollectionDelete(CollectionDeleteJob *job, int &errorCode, QString &errorText) const;

    virtual void checkCollectionFetch(CollectionFetchJob *job, int &errorCode, QString &errorText) const;

    virtual void checkCollectionModify(CollectionModifyJob *job, int &errorCode, QString &errorText) const;

    virtual void checkCollectionMove(CollectionMoveJob *job, int &errorCode, QString &errorText) const;

    virtual void checkItemCreate(ItemCreateJob *job, int &errorCode, QString &errorText) const;

    virtual void checkItemDelete(ItemDeleteJob *job, int &errorCode, QString &errorText) const;

    virtual void checkItemFetch(ItemFetchJob *job, int &errorCode, QString &errorText) const;

    virtual void checkItemModify(ItemModifyJob *job, int &errorCode, QString &errorText) const;

    virtual void checkItemMove(ItemMoveJob *job, int &errorCode, QString &errorText) const;

    virtual void checkStoreCompact(StoreCompactJob *job, int &errorCode, QString &errorText) const;

private:
    friend class AbstractLocalStorePrivate;
    std::unique_ptr<AbstractLocalStorePrivate> const d;
};
}
}
