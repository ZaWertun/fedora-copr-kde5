/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itemmodifyjob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::ItemModifyJobPrivate
{
public:
    ItemModifyJobPrivate()
        : mIgnorePayload(false)
    {
    }

    bool mIgnorePayload;
    Item mItem;
    QSet<QByteArray> mParts;
};

FileStore::ItemModifyJob::ItemModifyJob(const Item &item, FileStore::AbstractJobSession *session)
    : FileStore::Job(session)
    , d(new ItemModifyJobPrivate())
{
    d->mItem = item;

    session->addJob(this);
}

FileStore::ItemModifyJob::~ItemModifyJob() = default;

void FileStore::ItemModifyJob::setIgnorePayload(bool ignorePayload)
{
    d->mIgnorePayload = ignorePayload;
}

bool FileStore::ItemModifyJob::ignorePayload() const
{
    return d->mIgnorePayload;
}

Item FileStore::ItemModifyJob::item() const
{
    return d->mItem;
}

void FileStore::ItemModifyJob::setParts(const QSet<QByteArray> &parts)
{
    d->mParts = parts;
}

const QSet<QByteArray> &FileStore::ItemModifyJob::parts() const
{
    return d->mParts;
}

bool FileStore::ItemModifyJob::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}

void FileStore::ItemModifyJob::handleItemModified(const Item &item)
{
    d->mItem = item;
}
