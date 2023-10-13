/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemcreatejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::ItemCreateJobPrivate
{
public:
    Item mItem;
    Collection mCollection;
};

FileStore::ItemCreateJob::ItemCreateJob(const Item &item, const Collection &collection, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemCreateJobPrivate())
{
    d->mItem = item;
    d->mCollection = collection;

    session->addJob(this);
}

FileStore::ItemCreateJob::~ItemCreateJob() = default;

Collection FileStore::ItemCreateJob::collection() const
{
    return d->mCollection;
}

Item FileStore::ItemCreateJob::item() const
{
    return d->mItem;
}

bool FileStore::ItemCreateJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::ItemCreateJob::handleItemCreated(const Item &item)
{
    d->mItem = item;
}
