/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "oxutils.h"

#include <QStringList>

using namespace OXA;

QString OXUtils::writeBoolean(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString OXUtils::writeNumber(qlonglong value)
{
    return QString::number(value);
}

QString OXUtils::writeString(const QString &value)
{
    QStringList lines = value.split(QLatin1Char('\n'));

    for (int i = 0; i < lines.count(); ++i) {
        lines[i].replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
        lines[i].replace(QLatin1Char('"'), QStringLiteral("\\\""));
    }

    return lines.join(QLatin1Char('\n'));
}

QString OXUtils::writeName(const QString &value)
{
    // TODO: assert on invalid names
    return value;
}

QString OXUtils::writeDateTime(const QDateTime &value)
{
    QString result;

    // workaround, as QDateTime does not support negative time_t values
    QDateTime Time_t_S(QDate(1970, 1, 1), QTime(0, 0, 0), Qt::UTC);

    if (value < Time_t_S) {
        result = QString::number(Time_t_S.secsTo(value));
    } else {
        result = QString::number(value.toUTC().toSecsSinceEpoch());
    }

    return QString(result + QLatin1String("000"));
}

QString OXUtils::writeDate(QDate value)
{
    return writeDateTime(QDateTime(value, QTime(0, 0, 0), Qt::UTC));
}

bool OXUtils::readBoolean(const QString &text)
{
    if (text == QLatin1String("true")) {
        return true;
    } else if (text == QLatin1String("false")) {
        return false;
    } else {
        Q_ASSERT(false);
        return false;
    }
}

qlonglong OXUtils::readNumber(const QString &text)
{
    return text.toLongLong();
}

QString OXUtils::readString(const QString &text)
{
    QString value(text);
    value.replace(QLatin1String("\\\""), QLatin1String("\""));
    value.replace(QLatin1String("\\\\"), QLatin1String("\\"));

    return value;
}

QString OXUtils::readName(const QString &text)
{
    return text;
}

QDateTime OXUtils::readDateTime(const QString &text)
{
    // remove the trailing '000', they exceed the integer dimension
    const int ticks = text.mid(0, text.length() - 3).toLongLong();

    // workaround, as QDateTime does not support negative time_t values
    QDateTime value;
    if (ticks < 0) {
        value = QDateTime::fromSecsSinceEpoch(0);
        value = value.addSecs(ticks);
    } else {
        value = QDateTime::fromSecsSinceEpoch(ticks);
    }

    return value;
}

QDate OXUtils::readDate(const QString &text)
{
    return readDateTime(text).date();
}
