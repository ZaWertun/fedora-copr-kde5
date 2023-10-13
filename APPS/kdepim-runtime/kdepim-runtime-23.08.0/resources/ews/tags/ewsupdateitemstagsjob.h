/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsabstractchunkedjob.h"
#include "ewsjob.h"
#include "ewsupdateitemrequest.h"

#include <Akonadi/Item>

class EwsTagStore;
class EwsClient;
class EwsResource;

/**
 *  @brief  Job used to update Exchange items with tag information from Akonadi
 *
 *  This job cycles through all items and updates the Exchange database with tag information from
 *  the items.
 *
 *  The job relies on the tag store to retrieve tag identifiers and names that can be stored in
 *  Exchange. Due to buggy tag implementation in Akonadi it can happen that items contain tags not
 *  yet known to the EWS resource. In such case an additional tag fetch job is issues to fetch
 *  information about those tags so that they can be added to the tag store.
 */
class EwsUpdateItemsTagsJob : public EwsJob
{
    Q_OBJECT
public:
    EwsUpdateItemsTagsJob(const Akonadi::Item::List &items, EwsTagStore *tagStore, EwsClient &client, EwsResource *parent);
    ~EwsUpdateItemsTagsJob() override;

    void start() override;

    Akonadi::Item::List items()
    {
        return mItems;
    }

private Q_SLOTS:
    void itemsTagsChangedTagsFetched(KJob *job);
    void globalTagsWriteFinished(KJob *job);

Q_SIGNALS:
    void reportStatus(int status, const QString &message = QString());
    void reportPercent(int progress);

private:
    void updateItemsTagsRequestFinished(bool success, const QString &error);
    void doUpdateItemsTags();

    Akonadi::Item::List mItems;
    EwsTagStore *mTagStore = nullptr;
    EwsClient &mClient;

    EwsAbstractChunkedJob<EwsUpdateItemRequest, EwsUpdateItemRequest::ItemChange, EwsUpdateItemRequest::Response> mChunkedJob;
};
