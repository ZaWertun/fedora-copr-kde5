/*
    SPDX-FileCopyrightText: 2015 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ctagattribute.h"

CTagAttribute::CTagAttribute(const QString &ctag)
    : mCTag(ctag)
{
}

void CTagAttribute::setCTag(const QString &ctag)
{
    mCTag = ctag;
}

QString CTagAttribute::CTag() const
{
    return mCTag;
}

Akonadi::Attribute *CTagAttribute::clone() const
{
    return new CTagAttribute(mCTag);
}

QByteArray CTagAttribute::type() const
{
    static const QByteArray sType("ctag");
    return sType;
}

QByteArray CTagAttribute::serialized() const
{
    return mCTag.toUtf8();
}

void CTagAttribute::deserialize(const QByteArray &data)
{
    mCTag = QString::fromUtf8(data);
}
