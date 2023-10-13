/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-singlefileresource_export.h"
#include <Akonadi/Attribute>

#include <QMap>

namespace Akonadi
{
class AKONADI_SINGLEFILERESOURCE_EXPORT ImapQuotaAttribute : public Akonadi::Attribute
{
public:
    ImapQuotaAttribute();
    ImapQuotaAttribute(const QList<QByteArray> &roots, const QList<QMap<QByteArray, qint64>> &limits, const QList<QMap<QByteArray, qint64>> &usages);

    void setQuotas(const QList<QByteArray> &roots, const QList<QMap<QByteArray, qint64>> &limits, const QList<QMap<QByteArray, qint64>> &usages);

    QList<QByteArray> roots() const;
    QList<QMap<QByteArray, qint64>> limits() const;
    QList<QMap<QByteArray, qint64>> usages() const;

    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    QList<QByteArray> mRoots;
    QList<QMap<QByteArray, qint64>> mLimits;
    QList<QMap<QByteArray, qint64>> mUsages;
};
}
