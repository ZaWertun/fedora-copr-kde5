/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "storecompactjob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::StoreCompactJobPrivate
{
public:
    explicit StoreCompactJobPrivate(FileStore::StoreCompactJob *parent)
        : mParent(parent)
    {
    }

public:
    FileStore::StoreCompactJob *mParent = nullptr;

    Collection::List mCollections;
    Item::List mItems;
};

FileStore::StoreCompactJob::StoreCompactJob(FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new StoreCompactJobPrivate(this))
{
    session->addJob(this);
}

FileStore::StoreCompactJob::~StoreCompactJob() = default;

bool FileStore::StoreCompactJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

Item::List FileStore::StoreCompactJob::changedItems() const
{
    return d->mItems;
}

Collection::List FileStore::StoreCompactJob::changedCollections() const
{
    return d->mCollections;
}

void FileStore::StoreCompactJob::handleCollectionsChanged(const Collection::List &collections)
{
    d->mCollections << collections;
    Q_EMIT collectionsChanged(collections);
}

void FileStore::StoreCompactJob::handleItemsChanged(const Item::List &items)
{
    d->mItems << items;
    Q_EMIT itemsChanged(items);
}
