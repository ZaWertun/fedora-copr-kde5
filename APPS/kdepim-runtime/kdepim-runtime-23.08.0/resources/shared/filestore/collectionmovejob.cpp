/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectionmovejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::CollectionMoveJobPrivate
{
public:
    Collection mCollection;
    Collection mTargetParent;
};

FileStore::CollectionMoveJob::CollectionMoveJob(const Collection &collection, const Collection &targetParent, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new CollectionMoveJobPrivate())
{
    Q_ASSERT(session != nullptr);

    d->mCollection = collection;
    d->mTargetParent = targetParent;

    session->addJob(this);
}

FileStore::CollectionMoveJob::~CollectionMoveJob() = default;

Collection FileStore::CollectionMoveJob::collection() const
{
    return d->mCollection;
}

Collection FileStore::CollectionMoveJob::targetParent() const
{
    return d->mTargetParent;
}

bool FileStore::CollectionMoveJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::CollectionMoveJob::handleCollectionMoved(const Collection &collection)
{
    d->mCollection = collection;
}
