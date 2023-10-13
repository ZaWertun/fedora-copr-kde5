/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsabchpersonhandler.h"

#include <KCalendarCore/Todo>

#include "ewscreateabchpersonjob.h"
#include "ewsfetchabchpersondetailjob.h"
#include "ewsmodifyabchpersonjob.h"

using namespace Akonadi;

EwsAbchPersonHandler::EwsAbchPersonHandler() = default;

EwsAbchPersonHandler::~EwsAbchPersonHandler() = default;

EwsItemHandler *EwsAbchPersonHandler::factory()
{
    return new EwsAbchPersonHandler();
}

EwsFetchItemDetailJob *EwsAbchPersonHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchAbchContactDetailsJob(client, parent, collection);
}

void EwsAbchPersonHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsAbchPersonHandler::mimeType()
{
    return KCalendarCore::Todo::todoMimeType();
}

bool EwsAbchPersonHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    Q_UNUSED(item)
    Q_UNUSED(ewsItem)

    return true;
}

EwsModifyItemJob *EwsAbchPersonHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyAbchPersonJob(client, items, parts, parent);
}

EwsCreateItemJob *EwsAbchPersonHandler::createItemJob(EwsClient &client,
                                                      const Akonadi::Item &item,
                                                      const Akonadi::Collection &collection,
                                                      EwsTagStore *tagStore,
                                                      EwsResource *parent)
{
    return new EwsCreateAbchPersonJob(client, item, collection, tagStore, parent);
}

EWS_DECLARE_ITEM_HANDLER(EwsAbchPersonHandler, EwsItemTypeAbchPerson)
