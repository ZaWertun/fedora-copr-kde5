/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "calendaring.h"
#include "pimkolab_debug.h"

#include <QDate>
#include <QTimeZone>

#include "conversion/commonconversion.h"
#include "conversion/kcalconversion.h"

namespace Kolab
{
namespace Calendaring
{
bool conflicts(const Kolab::Event &e1, const Kolab::Event &e2)
{
    KCalendarCore::Event::Ptr k1 = Kolab::Conversion::toKCalendarCore(e1);
    KCalendarCore::Event::Ptr k2 = Kolab::Conversion::toKCalendarCore(e2);
    if (k2->dtEnd() < k1->dtStart()) {
        return false;
    } else if (k1->dtEnd() < k2->dtStart()) {
        return false;
    }
    return true;
}

std::vector<std::vector<Event>> getConflictingSets(const std::vector<Event> &events, const std::vector<Event> &events2)
{
    std::vector<std::vector<Kolab::Event>> ret;
    for (std::size_t i = 0; i < events.size(); i++) {
        std::vector<Kolab::Event> set;
        const Kolab::Event &event = events.at(i);
        set.push_back(event);
        for (std::size_t q = i + 1; q < events.size(); q++) {
            const Kolab::Event &e2 = events.at(q);
            if (conflicts(event, e2)) {
                set.push_back(e2);
            }
        }
        for (std::size_t m = 0; m < events2.size(); m++) {
            const Kolab::Event &e2 = events2.at(m);
            if (conflicts(event, e2)) {
                set.push_back(e2);
            }
        }
        if (set.size() > 1) {
            ret.push_back(set);
        }
    }
    return ret;
}

std::vector<Kolab::cDateTime> timeInInterval(const Kolab::Event &e, const Kolab::cDateTime &start, const Kolab::cDateTime &end)
{
    KCalendarCore::Event::Ptr k = Kolab::Conversion::toKCalendarCore(e);
    const KCalendarCore::DateTimeList list = k->recurrence()->timesInInterval(Kolab::Conversion::toDate(start), Kolab::Conversion::toDate(end));
    std::vector<Kolab::cDateTime> dtList;
    dtList.reserve(list.count());
    for (const QDateTime &dt : list) {
        dtList.push_back(Kolab::Conversion::fromDate(dt, start.isDateOnly()));
    }
    return dtList;
}

Calendar::Calendar()
    : mCalendar(new KCalendarCore::MemoryCalendar(Kolab::Conversion::getTimeSpec(true, std::string()))) // Always utc as it doesn't change anything anyways
{
}

void Calendar::addEvent(const Kolab::Event &event)
{
    KCalendarCore::Event::Ptr k = Kolab::Conversion::toKCalendarCore(event);
    if (!mCalendar->addEvent(k)) {
        qCWarning(PIMKOLAB_LOG) << "failed to add event";
    }
}

std::vector<Kolab::Event> Calendar::getEvents(const Kolab::cDateTime &start, const Kolab::cDateTime &end, bool sort)
{
    const QDateTime s = Kolab::Conversion::toDate(start);
    const QDateTime e = Kolab::Conversion::toDate(end);
    const QTimeZone tz = s.timeZone();
    KCalendarCore::Event::List list = mCalendar->events(s.date(), e.date(), tz, true);
    if (sort) {
        list = mCalendar->sortEvents(std::move(list), KCalendarCore::EventSortStartDate, KCalendarCore::SortDirectionAscending);
    }
    std::vector<Kolab::Event> eventlist;
    for (const KCalendarCore::Event::Ptr &event : std::as_const(list)) {
        // We have to filter the list by time
        if (event->dtEnd() >= s && e >= event->dtStart()) {
            eventlist.push_back(Kolab::Conversion::fromKCalendarCore(*event));
        }
    }
    return eventlist;
}
} // Namespace
} // Namespace
