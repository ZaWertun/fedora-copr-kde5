/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedDataPointer>

#include "ewsid.h"
#include "ewspropertyfield.h"

class EwsItemBasePrivate;

class EwsItemBase
{
public:
    EwsItemBase(const EwsItemBase &other);
    EwsItemBase(EwsItemBase &&other);
    virtual ~EwsItemBase();
    EwsItemBase &operator=(const EwsItemBase &other);
    EwsItemBase &operator=(EwsItemBase &&other);

    bool isValid() const;

    bool hasField(EwsItemFields f) const;
    QVariant operator[](EwsItemFields f) const;

    QVariant operator[](const EwsPropertyField &prop) const;

    void setField(EwsItemFields f, const QVariant &value);
    void setProperty(const EwsPropertyField &prop, const QVariant &value);

protected:
    EwsItemBase(const QSharedDataPointer<EwsItemBasePrivate> &priv);

    void resetFields();

    QSharedDataPointer<EwsItemBasePrivate> d;
};
