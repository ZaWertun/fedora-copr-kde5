/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifycalendarjob.h"

#include "ewsresource_debug.h"

EwsModifyCalendarJob::EwsModifyCalendarJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyCalendarJob::~EwsModifyCalendarJob() = default;

void EwsModifyCalendarJob::start()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Calendar item modification not implemented!");
    emitResult();
}
