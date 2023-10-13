/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "datetimeutils.h"

#include <QTimeZone>

namespace Kolab
{
namespace DateTimeUtils
{
std::string getLocalTimezone()
{
    const auto tz = QTimeZone::systemTimeZoneId();
    return tz.toStdString();
}
} // Namespace
} // Namespace
