/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "useridrequestjob.h"

#include "davmanager.h"
#include "foldersrequestjob.h"

using namespace OXA;

UserIdRequestJob::UserIdRequestJob(QObject *parent)
    : KJob(parent)
    , mUserId(-1)
{
}

void UserIdRequestJob::start()
{
    auto job = new FoldersRequestJob(0, FoldersRequestJob::Modified, this);
    connect(job, &FoldersRequestJob::result, this, &UserIdRequestJob::davJobFinished);

    job->start();
}

qlonglong UserIdRequestJob::userId() const
{
    return mUserId;
}

void UserIdRequestJob::davJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    auto requestJob = qobject_cast<FoldersRequestJob *>(job);
    Q_ASSERT(requestJob);

    const Folder::List folders = requestJob->folders();
    for (const Folder &folder : folders) {
        if (folder.folderId() == 1) {
            // Found folder with 'Private Folders' as parent, so the owner must
            // be the user that is currently logged in.
            mUserId = folder.owner();
            break;
        }
    }

    if (mUserId == -1) {
        setError(UserDefinedError);
        setErrorText(QStringLiteral("No private folder found"));
    }

    emitResult();
}
