/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectiondeletejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::CollectionDeleteJobPrivate
{
public:
    Collection mCollection;
};

FileStore::CollectionDeleteJob::CollectionDeleteJob(const Collection &collection, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new CollectionDeleteJobPrivate())
{
    Q_ASSERT(session != nullptr);

    d->mCollection = collection;

    session->addJob(this);
}

FileStore::CollectionDeleteJob::~CollectionDeleteJob() = default;

Collection FileStore::CollectionDeleteJob::collection() const
{
    return d->mCollection;
}

bool FileStore::CollectionDeleteJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::CollectionDeleteJob::handleCollectionDeleted(const Collection &collection)
{
    d->mCollection = collection;
}
