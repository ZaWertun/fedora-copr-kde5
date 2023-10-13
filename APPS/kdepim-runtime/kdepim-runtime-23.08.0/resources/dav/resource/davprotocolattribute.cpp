/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "davprotocolattribute.h"

DavProtocolAttribute::DavProtocolAttribute(int protocol)
    : mDavProtocol(protocol)
{
}

int DavProtocolAttribute::davProtocol() const
{
    return mDavProtocol;
}

void DavProtocolAttribute::setDavProtocol(int protocol)
{
    mDavProtocol = protocol;
}

Akonadi::Attribute *DavProtocolAttribute::clone() const
{
    return new DavProtocolAttribute(mDavProtocol);
}

QByteArray DavProtocolAttribute::type() const
{
    static const QByteArray sType("davprotocol");
    return sType;
}

QByteArray DavProtocolAttribute::serialized() const
{
    return QByteArray::number(mDavProtocol);
}

void DavProtocolAttribute::deserialize(const QByteArray &data)
{
    mDavProtocol = data.toInt();
}
