/*
    SPDX-FileCopyrightText: 2015-2021 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifymailjob.h"

#include <Akonadi/MessageFlags>

#include <KLocalizedString>

#include "ewsmailhandler.h"

#include "ewsresource_debug.h"

using namespace Akonadi;

constexpr unsigned ChunkSize = 10;

EwsModifyMailJob::EwsModifyMailJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
    , mChunkedJob(ChunkSize)
{
}

EwsModifyMailJob::~EwsModifyMailJob() = default;

void EwsModifyMailJob::start()
{
    bool doSubmit = false;
    EwsId itemId;

    EwsUpdateItemRequest::ItemChange::List itemChanges;
    itemChanges.reserve(mItems.size());
    for (const Item &item : std::as_const(mItems)) {
        itemId = EwsId(item.remoteId(), item.remoteRevision());

        if (mParts.contains("FLAGS")) {
            EwsUpdateItemRequest::ItemChange ic(itemId, EwsItemTypeMessage);
            QHash<EwsPropertyField, QVariant> propertyHash = EwsMailHandler::writeFlags(item.flags());

            for (auto it = propertyHash.cbegin(), end = propertyHash.cend(); it != end; ++it) {
                EwsUpdateItemRequest::Update *upd;
                if (it.value().isNull()) {
                    upd = new EwsUpdateItemRequest::DeleteUpdate(it.key());
                } else {
                    upd = new EwsUpdateItemRequest::SetUpdate(it.key(), it.value());
                }
                ic.addUpdate(upd);
            }

            itemChanges.append(ic);
            doSubmit = true;
        }
    }

    if (doSubmit) {
        mChunkedJob.setItems(itemChanges);
        mChunkedJob.start(
            [this](const EwsUpdateItemRequest::ItemChange::List::const_iterator &firstChange,
                   const EwsUpdateItemRequest::ItemChange::List::const_iterator &lastChange) {
                auto req = new EwsUpdateItemRequest(mClient, this);
                req->addItemChanges(firstChange, lastChange);
                return req;
            },
            [](EwsUpdateItemRequest *req) {
                return req->responses();
            },
            [this](unsigned int progress) {
                Q_EMIT reportPercent(progress);
            },
            [this](bool success, const QString &error) {
                updateItemsFinished(success, error);
            });
    } else {
        emitResult();
    }
}

void EwsModifyMailJob::updateItemsFinished(bool success, const QString &error)
{
    if (!success) {
        setErrorText(error);
        emitResult();
        return;
    }

    Item::List::iterator it = mItems.begin();
    for (const auto &resp : mChunkedJob.responses()) {
        if (!resp.isSuccess()) {
            setErrorText(i18nc("@info:status", "Item update failed: ") + resp.responseMessage());
            emitResult();
            return;
        }

        /* In general EWS guarantees that the order of response items will match the order of request items.
         * It is therefore safe to iterate these in parallel. */
        if (it->remoteId() == resp.itemId().id()) {
            it->setRemoteRevision(resp.itemId().changeKey());
            ++it;
        } else {
            setErrorText(i18nc("@info:status", "Item out of order while processing update item response."));
            emitResult();
            return;
        }
    }
    emitResult();
}
