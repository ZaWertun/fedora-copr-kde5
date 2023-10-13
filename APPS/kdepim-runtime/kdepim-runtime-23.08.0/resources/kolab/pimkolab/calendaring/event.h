/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once
#include <kolabevent.h>

#include "kolab_export.h"
#include <icalendar/icalendar.h>

namespace Kolab
{
namespace Calendaring
{
class KOLAB_EXPORT Event : public Kolab::Event
{
public:
    Event();
    Event(const Kolab::Event &);
    ~Event();

    bool read(const std::string &);
    /**
     * Convert to kolab xml format.
     */
    std::string write() const;

    bool fromMime(const std::string &);

    /**
     * Convert to kolab mime format.
     */
    std::string toMime() const;

    enum ITipMethod {
        iTIPPublish, /**< Event, to-do, journal or freebusy posting */
        iTIPRequest, /**< Event, to-do or freebusy scheduling request */
        iTIPReply, /**< Event, to-do or freebusy reply to request */
        iTIPAdd, /**< Event, to-do or journal additional property request */
        iTIPCancel, /**< Event, to-do or journal cancellation notice */
        iTIPRefresh, /**< Event or to-do description update request */
        iTIPCounter, /**< Event or to-do submit counter proposal */
        iTIPDeclineCounter, /**< Event or to-do decline a counter proposal */
        iTIPNoMethod /**< No method */
    };

    bool fromICal(const std::string &);
    std::string toICal() const;

    bool fromIMip(const std::string &);
    std::string toIMip(ITipMethod method) const;

    /**
     * Returns the scheduling method from the last fromIMip call
     */
    ITipMethod getSchedulingMethod() const;

    /**
     * Updates the delegators and delegatees of the event.
     *
     * Creates a new attendee for each missing delegatee (delegators are expected to be existing), and then updates each delegatee with the delegator
     * (delegatedFrom). Delegators delegatedTo is updated accordingly. Existing attendees are tried to be found by uid/email/name (in this order).
     *
     */
    void delegate(const std::vector<Kolab::Attendee> &delegators, const std::vector<Kolab::Attendee> &delegatees);

    /**
     * Get attendee by uid/email/name (in this order)
     */
    Kolab::Attendee getAttendee(const std::string &);

    /**
     * Returns the next occurrence for a recurring event.
     *
     * If the start date of the event is passed in, the second occurrence is returned (so it can be used in a for loop to loop through all occurrences).
     *
     * If there is no next occurrence or the event is not recurring at all an invalid cDateTime is returned.
     */
    Kolab::cDateTime getNextOccurence(const Kolab::cDateTime &);

    /**
     * Returns the corresponding end date-time for a specific occurrence.
     * @param start is the start date of the occurrence.
     */
    Kolab::cDateTime getOccurenceEndDate(const Kolab::cDateTime &start);

    /**
     * Returns the last occurrence, or and invalid cDateTime if the event is not recurring or recurring idenfinitely.
     */
    Kolab::cDateTime getLastOccurrence() const;

private:
    Kolab::Attendee *getAttendee(const ContactReference &);
    Kolab::ITipHandler mITipHandler;
};
}
}
