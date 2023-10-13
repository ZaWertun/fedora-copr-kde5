/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitemhandler.h"

#include "ewsresource.h"
#include "ewsresource_debug.h"

struct HandlerFactory {
    EwsItemType type;
    EwsItemHandler::ItemHandlerFactory factory;
};

using HandlerList = QList<HandlerFactory>;
using HandlerHash = QHash<EwsItemType, QSharedPointer<EwsItemHandler>>;

Q_GLOBAL_STATIC(HandlerList, handlerFactories)
Q_GLOBAL_STATIC(HandlerHash, handlers)

EwsItemHandler::~EwsItemHandler() = default;

void EwsItemHandler::registerItemHandler(EwsItemType type, const ItemHandlerFactory &factory)
{
    handlerFactories->append({type, factory});
}

EwsItemHandler *EwsItemHandler::itemHandler(EwsItemType type)
{
    HandlerHash::iterator it = handlers->find(type);
    if (it != handlers->end()) {
        return it->data();
    } else {
        const HandlerList::const_iterator end(handlerFactories->cend());
        for (HandlerList::const_iterator it = handlerFactories->cbegin(); it != end; ++it) {
            if (it->type == type) {
                EwsItemHandler *handler = it->factory();
                (*handlers)[type].reset(handler);
                return handler;
            }
        }
        qCWarning(EWSRES_LOG) << QStringLiteral("Could not find handler for item type %1").arg(type);

        return nullptr;
    }
}

EwsItemType EwsItemHandler::mimeToItemType(const QString &mimeType)
{
    if (mimeType == itemHandler(EwsItemTypeMessage)->mimeType()) {
        return EwsItemTypeMessage;
    } else if (mimeType == itemHandler(EwsItemTypeCalendarItem)->mimeType()) {
        return EwsItemTypeCalendarItem;
    } else if (mimeType == itemHandler(EwsItemTypeTask)->mimeType()) {
        return EwsItemTypeTask;
    } else if (mimeType == itemHandler(EwsItemTypeContact)->mimeType()) {
        return EwsItemTypeContact;
    } else {
        return EwsItemTypeItem;
    }
}

QHash<EwsPropertyField, QVariant> EwsItemHandler::writeFlags(const QSet<QByteArray> &flags)
{
    QHash<EwsPropertyField, QVariant> propertyHash;

    if (flags.isEmpty()) {
        propertyHash.insert(EwsResource::flagsProperty, QVariant());
    } else {
        QStringList flagList;
        flagList.reserve(flags.count());
        for (const QByteArray &flag : flags) {
            flagList.append(QString::fromLatin1(flag));
        }
        propertyHash.insert(EwsResource::flagsProperty, flagList);
    }

    return propertyHash;
}

QSet<QByteArray> EwsItemHandler::readFlags(const EwsItem &item)
{
    QSet<QByteArray> flags;

    QVariant flagProp = item[EwsResource::flagsProperty];
    if (!flagProp.isNull() && (flagProp.canConvert<QStringList>())) {
        const QStringList flagList = flagProp.toStringList();
        flags.reserve(flagList.count());
        for (const QString &flag : flagList) {
            flags.insert(flag.toLatin1());
        }
    }

    return flags;
}

QList<EwsPropertyField> EwsItemHandler::flagsProperties()
{
    return {EwsResource::flagsProperty};
}

QList<EwsPropertyField> EwsItemHandler::tagsProperties()
{
    return {EwsResource::tagsProperty};
}
