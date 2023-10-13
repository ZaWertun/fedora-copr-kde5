/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectioncreatejob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::CollectionCreateJobPrivate
{
public:
    Collection mCollection;
    Collection mTargetParent;
};

FileStore::CollectionCreateJob::CollectionCreateJob(const Collection &collection, const Collection &targetParent, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new CollectionCreateJobPrivate())
{
    Q_ASSERT(session != nullptr);

    d->mCollection = collection;
    d->mTargetParent = targetParent;

    session->addJob(this);
}

FileStore::CollectionCreateJob::~CollectionCreateJob() = default;

Collection FileStore::CollectionCreateJob::collection() const
{
    return d->mCollection;
}

Collection FileStore::CollectionCreateJob::targetParent() const
{
    return d->mTargetParent;
}

bool FileStore::CollectionCreateJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::CollectionCreateJob::handleCollectionCreated(const Collection &collection)
{
    d->mCollection = collection;
}
