/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QMultiMap>

#include "ewsitembase.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class EwsItemPrivate;

class EwsItem : public EwsItemBase
{
public:
    typedef QList<EwsItem> List;
    typedef QMultiMap<QString, QString> HeaderMap;

    EwsItem();
    explicit EwsItem(QXmlStreamReader &reader);
    EwsItem(const EwsItem &other);
    EwsItem(EwsItem &&other);
    ~EwsItem() override;

    EwsItem &operator=(const EwsItem &other);
    EwsItem &operator=(EwsItem &&other);

    EwsItemType type() const;
    void setType(EwsItemType type);
    EwsItemType internalType() const;

    bool write(QXmlStreamWriter &writer) const;

    bool operator==(const EwsItem &other) const;

protected:
    bool readBaseItemElement(QXmlStreamReader &reader);
};

Q_DECLARE_METATYPE(EwsItem)
Q_DECLARE_METATYPE(EwsItem::List)
Q_DECLARE_METATYPE(EwsItem::HeaderMap)
