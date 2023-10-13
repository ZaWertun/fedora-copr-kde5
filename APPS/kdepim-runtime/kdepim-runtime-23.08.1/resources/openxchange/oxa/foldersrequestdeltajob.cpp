/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "foldersrequestdeltajob.h"

#include "foldersrequestjob.h"

using namespace OXA;

FoldersRequestDeltaJob::FoldersRequestDeltaJob(qulonglong lastSync, QObject *parent)
    : KJob(parent)
    , mLastSync(lastSync)
{
}

void FoldersRequestDeltaJob::start()
{
    auto modifiedJob = new FoldersRequestJob(mLastSync, FoldersRequestJob::Modified, this);
    connect(modifiedJob, &FoldersRequestJob::result, this, &FoldersRequestDeltaJob::fetchModifiedJobFinished);
    modifiedJob->start();

    auto deletedJob = new FoldersRequestJob(mLastSync, FoldersRequestJob::Deleted, this);
    connect(deletedJob, &FoldersRequestJob::result, this, &FoldersRequestDeltaJob::fetchDeletedJobFinished);
    deletedJob->start();
}

Folder::List FoldersRequestDeltaJob::modifiedFolders() const
{
    return mModifiedFolders;
}

Folder::List FoldersRequestDeltaJob::deletedFolders() const
{
    return mDeletedFolders;
}

void FoldersRequestDeltaJob::fetchModifiedJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const FoldersRequestJob *requestJob = qobject_cast<FoldersRequestJob *>(job);

    mModifiedFolders << requestJob->folders();

    mJobFinishedCount++;

    if (mJobFinishedCount == 2) {
        emitResult();
    }
}

void FoldersRequestDeltaJob::fetchDeletedJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const FoldersRequestJob *requestJob = qobject_cast<FoldersRequestJob *>(job);

    mDeletedFolders << requestJob->folders();

    mJobFinishedCount++;

    if (mJobFinishedCount == 2) {
        emitResult();
    }
}
