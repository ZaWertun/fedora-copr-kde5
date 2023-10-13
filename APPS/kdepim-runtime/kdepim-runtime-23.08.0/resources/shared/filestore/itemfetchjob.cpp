/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemfetchjob.h"

#include "session_p.h"

#include <Akonadi/ItemFetchScope>

using namespace Akonadi;

class FileStore::ItemFetchJobPrivate
{
public:
    ItemFetchScope mFetchScope;

    Item::List mRequestedItems;
    Item::List mResultItems;

    Collection mCollection;
    Item mItem;
};

FileStore::ItemFetchJob::ItemFetchJob(const Collection &collection, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemFetchJobPrivate())
{
    d->mCollection = collection;

    session->addJob(this);
}

FileStore::ItemFetchJob::ItemFetchJob(const Item &item, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemFetchJobPrivate())
{
    d->mItem = item;

    session->addJob(this);
}

FileStore::ItemFetchJob::ItemFetchJob(const Item::List &items, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemFetchJobPrivate())
{
    d->mRequestedItems = items;

    session->addJob(this);
}

FileStore::ItemFetchJob::~ItemFetchJob() = default;

Collection FileStore::ItemFetchJob::collection() const
{
    return d->mCollection;
}

Item FileStore::ItemFetchJob::item() const
{
    return d->mItem;
}

void FileStore::ItemFetchJob::setFetchScope(const ItemFetchScope &fetchScope)
{
    d->mFetchScope = fetchScope;
}

ItemFetchScope &FileStore::ItemFetchJob::fetchScope()
{
    return d->mFetchScope;
}

Item::List FileStore::ItemFetchJob::requestedItems() const
{
    return d->mRequestedItems;
}

Item::List FileStore::ItemFetchJob::items() const
{
    return d->mResultItems;
}

bool FileStore::ItemFetchJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::ItemFetchJob::handleItemsReceived(const Item::List &items)
{
    d->mResultItems << items;

    Q_EMIT itemsReceived(items);
}
