/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Attribute>

class DavProtocolAttribute : public Akonadi::Attribute
{
public:
    explicit DavProtocolAttribute(int protocol = 0);

    void setDavProtocol(int protocol);
    int davProtocol() const;

    Akonadi::Attribute *clone() const override;
    QByteArray type() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    int mDavProtocol = 0;
};
