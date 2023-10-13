/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-filestore_export.h"

#include <QVector>

// TODO not nice, collection fetch type should probably be in its own header
#include "collectionfetchjob.h"

namespace Akonadi
{
class Collection;
class Item;

namespace FileStore
{
class CollectionCreateJob;
class CollectionDeleteJob;
class CollectionFetchJob;
class CollectionModifyJob;
class CollectionMoveJob;
class ItemCreateJob;
class ItemDeleteJob;
class ItemFetchJob;
class ItemModifyJob;
class ItemMoveJob;
class StoreCompactJob;

/**
 */
class AKONADI_FILESTORE_EXPORT StoreInterface
{
public:
    virtual ~StoreInterface() = default;

    virtual Collection topLevelCollection() const = 0;

    virtual CollectionCreateJob *createCollection(const Collection &collection, const Collection &targetParent) = 0;

    virtual CollectionFetchJob *fetchCollections(const Collection &collection, CollectionFetchJob::Type type = CollectionFetchJob::FirstLevel) const = 0;

    virtual CollectionDeleteJob *deleteCollection(const Collection &collection) = 0;

    virtual CollectionModifyJob *modifyCollection(const Collection &collection) = 0;

    virtual CollectionMoveJob *moveCollection(const Collection &collection, const Collection &targetParent) = 0;

    virtual ItemFetchJob *fetchItems(const Collection &collection) const = 0;

    virtual ItemFetchJob *fetchItems(const QVector<Item> &items) const = 0;

    virtual ItemFetchJob *fetchItem(const Item &item) const = 0;

    virtual ItemCreateJob *createItem(const Item &item, const Collection &collection) = 0;

    virtual ItemModifyJob *modifyItem(const Item &item) = 0;

    virtual ItemDeleteJob *deleteItem(const Item &item) = 0;

    virtual ItemMoveJob *moveItem(const Item &item, const Collection &targetParent) = 0;

    virtual StoreCompactJob *compactStore() = 0;

protected:
    virtual void setTopLevelCollection(const Collection &collection) = 0;
};
}
}
