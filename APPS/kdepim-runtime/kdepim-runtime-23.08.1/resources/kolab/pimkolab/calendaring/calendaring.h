/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <KCalendarCore/Event>
#include <KCalendarCore/MemoryCalendar>
#include <kolabevent.h>
#include <memory>

namespace Kolab
{
namespace Calendaring
{
/**
 * Returns true if the events conflict (overlap)
 * Start and end date/time is inclusive.
 *
 * Does not take recurrences into account.
 */
KOLAB_EXPORT bool conflicts(const Kolab::Event &, const Kolab::Event &);

/**
 * Returns sets of the events which are directly conflicting with each other.
 * The same event may appear in multiple sets.
 * Non-conflicting events are not returned.
 * conflicts() is used for conflict detection.
 *
 * If the second list is given, each event from the first list is additionally checked against each event of the second set.
 * Conflicts within the second list are not detected.
 *
 * The checked event from the first list comes always first in the returned set.
 */
KOLAB_EXPORT std::vector<std::vector<Kolab::Event>> getConflictingSets(const std::vector<Kolab::Event> &,
                                                                       const std::vector<Kolab::Event> & = std::vector<Kolab::Event>());

/**
 * Returns the dates in which the event recurs within the specified timespan.
 */
KOLAB_EXPORT std::vector<Kolab::cDateTime> timeInInterval(const Kolab::Event &, const Kolab::cDateTime &start, const Kolab::cDateTime &end);

/**
 * In-Memory Calendar Cache
 */
class KOLAB_EXPORT Calendar
{
public:
    explicit Calendar();
    /**
     * Add an event to the in-memory calendar.
     */
    void addEvent(const Kolab::Event &);
    /**
     * Returns all events within the specified interval (start and end inclusive).
     *
     * @param sort controls if the resulting event set is sorted in ascending order according to the start date
     */
    std::vector<Kolab::Event> getEvents(const Kolab::cDateTime &start, const Kolab::cDateTime &end, bool sort);

private:
    Calendar(const Calendar &) = delete;
    void operator=(const Calendar &) = delete;
    std::unique_ptr<KCalendarCore::MemoryCalendar> mCalendar;
};
} // Namespace
} // Namespace
