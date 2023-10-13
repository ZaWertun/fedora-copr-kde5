/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectionmodifyjob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::CollectionModifyJobPrivate
{
public:
    Collection mCollection;
};

FileStore::CollectionModifyJob::CollectionModifyJob(const Collection &collection, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new CollectionModifyJobPrivate())
{
    Q_ASSERT(session != nullptr);

    d->mCollection = collection;

    session->addJob(this);
}

FileStore::CollectionModifyJob::~CollectionModifyJob() = default;

Collection FileStore::CollectionModifyJob::collection() const
{
    return d->mCollection;
}

bool FileStore::CollectionModifyJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::CollectionModifyJob::handleCollectionModified(const Collection &collection)
{
    d->mCollection = collection;
}
