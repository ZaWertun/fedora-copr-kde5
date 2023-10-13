/*
 *  SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "createandsettagsjob.h"

#include <Akonadi/ItemModifyJob>
#include <Akonadi/TagCreateJob>

#include <QDebug>

using namespace Akonadi;

CreateAndSetTagsJob::CreateAndSetTagsJob(const Item &item, const Akonadi::Tag::List &tags, QObject *parent)
    : KJob(parent)
    , mItem(item)
    , mTags(tags)
    , mCount(0)
{
}

void CreateAndSetTagsJob::start()
{
    if (mTags.isEmpty()) {
        emitResult();
    }
    for (const Akonadi::Tag &tag : std::as_const(mTags)) {
        auto createJob = new Akonadi::TagCreateJob(tag, this);
        createJob->setMergeIfExisting(true);
        connect(createJob, &Akonadi::TagCreateJob::result, this, &CreateAndSetTagsJob::onCreateDone);
    }
}

void CreateAndSetTagsJob::onCreateDone(KJob *job)
{
    mCount++;
    if (job->error()) {
        qWarning() << "Failed to create tag " << job->errorString();
    } else {
        auto createJob = static_cast<Akonadi::TagCreateJob *>(job);
        mCreatedTags << createJob->tag();
    }
    if (mCount == mTags.size()) {
        for (const Akonadi::Tag &tag : std::as_const(mCreatedTags)) {
            mItem.setTag(tag);
        }
        auto modJob = new Akonadi::ItemModifyJob(mItem, this);
        connect(modJob, &Akonadi::ItemModifyJob::result, this, &CreateAndSetTagsJob::onModifyDone);
    }
}

void CreateAndSetTagsJob::onModifyDone(KJob *job)
{
    if (job->error()) {
        qWarning() << "Failed to modify item " << job->errorString();
        setError(KJob::UserDefinedError);
    }
    emitResult();
}
