/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifyabchpersonjob.h"

#include "ewsresource_debug.h"

EwsModifyAbchPersonJob::EwsModifyAbchPersonJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyAbchPersonJob::~EwsModifyAbchPersonJob() = default;

void EwsModifyAbchPersonJob::start()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Abch person item modification not implemented!");
    emitResult();
}
