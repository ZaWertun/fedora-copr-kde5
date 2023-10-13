/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifyitemflagsjob.h"

#include "ewsitemhandler.h"
#include "ewsmodifyitemjob.h"

using namespace Akonadi;

EwsModifyItemFlagsJob::EwsModifyItemFlagsJob(EwsClient &client,
                                             QObject *parent,
                                             const Item::List &items,
                                             const QSet<QByteArray> &addedFlags,
                                             const QSet<QByteArray> &removedFlags)
    : EwsJob(parent)
    , mItems(items)
    , mClient(client)
    , mAddedFlags(addedFlags)
    , mRemovedFlags(removedFlags)
{
}

EwsModifyItemFlagsJob::~EwsModifyItemFlagsJob() = default;

void EwsModifyItemFlagsJob::itemModifyFinished(KJob *job)
{
    if (job->error()) {
        setErrorText(job->errorString());
        emitResult();
        return;
    }

    auto req = qobject_cast<EwsModifyItemJob *>(job);
    if (!req) {
        setErrorText(QStringLiteral("Invalid EwsModifyItemJob job object"));
        emitResult();
        return;
    }

    mResultItems += req->items();
    removeSubjob(job);

    if (subjobs().isEmpty()) {
        Q_ASSERT(mResultItems.size() == mItems.size());
        emitResult();
    }
}

void EwsModifyItemFlagsJob::start()
{
    Item::List items[EwsItemTypeUnknown];

    for (const Item &item : std::as_const(mItems)) {
        EwsItemType type = EwsItemHandler::mimeToItemType(item.mimeType());
        if (type == EwsItemTypeUnknown) {
            setErrorText(QStringLiteral("Unknown item type %1 for item %2").arg(item.mimeType(), item.remoteId()));
            emitResult();
            return;
        } else {
            items[type].append(item);
        }
    }

    bool started = false;
    for (int type = 0; type < EwsItemTypeUnknown; type++) {
        if (!items[static_cast<EwsItemType>(type)].isEmpty()) {
            EwsItemHandler *handler = EwsItemHandler::itemHandler(static_cast<EwsItemType>(type));
            EwsModifyItemJob *job = handler->modifyItemJob(mClient, items[type], QSet<QByteArray>() << "FLAGS", this);
            connect(job, &EwsModifyItemJob::result, this, &EwsModifyItemFlagsJob::itemModifyFinished);
            connect(job, &EwsModifyItemJob::reportStatus, this, [this](int s, const QString &message) {
                Q_EMIT reportStatus(s, message);
            });
            connect(job, &EwsModifyItemJob::reportPercent, this, [this](int p) {
                Q_EMIT reportPercent(p);
            });

            addSubjob(job);
            job->start();
            started = true;
        }
    }

    if (!started) {
        setErrorText(QStringLiteral("No items to process"));
        emitResult();
    }
}
