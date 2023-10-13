/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapquotaattribute.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <QDebug>

using namespace Akonadi;

ImapQuotaAttribute::ImapQuotaAttribute() = default;

Akonadi::ImapQuotaAttribute::ImapQuotaAttribute(const QList<QByteArray> &roots,
                                                const QList<QMap<QByteArray, qint64>> &limits,
                                                const QList<QMap<QByteArray, qint64>> &usages)
    : mRoots(roots)
    , mLimits(limits)
    , mUsages(usages)
{
    Q_ASSERT(roots.size() == limits.size());
    Q_ASSERT(roots.size() == usages.size());
}

void Akonadi::ImapQuotaAttribute::setQuotas(const QList<QByteArray> &roots,
                                            const QList<QMap<QByteArray, qint64>> &limits,
                                            const QList<QMap<QByteArray, qint64>> &usages)
{
    Q_ASSERT(roots.size() == limits.size());
    Q_ASSERT(roots.size() == usages.size());

    mRoots = roots;
    mLimits = limits;
    mUsages = usages;
}

QList<QByteArray> Akonadi::ImapQuotaAttribute::roots() const
{
    return mRoots;
}

QList<QMap<QByteArray, qint64>> Akonadi::ImapQuotaAttribute::limits() const
{
    return mLimits;
}

QList<QMap<QByteArray, qint64>> Akonadi::ImapQuotaAttribute::usages() const
{
    return mUsages;
}

QByteArray ImapQuotaAttribute::type() const
{
    static const QByteArray sType("imapquota");
    return sType;
}

Akonadi::Attribute *ImapQuotaAttribute::clone() const
{
    return new ImapQuotaAttribute(mRoots, mLimits, mUsages);
}

QByteArray ImapQuotaAttribute::serialized() const
{
    QByteArray result = "";

    // First the roots list
    for (const QByteArray &root : std::as_const(mRoots)) {
        result += root + " % ";
    }
    result.chop(3);

    result += " %%%% "; // Members separator

    // Then the limit maps list
    for (int i = 0; i < mLimits.size(); ++i) {
        const QMap<QByteArray, qint64> limits = mLimits[i];
        for (auto it = limits.cbegin(), end = limits.cend(); it != end; ++it) {
            result += it.key();
            result += " % "; // We use this separator as '%' is not allowed in keys or values
            result += QByteArray::number(it.value());
            result += " %% "; // Pairs separator
        }
        result.chop(4);
        result += " %%% "; // Maps separator
    }
    result.chop(5);

    result += " %%%% "; // Members separator

    // Then the usage maps list
    for (int i = 0; i < mUsages.size(); ++i) {
        const QMap<QByteArray, qint64> usages = mUsages[i];
        for (auto it = usages.cbegin(), end = usages.cend(); it != end; ++it) {
            result += it.key();
            result += " % "; // We use this separator as '%' is not allowed in keys or values
            result += QByteArray::number(it.value());
            result += " %% "; // Pairs separator
        }
        result.chop(4);
        result += " %%% "; // Maps separator
    }
    result.chop(5);

    return result;
}

void ImapQuotaAttribute::deserialize(const QByteArray &data)
{
    mRoots.clear();
    mLimits.clear();
    mUsages.clear();

    // Nothing was saved.
    if (data.trimmed().isEmpty()) {
        return;
    }

    QString string = QString::fromUtf8(data); // QByteArray has no proper split, so we're forced to convert to QString...

    QStringList members = string.split(QStringLiteral("%%%%"));

    // We expect exactly three members (roots, limits and usages), otherwise something is funky
    if (members.size() != 3) {
        qWarning() << "We didn't find exactly three members in this quota serialization";
        return;
    }

    QStringList roots = members[0].trimmed().split(QStringLiteral(" % "));
    for (const QString &root : std::as_const(roots)) {
        mRoots << root.trimmed().toUtf8();
    }

    const QStringList allLimits = members[1].trimmed().split(QStringLiteral("%%%"));

    for (const QString &limits : std::as_const(allLimits)) {
        QMap<QByteArray, qint64> limitsMap;
        const QStringList strLines = limits.split(QStringLiteral("%%"));
        QList<QByteArray> lines;
        lines.reserve(strLines.count());
        for (const QString &strLine : strLines) {
            lines << strLine.trimmed().toUtf8();
        }

        for (const QByteArray &line : std::as_const(lines)) {
            QByteArray trimmed = line.trimmed();
            int wsIndex = trimmed.indexOf('%');
            const QByteArray key = trimmed.mid(0, wsIndex).trimmed();
            const QByteArray value = trimmed.mid(wsIndex + 1, line.length() - wsIndex).trimmed();
            limitsMap[key] = value.toLongLong();
        }

        mLimits << limitsMap;
    }

    const QStringList allUsages = members[2].trimmed().split(QStringLiteral("%%%"));
    mUsages.reserve(allUsages.count());

    for (const QString &usages : std::as_const(allUsages)) {
        QMap<QByteArray, qint64> usagesMap;
        const QStringList strLines = usages.split(QStringLiteral("%%"));
        QList<QByteArray> lines;
        lines.reserve(strLines.count());
        for (const QString &strLine : std::as_const(strLines)) {
            lines << strLine.trimmed().toUtf8();
        }

        for (const QByteArray &line : std::as_const(lines)) {
            QByteArray trimmed = line.trimmed();
            int wsIndex = trimmed.indexOf('%');
            const QByteArray key = trimmed.mid(0, wsIndex).trimmed();
            const QByteArray value = trimmed.mid(wsIndex + 1, line.length() - wsIndex).trimmed();
            usagesMap[key] = value.toLongLong();
        }

        mUsages << usagesMap;
    }
}
