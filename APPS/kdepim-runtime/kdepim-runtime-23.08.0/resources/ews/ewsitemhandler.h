/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QSharedPointer>

#include "ewspropertyfield.h"
#include "ewstypes.h"

namespace Akonadi
{
class Collection;
class Item;
}
class EwsClient;
class EwsFetchItemDetailJob;
class EwsModifyItemJob;
class EwsCreateItemJob;
class EwsItem;
class EwsTagStore;
class EwsResource;

class EwsItemHandler
{
public:
    virtual ~EwsItemHandler();

    virtual EwsFetchItemDetailJob *fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection) = 0;
    virtual void setSeenFlag(Akonadi::Item &item, bool value) = 0;
    virtual QString mimeType() = 0;
    virtual bool setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem) = 0;
    virtual EwsModifyItemJob *modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent) = 0;
    virtual EwsCreateItemJob *
    createItemJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent) = 0;

    using ItemHandlerFactory = std::function<EwsItemHandler *()>;
    static void registerItemHandler(EwsItemType type, const ItemHandlerFactory &factory);
    static EwsItemHandler *itemHandler(EwsItemType type);
    static EwsItemType mimeToItemType(const QString &mimeType);
    static QHash<EwsPropertyField, QVariant> writeFlags(const QSet<QByteArray> &flags);
    static QSet<QByteArray> readFlags(const EwsItem &item);
    static QList<EwsPropertyField> flagsProperties();
    static QList<EwsPropertyField> tagsProperties();

private:
    struct HandlerFactory {
        EwsItemType type;
        ItemHandlerFactory factory;
    };
};

#define EWS_DECLARE_ITEM_HANDLER(clsname, type)                                                                                                                \
    class type##_itemhandler_registrar                                                                                                                         \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        type##_itemhandler_registrar()                                                                                                                         \
        {                                                                                                                                                      \
            EwsItemHandler::registerItemHandler(type, &clsname::factory);                                                                                      \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    const type##_itemhandler_registrar type##_itemhandler_registrar_object;
