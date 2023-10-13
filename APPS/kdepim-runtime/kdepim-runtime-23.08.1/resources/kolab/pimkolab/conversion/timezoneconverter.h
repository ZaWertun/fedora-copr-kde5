/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <QString>

class TimezoneConverter
{
public:
    static QString normalizeTimezone(const QString &tz);

private:
    static QString fromCityName(const QString &tz);
    static QString fromHardcodedList(const QString &tz);
    static QString fromGMTOffsetTimezone(const QString &tz);
};
