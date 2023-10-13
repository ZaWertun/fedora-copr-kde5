/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uidvalidityattribute.h"

#include <QByteArray>

UidValidityAttribute::UidValidityAttribute(int uidvalidity)
    : mUidValidity(uidvalidity)
{
}

void UidValidityAttribute::setUidValidity(int uidvalidity)
{
    mUidValidity = uidvalidity;
}

int UidValidityAttribute::uidValidity() const
{
    return mUidValidity;
}

QByteArray UidValidityAttribute::type() const
{
    static const QByteArray sType("uidvalidity");
    return sType;
}

Akonadi::Attribute *UidValidityAttribute::clone() const
{
    return new UidValidityAttribute(mUidValidity);
}

QByteArray UidValidityAttribute::serialized() const
{
    return QByteArray::number(mUidValidity);
}

void UidValidityAttribute::deserialize(const QByteArray &data)
{
    mUidValidity = data.toInt();
}
