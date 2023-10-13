/*
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Attribute>

class UidValidityAttribute : public Akonadi::Attribute
{
public:
    explicit UidValidityAttribute(int uidvalidity = 0);
    void setUidValidity(int uidvalidity);
    int uidValidity() const;
    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    int mUidValidity;
};
