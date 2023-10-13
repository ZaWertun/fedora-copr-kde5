/*
    SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gidmigrationjob.h"
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>

using namespace Akonadi;

UpdateJob::UpdateJob(const Collection &col, QObject *parent)
    : Job(parent)
    , mCollection(col)
    , mModJobRunning(false)
{
}

UpdateJob::~UpdateJob() = default;

void UpdateJob::doStart()
{
    auto fetchJob = new ItemFetchJob(mCollection, this);
    fetchJob->fetchScope().setCacheOnly(true);
    fetchJob->fetchScope().setIgnoreRetrievalErrors(true);
    fetchJob->fetchScope().setFetchModificationTime(false);
    fetchJob->fetchScope().setFetchRemoteIdentification(false);
    fetchJob->fetchScope().fetchFullPayload(true);
    // Limit scope to envelope only for mail
    connect(fetchJob, &ItemFetchJob::itemsReceived, this, &UpdateJob::itemsReceived);
}

void UpdateJob::itemsReceived(const Akonadi::Item::List &items)
{
    // We're queuing items rather than creating ItemModifyJobs directly due to memory concerns
    // I'm not sure if that would indeed be a problem (a ModifyJob shouldn't be much larger than the item) but we'd have to compare memory usage first when
    // creating large amounts of ItemModifyJobs.
    for (const Akonadi::Item &item : items) {
        mItemQueue.enqueue(item);
    }
    processNext();
}

void UpdateJob::slotResult(KJob *job)
{
    // This slot is automatically called for all subjobs by KCompositeJob
    // FIXME the fetch job emits result before itemsReceived, because itemsReceived is triggered using the result signal (which is wrong IMO). See
    // ItemFetchJob::timeout If result was emitted at the end we could avoid having to call processNext in itemsReceived and locking it.
    auto const fetchJob = qobject_cast<ItemFetchJob *>(job);
    const bool fetchReturnedNoItems = fetchJob && fetchJob->items().isEmpty();
    Job::slotResult(job);
    if (fetchReturnedNoItems) {
        emitResult();
    } else if (!fetchJob) {
        mModJobRunning = false;
        if (!hasSubjobs()) {
            if (!processNext()) {
                emitResult();
            }
        }
    }
}

bool UpdateJob::processNext()
{
    if (mModJobRunning || mItemQueue.isEmpty()) {
        return false;
    }
    const Akonadi::Item &item = mItemQueue.dequeue();
    // Only the single item modifyjob updates the gid
    auto modJob = new ItemModifyJob(item, this);
    modJob->setUpdateGid(true);
    modJob->setIgnorePayload(true);
    mModJobRunning = true;
    return true;
}

GidMigrationJob::GidMigrationJob(const QStringList &mimeTypeFilter, QObject *parent)
    : Job(parent)
    , mMimeTypeFilter(mimeTypeFilter)
{
}

GidMigrationJob::~GidMigrationJob() = default;

void GidMigrationJob::doStart()
{
    auto fetchJob = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
    fetchJob->fetchScope().setContentMimeTypes(mMimeTypeFilter);
    connect(fetchJob, &CollectionFetchJob::collectionsReceived, this, &GidMigrationJob::collectionsReceived);
    connect(fetchJob, &CollectionFetchJob::result, this, &GidMigrationJob::collectionsFetched);
}

void GidMigrationJob::collectionsReceived(const Collection::List &collections)
{
    mCollections << collections;
}

void GidMigrationJob::collectionsFetched(KJob *job)
{
    // Errors are propagated by KCompositeJob
    if (!job->error()) {
        processCollection();
    }
}

void GidMigrationJob::processCollection()
{
    if (mCollections.isEmpty()) {
        emitResult();
        return;
    }
    const Collection col = mCollections.takeLast();
    auto updateJob = new UpdateJob(col, this);
    connect(updateJob, &UpdateJob::result, this, &GidMigrationJob::itemsUpdated);
}

void GidMigrationJob::itemsUpdated(KJob *job)
{
    // Errors are propagated by KCompositeJob
    if (!job->error()) {
        processCollection();
    }
}
