/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchabchpersondetailjob.h"

#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsFetchAbchContactDetailsJob::EwsFetchAbchContactDetailsJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    mRequest->setItemShape(shape);
}

EwsFetchAbchContactDetailsJob::~EwsFetchAbchContactDetailsJob() = default;

void EwsFetchAbchContactDetailsJob::processItems(const EwsGetItemRequest::Response::List &responses)
{
    Item::List::iterator it = mChangedItems.begin();

    for (const EwsGetItemRequest::Response &resp : responses) {
        Item &item = *it;

        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item %1").arg(item.remoteId());
            continue;
        }

        // const EwsItem &ewsItem = resp.item();

        // TODO: Implement

        ++it;
    }

    emitResult();
}
