/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectionflagsattribute.h"

#include <QByteArray>

using namespace Akonadi;

CollectionFlagsAttribute::CollectionFlagsAttribute(const QList<QByteArray> &flags)
    : mFlags(flags)
{
}

void CollectionFlagsAttribute::setFlags(const QList<QByteArray> &flags)
{
    mFlags = flags;
}

QList<QByteArray> CollectionFlagsAttribute::flags() const
{
    return mFlags;
}

QByteArray CollectionFlagsAttribute::type() const
{
    static const QByteArray sType("collectionflags");
    return sType;
}

Akonadi::Attribute *CollectionFlagsAttribute::clone() const
{
    return new CollectionFlagsAttribute(mFlags);
}

QByteArray CollectionFlagsAttribute::serialized() const
{
    QByteArray result;

    for (const QByteArray &flag : std::as_const(mFlags)) {
        result += flag + ' ';
    }
    result.chop(1);

    return result;
}

void CollectionFlagsAttribute::deserialize(const QByteArray &data)
{
    mFlags = data.split(' ');
}
