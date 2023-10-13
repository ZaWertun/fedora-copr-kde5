/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QString>

namespace OXA
{
namespace OXUtils
{
Q_REQUIRED_RESULT QString writeBoolean(bool value);
Q_REQUIRED_RESULT QString writeNumber(qlonglong value);
Q_REQUIRED_RESULT QString writeString(const QString &value);
Q_REQUIRED_RESULT QString writeName(const QString &value);
Q_REQUIRED_RESULT QString writeDateTime(const QDateTime &value);
Q_REQUIRED_RESULT QString writeDate(QDate value);

Q_REQUIRED_RESULT bool readBoolean(const QString &text);
Q_REQUIRED_RESULT qlonglong readNumber(const QString &text);
Q_REQUIRED_RESULT QString readString(const QString &text);
Q_REQUIRED_RESULT QString readName(const QString &text);
Q_REQUIRED_RESULT QDateTime readDateTime(const QString &text);
Q_REQUIRED_RESULT QDate readDate(const QString &text);
}
}
