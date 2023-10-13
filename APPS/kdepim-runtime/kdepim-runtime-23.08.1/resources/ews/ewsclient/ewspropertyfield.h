/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedDataPointer>
#include <QXmlStreamWriter>

#include "ewstypes.h"

class EwsPropertyFieldPrivate;

class EwsPropertyField
{
public:
    enum Type {
        Field,
        ExtendedField,
        IndexedField,
        UnknownField,
    };

    EwsPropertyField();
    explicit EwsPropertyField(const QString &uri); // FieldURI
    EwsPropertyField(const QString &uri, unsigned index); // IndexedFieldURI
    EwsPropertyField(EwsDistinguishedPropSetId psid, unsigned id, EwsPropertyType type);
    EwsPropertyField(EwsDistinguishedPropSetId psid, const QString &name, EwsPropertyType type);
    EwsPropertyField(const QString &psid, unsigned id, EwsPropertyType type);
    EwsPropertyField(const QString &psid, const QString &name, EwsPropertyType type);
    EwsPropertyField(unsigned tag, EwsPropertyType type);
    EwsPropertyField(const EwsPropertyField &other);
    ~EwsPropertyField();

    EwsPropertyField &operator=(const EwsPropertyField &other);
    bool operator==(const EwsPropertyField &other) const;

    EwsPropertyField(EwsPropertyField &&other);
    EwsPropertyField &operator=(EwsPropertyField &&other);

    void write(QXmlStreamWriter &writer) const;
    bool read(QXmlStreamReader &reader);

    bool writeWithValue(QXmlStreamWriter &writer, const QVariant &value) const;
    void writeValue(QXmlStreamWriter &writer, const QVariant &value) const;
    void writeExtendedValue(QXmlStreamWriter &writer, const QVariant &value) const;

    Type type() const;
    QString uri() const;

private:
    QSharedDataPointer<EwsPropertyFieldPrivate> d;

    friend uint qHash(const EwsPropertyField &prop, uint seed);
    friend QDebug operator<<(QDebug debug, const EwsPropertyField &prop);
};

uint qHash(const EwsPropertyField &prop, uint seed);

QDebug operator<<(QDebug debug, const EwsPropertyField &prop);
