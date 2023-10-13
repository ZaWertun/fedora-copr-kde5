/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsid.h"
#include <QHash>
#include <QSharedData>

class EwsItemBasePrivate : public QSharedData
{
public:
    typedef QHash<EwsPropertyField, QVariant> PropertyHash;

    EwsItemBasePrivate();
    virtual ~EwsItemBasePrivate();

    virtual EwsItemBasePrivate *clone() const = 0;

    static bool extendedPropertyReader(QXmlStreamReader &reader, QVariant &val);
    static bool extendedPropertyWriter(QXmlStreamWriter &writer, const QVariant &val);

    // When the item, is first constructed it will only contain the id and will therefore be
    // invalid. Once updated through EWS the remaining data will be populated and the item will
    // be valid.
    bool mValid;

    QHash<EwsItemFields, QVariant> mFields;

    bool operator==(const EwsItemBasePrivate &other) const;
};

template<>
Q_INLINE_TEMPLATE EwsItemBasePrivate *QSharedDataPointer<EwsItemBasePrivate>::clone()
{
    return d->clone();
}

Q_DECLARE_METATYPE(EwsItemBasePrivate::PropertyHash)
