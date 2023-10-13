/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreateitemjob.h"

#include "ewsresource.h"
#include "tags/ewsakonaditagssyncjob.h"
#include "tags/ewstagstore.h"

EwsCreateItemJob::EwsCreateItemJob(EwsClient &client,
                                   const Akonadi::Item &item,
                                   const Akonadi::Collection &collection,
                                   EwsTagStore *tagStore,
                                   EwsResource *parent)
    : EwsJob(parent)
    , mItem(item)
    , mCollection(collection) // never use
    , mClient(client)
    , mTagStore(tagStore)
{
}

EwsCreateItemJob::~EwsCreateItemJob() = default;

const Akonadi::Item &EwsCreateItemJob::item() const
{
    return mItem;
}

void EwsCreateItemJob::start()
{
    /* Before starting check if all Akonadi tags are known to the tag store */
    bool syncNeeded = false;
    const auto tags{mItem.tags()};
    for (const Akonadi::Tag &tag : tags) {
        if (!mTagStore->containsId(tag.id())) {
            syncNeeded = true;
            break;
        }
    }

    if (syncNeeded) {
        auto job = new EwsAkonadiTagsSyncJob(mTagStore, mClient, qobject_cast<EwsResource *>(parent())->rootCollection(), this);
        connect(job, &EwsAkonadiTagsSyncJob::result, this, &EwsCreateItemJob::tagSyncFinished);
        job->start();
    } else {
        doStart();
    }
}

void EwsCreateItemJob::populateCommonProperties(EwsItem &item)
{
    if (!mTagStore->writeEwsProperties(mItem, item)) {
        setErrorMsg(QStringLiteral("Failed to write tags despite an earlier sync"));
    }
}

void EwsCreateItemJob::tagSyncFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorText());
        emitResult();
    } else {
        doStart();
    }
}
