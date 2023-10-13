/*
    SPDX-FileCopyrightText: 2015 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Attribute>

#include <QString>

class CTagAttribute : public Akonadi::Attribute
{
public:
    explicit CTagAttribute(const QString &ctag = QString());

    void setCTag(const QString &ctag);
    QString CTag() const;

    Akonadi::Attribute *clone() const override;
    QByteArray type() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    QString mCTag;
};
