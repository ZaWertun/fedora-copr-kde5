/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "noselectattribute.h"

#include <QByteArray>

NoSelectAttribute::NoSelectAttribute(bool noSelect)
    : mNoSelect(noSelect)
{
}

void NoSelectAttribute::setNoSelect(bool noSelect)
{
    mNoSelect = noSelect;
}

bool NoSelectAttribute::noSelect() const
{
    return mNoSelect;
}

QByteArray NoSelectAttribute::type() const
{
    static const QByteArray sType("noselect");
    return sType;
}

Akonadi::Attribute *NoSelectAttribute::clone() const
{
    return new NoSelectAttribute(mNoSelect);
}

QByteArray NoSelectAttribute::serialized() const
{
    return mNoSelect ? QByteArray::number(1) : QByteArray::number(0);
}

void NoSelectAttribute::deserialize(const QByteArray &data)
{
    mNoSelect = (data.toInt() == 0) ? false : true;
}
