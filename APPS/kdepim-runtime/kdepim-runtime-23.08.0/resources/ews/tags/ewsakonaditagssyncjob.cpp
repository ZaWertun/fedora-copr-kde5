/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsakonaditagssyncjob.h"

#include "ewsglobaltagswritejob.h"
#include "ewstagstore.h"
#include <Akonadi/TagFetchJob>
#include <Akonadi/TagFetchScope>

using namespace Akonadi;

EwsAkonadiTagsSyncJob::EwsAkonadiTagsSyncJob(EwsTagStore *tagStore, EwsClient &client, const Collection &rootCollection, QObject *parent)
    : EwsJob(parent)
    , mTagStore(tagStore)
    , mClient(client)
    , mRootCollection(rootCollection)
{
}

EwsAkonadiTagsSyncJob::~EwsAkonadiTagsSyncJob() = default;

void EwsAkonadiTagsSyncJob::start()
{
    auto job = new TagFetchJob(this);
    job->fetchScope().setFetchRemoteId(true);
    connect(job, &TagFetchJob::result, this, &EwsAkonadiTagsSyncJob::tagFetchFinished);
}

void EwsAkonadiTagsSyncJob::tagFetchFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorString());
        emitResult();
        return;
    }

    auto tagJob = qobject_cast<TagFetchJob *>(job);
    Q_ASSERT(tagJob);

    if (mTagStore->syncTags(tagJob->tags())) {
        auto tagJob = new EwsGlobalTagsWriteJob(mTagStore, mClient, mRootCollection, this);
        connect(tagJob, &EwsGlobalTagsWriteJob::result, this, &EwsAkonadiTagsSyncJob::tagWriteFinished);
        tagJob->start();
    } else {
        emitResult();
    }
}

void EwsAkonadiTagsSyncJob::tagWriteFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorString());
    }

    emitResult();
}
