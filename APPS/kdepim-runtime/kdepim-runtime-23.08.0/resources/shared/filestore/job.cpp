/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "job.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::JobPrivate
{
public:
};

FileStore::Job::Job(FileStore::AbstractJobSession *session)
    : KJob(session)
    , d(nullptr /*new Private(this)*/) // nullptr until it's needed
{
    setAutoDelete(true);
}

FileStore::Job::~Job() = default;

void FileStore::Job::start()
{
}

bool FileStore::Job::accept(FileStore::Job::Visitor *visitor)
{
    return visitor->visit(this);
}
