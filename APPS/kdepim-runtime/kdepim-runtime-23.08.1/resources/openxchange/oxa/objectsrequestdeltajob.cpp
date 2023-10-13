/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectsrequestdeltajob.h"

#include "objectsrequestjob.h"

using namespace OXA;

ObjectsRequestDeltaJob::ObjectsRequestDeltaJob(const Folder &folder, qulonglong lastSync, QObject *parent)
    : KJob(parent)
    , mFolder(folder)
    , mLastSync(lastSync)
    , mJobFinishedCount(0)
{
}

void ObjectsRequestDeltaJob::start()
{
    auto modifiedJob = new ObjectsRequestJob(mFolder, mLastSync, ObjectsRequestJob::Modified, this);
    connect(modifiedJob, &ObjectsRequestJob::result, this, &ObjectsRequestDeltaJob::fetchModifiedJobFinished);
    modifiedJob->start();

    auto deletedJob = new ObjectsRequestJob(mFolder, mLastSync, ObjectsRequestJob::Deleted, this);
    connect(deletedJob, &ObjectsRequestJob::result, this, &ObjectsRequestDeltaJob::fetchDeletedJobFinished);
    deletedJob->start();
}

Object::List ObjectsRequestDeltaJob::modifiedObjects() const
{
    return mModifiedObjects;
}

Object::List ObjectsRequestDeltaJob::deletedObjects() const
{
    return mDeletedObjects;
}

void ObjectsRequestDeltaJob::fetchModifiedJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const ObjectsRequestJob *requestJob = qobject_cast<ObjectsRequestJob *>(job);

    mModifiedObjects << requestJob->objects();

    mJobFinishedCount++;

    if (mJobFinishedCount == 2) {
        emitResult();
    }
}

void ObjectsRequestDeltaJob::fetchDeletedJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const ObjectsRequestJob *requestJob = qobject_cast<ObjectsRequestJob *>(job);

    mDeletedObjects << requestJob->objects();

    mJobFinishedCount++;

    if (mJobFinishedCount == 2) {
        emitResult();
    }
}
