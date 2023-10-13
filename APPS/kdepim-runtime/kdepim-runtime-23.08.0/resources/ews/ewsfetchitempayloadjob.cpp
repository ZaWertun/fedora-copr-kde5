/*
    SPDX-FileCopyrightText: 2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchitempayloadjob.h"

#include <KLocalizedString>

#include "ewsitemhandler.h"

#include "ewsresource_debug.h"

using namespace Akonadi;

constexpr unsigned ChunkSize = 10;

EwsFetchItemPayloadJob::EwsFetchItemPayloadJob(EwsClient &client, QObject *parent, const Akonadi::Item::List &items)
    : EwsJob(parent)
    , mItems(items)
    , mClient(client)
    , mChunkedJob(ChunkSize)
{
}

void EwsFetchItemPayloadJob::start()
{
    EwsId::List ids;
    ids.reserve(mItems.count());
    for (const Item &item : std::as_const(mItems)) {
        ids << EwsId(item.remoteId(), item.remoteRevision());
    }

    mChunkedJob.setItems(ids);
    mChunkedJob.start(
        [this](const EwsId::List::const_iterator &firstId, const EwsId::List::const_iterator &lastId) {
            auto req = new EwsGetItemRequest(mClient, this);
            EwsId::List ids;
            for (auto it = firstId; it != lastId; ++it) {
                ids.append(*it);
            }
            req->setItemIds(ids);
            EwsItemShape shape(EwsShapeIdOnly);
            shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
            req->setItemShape(shape);
            return req;
        },
        [](EwsGetItemRequest *req) {
            return req->responses();
        },
        [this](unsigned int progress) {
            Q_EMIT reportPercent(progress);
        },
        [this](bool success, const QString &error) {
            itemFetchFinished(success, error);
        });
}

void EwsFetchItemPayloadJob::itemFetchFinished(bool success, const QString & /*error*/)
{
    if (!success) {
        setErrorText(i18nc("@info:status", "Failed to process items retrieval request"));
        emitResult();
        return;
    }

    const EwsGetItemRequest::Response &resp = mChunkedJob.responses()[0];
    if (!resp.isSuccess()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << QStringLiteral("retrieveItems: Item fetch failed.");
        setErrorText(i18nc("@info:status", "Failed to retrieve items"));
        emitResult();
        return;
    }

    if (mItems.size() != mChunkedJob.responses().size()) {
        qCWarningNC(EWSRES_AGENTIF_LOG) << QStringLiteral("retrieveItems: incorrect number of responses.");
        setErrorText(i18nc("@info:status", "Failed to retrieve items - incorrect number of responses"));
        emitResult();
        return;
    }

    /* In general EWS guarantees that the order of response items will match the order of request items.
     * It is therefore safe to iterate these in parallel. */
    auto it = mItems.begin();
    for (const auto &resp : mChunkedJob.responses()) {
        const EwsItem &ewsItem = resp.item();
        auto id = ewsItem[EwsItemFieldItemId].value<EwsId>();
        if (it->remoteId() != id.id()) {
            qCWarningNC(EWSRES_AGENTIF_LOG) << QStringLiteral("retrieveItems: Akonadi item not found for item %1.").arg(id.id());
            setErrorText(i18nc("@info:status", "Failed to retrieve items - Akonadi item not found for item %1", id.id()));
            emitResult();
            return;
        }
        EwsItemType type = ewsItem.internalType();
        if (type == EwsItemTypeUnknown) {
            qCWarningNC(EWSRES_AGENTIF_LOG) << QStringLiteral("retrieveItems: Unknown item type for item %1!").arg(id.id());
            setErrorText(i18nc("@info:status", "Failed to retrieve items - Unknown item type for item %1", id.id()));
            emitResult();
            return;
        }
        if (!EwsItemHandler::itemHandler(type)->setItemPayload(*it, ewsItem)) {
            qCWarningNC(EWSRES_AGENTIF_LOG) << "retrieveItems: Failed to fetch item payload";
            setErrorText(i18nc("@info:status", "Failed to fetch item payload"));
            emitResult();
            return;
        }
        ++it;
    }

    emitResult();
}
