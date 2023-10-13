/*
    SPDX-FileCopyrightText: 2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QVector>

#include "ewsjob.h"

template<typename Req, typename ReqItem, typename RespItem>
class EwsAbstractChunkedJob
{
public:
    EwsAbstractChunkedJob(unsigned int chunkSize);
    ~EwsAbstractChunkedJob() = default;

    using ReqItemList = QVector<ReqItem>;
    using RespItemList = QVector<RespItem>;

    void setItems(const ReqItemList &items);
    const RespItemList &responses() const;

    template<typename ReqSetupFn, typename RespGetFn, typename ProgressFn, typename ResultFn>
    void start(ReqSetupFn reqSetupFn, RespGetFn respGetFn, ProgressFn progressFn, ResultFn resultFn);

private:
    unsigned int mChunkSize;
    unsigned int mItemsDone;
    Req *mRequest = nullptr;
    ReqItemList mItems;
    RespItemList mResponses;
};

template<typename Req, typename ReqItem, typename RespItem>
EwsAbstractChunkedJob<Req, ReqItem, RespItem>::EwsAbstractChunkedJob(unsigned int chunkSize)
    : mChunkSize(chunkSize)
    , mItemsDone(0)
{
}

template<typename Req, typename ReqItem, typename RespItem>
void EwsAbstractChunkedJob<Req, ReqItem, RespItem>::setItems(const ReqItemList &items)
{
    mItems = items;
}

template<typename Req, typename ReqItem, typename RespItem>
const QVector<RespItem> &EwsAbstractChunkedJob<Req, ReqItem, RespItem>::responses() const
{
    return mResponses;
}

template<typename Req, typename ReqItem, typename RespItem>
template<typename ReqSetupFn, typename RespGetFn, typename ProgressFn, typename ResultFn>
void EwsAbstractChunkedJob<Req, ReqItem, RespItem>::start(ReqSetupFn reqSetupFn, RespGetFn respGetFn, ProgressFn progressFn, ResultFn resultFn)
{
    int itemsToDo = qMin<int>(mItems.size() - mItemsDone, mChunkSize);
    if (itemsToDo == 0) {
        resultFn(true, QString());
        return;
    }

    mRequest = reqSetupFn(mItems.cbegin() + mItemsDone, mItems.cbegin() + mItemsDone + itemsToDo);
    if (!mRequest) {
        resultFn(false, QStringLiteral("Failed to set-up request"));
        return;
    }

    mItemsDone += itemsToDo;

    QObject::connect(mRequest, &KJob::result, [this, reqSetupFn, respGetFn, progressFn, resultFn, itemsToDo](KJob *job) {
        if (job->error()) {
            resultFn(false, job->errorString());
            return;
        }

        Req *req = qobject_cast<Req *>(job);
        if (!req) {
            resultFn(false, QStringLiteral("Incorrect request object type"));
            return;
        }

        auto responses = respGetFn(req);
        Q_ASSERT(responses.size() == itemsToDo);
        mResponses += responses;

        progressFn(mItemsDone * 100 / mItems.size());

        start(reqSetupFn, respGetFn, progressFn, resultFn);
    });

    mRequest->start();
}
