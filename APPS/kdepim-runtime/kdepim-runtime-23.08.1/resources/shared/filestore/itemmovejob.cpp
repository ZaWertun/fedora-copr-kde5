/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemmovejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::ItemMoveJobPrivate
{
public:
    Item mItem;
    Collection mTargetParent;
};

FileStore::ItemMoveJob::ItemMoveJob(const Item &item, const Collection &targetParent, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemMoveJobPrivate())
{
    d->mItem = item;
    d->mTargetParent = targetParent;

    session->addJob(this);
}

FileStore::ItemMoveJob::~ItemMoveJob() = default;

Collection FileStore::ItemMoveJob::targetParent() const
{
    return d->mTargetParent;
}

Item FileStore::ItemMoveJob::item() const
{
    return d->mItem;
}

bool FileStore::ItemMoveJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::ItemMoveJob::handleItemMoved(const Item &item)
{
    d->mItem = item;
}
