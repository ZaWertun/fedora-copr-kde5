/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreatetaskjob.h"

#include "ewsresource_debug.h"

EwsCreateTaskJob::EwsCreateTaskJob(EwsClient &client,
                                   const Akonadi::Item &item,
                                   const Akonadi::Collection &collection,
                                   EwsTagStore *tagStore,
                                   EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateTaskJob::~EwsCreateTaskJob() = default;

void EwsCreateTaskJob::doStart()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Task item creation not implemented!");
    emitResult();
}

bool EwsCreateTaskJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending task items is not supported!");
    return false;
}
