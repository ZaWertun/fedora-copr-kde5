/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"
#include <KCalendarCore/Event>
#include <kolabevent.h>
#include <kolabfreebusy.h>

namespace Kolab
{
namespace FreebusyUtils
{
KOLAB_EXPORT Freebusy generateFreeBusy(const QVector<KCalendarCore::Event::Ptr> &events,
                                       const QDateTime &startDate,
                                       const QDateTime &endDate,
                                       const KCalendarCore::Person &organizer,
                                       bool allDay);
KOLAB_EXPORT std::string toIFB(const Kolab::Freebusy &);

Kolab::Freebusy generateFreeBusy(const std::vector<Kolab::Event> &events, const Kolab::cDateTime &startDate, const Kolab::cDateTime &endDate);
KOLAB_EXPORT Kolab::Freebusy
aggregateFreeBusy(const std::vector<Kolab::Freebusy> &fbs, const std::string &organizerEmail, const std::string &organizerName, bool simple = true);
}
}
