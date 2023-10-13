/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreateabchpersonjob.h"

#include "ewsresource_debug.h"

EwsCreateAbchPersonJob::EwsCreateAbchPersonJob(EwsClient &client,
                                               const Akonadi::Item &item,
                                               const Akonadi::Collection &collection,
                                               EwsTagStore *tagStore,
                                               EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateAbchPersonJob::~EwsCreateAbchPersonJob() = default;

void EwsCreateAbchPersonJob::doStart()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Abch person item creation not implemented!");
    emitResult();
}

bool EwsCreateAbchPersonJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending abch person items is not supported!");
    return false;
}
