/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemdeletejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::ItemDeleteJobPrivate
{
public:
    Item mItem;
};

FileStore::ItemDeleteJob::ItemDeleteJob(const Item &item, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemDeleteJobPrivate())
{
    d->mItem = item;

    session->addJob(this);
}

FileStore::ItemDeleteJob::~ItemDeleteJob() = default;

Item FileStore::ItemDeleteJob::item() const
{
    return d->mItem;
}

bool FileStore::ItemDeleteJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::ItemDeleteJob::handleItemDeleted(const Item &item)
{
    d->mItem = item;
}
