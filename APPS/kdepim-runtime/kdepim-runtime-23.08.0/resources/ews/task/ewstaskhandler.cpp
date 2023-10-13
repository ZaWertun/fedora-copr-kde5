/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewstaskhandler.h"

#include <KCalendarCore/Todo>

#include "ewscreatetaskjob.h"
#include "ewsfetchtaskdetailjob.h"
#include "ewsmodifytaskjob.h"

using namespace Akonadi;

EwsTaskHandler::EwsTaskHandler() = default;

EwsTaskHandler::~EwsTaskHandler() = default;

EwsItemHandler *EwsTaskHandler::factory()
{
    return new EwsTaskHandler();
}

EwsFetchItemDetailJob *EwsTaskHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchTaskDetailJob(client, parent, collection);
}

void EwsTaskHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsTaskHandler::mimeType()
{
    return KCalendarCore::Todo::todoMimeType();
}

bool EwsTaskHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    Q_UNUSED(item)
    Q_UNUSED(ewsItem)

    return true;
}

EwsModifyItemJob *EwsTaskHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyTaskJob(client, items, parts, parent);
}

EwsCreateItemJob *
EwsTaskHandler::createItemJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent)
{
    return new EwsCreateTaskJob(client, item, collection, tagStore, parent);
}

EWS_DECLARE_ITEM_HANDLER(EwsTaskHandler, EwsItemTypeTask)
