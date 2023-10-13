/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QPointer>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include "ewsclient.h"
#include "ewsgetitemrequest.h"
#include "ewsid.h"
#include "ewsitem.h"
#include "ewsjob.h"
#include "ewstypes.h"

class EwsFetchItemDetailJob : public EwsJob
{
    Q_OBJECT
public:
    EwsFetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection);
    ~EwsFetchItemDetailJob() override;

    void setItemLists(const Akonadi::Item::List &changedItems, Akonadi::Item::List *deletedItems);

    Akonadi::Item::List changedItems() const
    {
        return mChangedItems;
    }

    void start() override;

protected:
    virtual void processItems(const EwsGetItemRequest::Response::List &responses) = 0;

    QPointer<EwsGetItemRequest> mRequest;
    Akonadi::Item::List mChangedItems;
    Akonadi::Item::List *mDeletedItems = nullptr;
    EwsClient &mClient;
    const Akonadi::Collection mCollection;
private Q_SLOTS:
    void itemDetailFetched(KJob *job);
};
