/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreatecontactjob.h"

#include "ewsresource_debug.h"

EwsCreateContactJob::EwsCreateContactJob(EwsClient &client,
                                         const Akonadi::Item &item,
                                         const Akonadi::Collection &collection,
                                         EwsTagStore *tagStore,
                                         EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateContactJob::~EwsCreateContactJob() = default;

void EwsCreateContactJob::doStart()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Contact item creation not implemented!");
    emitResult();
}

bool EwsCreateContactJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending contact items is not supported!");
    return false;
}
