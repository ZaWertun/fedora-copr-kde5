/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uidnextattribute.h"

#include <QByteArray>

UidNextAttribute::UidNextAttribute(int uidnext)
    : mUidNext(uidnext)
{
}

void UidNextAttribute::setUidNext(int uidnext)
{
    mUidNext = uidnext;
}

int UidNextAttribute::uidNext() const
{
    return mUidNext;
}

QByteArray UidNextAttribute::type() const
{
    static const QByteArray sType("uidnext");
    return sType;
}

Akonadi::Attribute *UidNextAttribute::clone() const
{
    return new UidNextAttribute(mUidNext);
}

QByteArray UidNextAttribute::serialized() const
{
    return QByteArray::number(mUidNext);
}

void UidNextAttribute::deserialize(const QByteArray &data)
{
    mUidNext = data.toInt();
}
