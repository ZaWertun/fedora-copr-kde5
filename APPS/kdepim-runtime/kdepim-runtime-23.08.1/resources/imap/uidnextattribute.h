/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Attribute>

class UidNextAttribute : public Akonadi::Attribute
{
public:
    explicit UidNextAttribute(int uidnext = 0);
    void setUidNext(int uidnext);
    int uidNext() const;
    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    int mUidNext;
};
