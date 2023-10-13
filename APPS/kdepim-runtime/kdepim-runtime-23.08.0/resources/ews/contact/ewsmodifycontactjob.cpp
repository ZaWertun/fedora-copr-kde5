/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifycontactjob.h"

#include "ewsresource_debug.h"

EwsModifyContactJob::EwsModifyContactJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyContactJob::~EwsModifyContactJob() = default;

void EwsModifyContactJob::start()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Contact item modification not implemented!");
    emitResult();
}
