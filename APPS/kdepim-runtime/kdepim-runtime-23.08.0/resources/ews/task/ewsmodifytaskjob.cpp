/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifytaskjob.h"

#include "ewsresource_debug.h"

EwsModifyTaskJob::EwsModifyTaskJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyTaskJob::~EwsModifyTaskJob() = default;

void EwsModifyTaskJob::start()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Task item modification not implemented!");
    emitResult();
}
