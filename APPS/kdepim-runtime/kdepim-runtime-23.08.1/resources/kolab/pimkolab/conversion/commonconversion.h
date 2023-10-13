/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <QDateTime>
#include <QStringList>
#include <QUrl>
#include <kolabcontainers.h>

class QTimeZone;

namespace Kolab
{
namespace Conversion
{
KOLAB_EXPORT QDateTime toDate(const Kolab::cDateTime &dt);
KOLAB_EXPORT cDateTime fromDate(const QDateTime &dt, bool isAllDay);
QStringList toStringList(const std::vector<std::string> &l);
std::vector<std::string> fromStringList(const QStringList &l);
/**
 * Returns a UTC, Floating Time or Timezone
 */
QTimeZone getTimeZone(const std::string &timezone);
QTimeZone getTimeSpec(bool isUtc, const std::string &timezone);

QUrl toMailto(const std::string &email, const std::string &name = std::string());
std::string fromMailto(const QUrl &mailtoUri, std::string &name);
QPair<std::string, std::string> fromMailto(const std::string &mailto);

inline std::string toStdString(const QString &s)
{
    return std::string(s.toUtf8().constData());
}

inline QString fromStdString(const std::string &s)
{
    return QString::fromUtf8(s.c_str());
}
}
}
