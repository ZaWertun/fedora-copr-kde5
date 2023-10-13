/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsabstractchunkedjob.h"
#include "ewsmodifyitemjob.h"
#include "ewsupdateitemrequest.h"

class EwsModifyMailJob : public EwsModifyItemJob
{
    Q_OBJECT
public:
    EwsModifyMailJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent);
    ~EwsModifyMailJob() override;
    void start() override;

private:
    void updateItemsFinished(bool success, const QString &error);

    EwsAbstractChunkedJob<EwsUpdateItemRequest, EwsUpdateItemRequest::ItemChange, EwsUpdateItemRequest::Response> mChunkedJob;
};
