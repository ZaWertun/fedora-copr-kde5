/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreatecalendarjob.h"

#include "ewsresource_debug.h"

EwsCreateCalendarJob::EwsCreateCalendarJob(EwsClient &client,
                                           const Akonadi::Item &item,
                                           const Akonadi::Collection &collection,
                                           EwsTagStore *tagStore,
                                           EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateCalendarJob::~EwsCreateCalendarJob() = default;

void EwsCreateCalendarJob::doStart()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Calendar item creation not implemented!");
    emitResult();
}

bool EwsCreateCalendarJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending calendar items is not supported!");
    return false;
}
