/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchitemdetailjob.h"

EwsFetchItemDetailJob::EwsFetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsJob(parent)
    , mDeletedItems(nullptr)
    , mClient(client)
    , mCollection(collection) // never used
{
    mRequest = new EwsGetItemRequest(client, this);
    connect(mRequest.data(), &KJob::result, this, &EwsFetchItemDetailJob::itemDetailFetched);
    addSubjob(mRequest);
}

EwsFetchItemDetailJob::~EwsFetchItemDetailJob() = default;

void EwsFetchItemDetailJob::setItemLists(const Akonadi::Item::List &changedItems, Akonadi::Item::List *deletedItems)
{
    mChangedItems = changedItems;
    mDeletedItems = deletedItems;

    EwsId::List ids;
    ids.reserve(changedItems.count());

    for (const Akonadi::Item &item : changedItems) {
        EwsId id(item.remoteId(), item.remoteRevision());
        ids.append(id);
    }

    mRequest->setItemIds(ids);
}

void EwsFetchItemDetailJob::itemDetailFetched(KJob *job)
{
    if (!job->error() && job == mRequest) {
        Q_ASSERT(mChangedItems.size() == mRequest->responses().size());

        processItems(mRequest->responses());
    }
}

void EwsFetchItemDetailJob::start()
{
    mRequest->start();
}
