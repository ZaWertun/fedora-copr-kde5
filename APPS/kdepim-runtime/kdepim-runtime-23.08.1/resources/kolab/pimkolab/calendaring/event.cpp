/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "event.h"
#include <conversion/commonconversion.h>
#include <conversion/kcalconversion.h>
#include <kolabformat/kolabobject.h>

#include <iostream>
#include <kolabevent_p.h>
#include <kolabformat.h>

namespace Kolab
{
namespace Calendaring
{
Event::Event()
    : Kolab::Event()
{
    setUid(Kolab::generateUID());
}

Event::Event(const Kolab::Event &e)
    : Kolab::Event(e)
{
}

Event::~Event() = default;

bool Event::read(const std::string &string)
{
    const Kolab::Event &e = Kolab::readEvent(string, false);
    if (Kolab::error()) {
        return false;
    }
    Kolab::Event::operator=(e);
    return true;
}

std::string Event::write() const
{
    return Kolab::writeEvent(*this);
}

bool Event::fromMime(const std::string &input)
{
    KMime::Message::Ptr msg = KMime::Message::Ptr(new KMime::Message);
    msg->setContent(KMime::CRLFtoLF(Kolab::Conversion::fromStdString(input).toUtf8()));
    msg->parse();
    msg->content(KMime::ContentIndex());
    KolabObjectReader reader(msg);
    if (reader.getType() != EventObject) {
        std::cout << "not an event ";
        return false;
    }
    const Kolab::Event &e = Kolab::Conversion::fromKCalendarCore(*reader.getEvent());
    Kolab::Event::operator=(e);
    return true;
}

std::string Event::toMime() const
{
    return std::string(KolabObjectWriter::writeEvent(Kolab::Conversion::toKCalendarCore(*this))->encodedContent().constData());
}

bool Event::fromICal(const std::string &input)
{
    std::vector<Kolab::Event> list = fromICalEvents(input);
    if (list.size() != 1) {
        std::cout << "invalid number of events: " << list.size();
        return false;
    }
    Kolab::Event::operator=(list.at(0));
    return true;
}

std::string Event::toICal() const
{
    std::vector<Kolab::Event> list;
    list.push_back(*this);
    return Kolab::toICal(list);
}

bool Event::fromIMip(const std::string &input)
{
    const std::vector<Kolab::Event> list = mITipHandler.fromIMip(input);
    if (list.size() != 1) {
        std::cout << "invalid number of events: " << list.size();
        return false;
    }
    Kolab::Event::operator=(list.at(0));
    return true;
}

std::string Event::toIMip(ITipMethod method) const
{
    std::vector<Kolab::Event> list;
    list.push_back(*this);
    return mITipHandler.toIMip(*this, static_cast<ITipHandler::ITipMethod>(method), organizer().email());
}

Calendaring::Event::ITipMethod Event::getSchedulingMethod() const
{
    Q_ASSERT((int)iTIPPublish == (int)ITipHandler::iTIPPublish);
    Q_ASSERT((int)iTIPNoMethod == (int)ITipHandler::iTIPNoMethod);
    return static_cast<ITipMethod>(mITipHandler.method());
}

bool contains(const Kolab::ContactReference &delegatorRef, const std::vector<Kolab::ContactReference> &list)
{
    for (const Kolab::ContactReference &ref : list) {
        if (delegatorRef.uid() == ref.uid() || delegatorRef.email() == ref.email() || delegatorRef.name() == ref.name()) {
            return true;
        }
    }
    return false;
}

void Event::delegate(const std::vector<Attendee> &delegators, const std::vector<Attendee> &delegatees)
{
    // First build a list of attendee references, and insert any missing attendees
    std::vector<Kolab::Attendee *> delegateesRef;
    for (const Attendee &a : delegatees) {
        if (Attendee *attendee = getAttendee(a.contact())) {
            delegateesRef.push_back(attendee);
        } else {
            d->attendees.push_back(a);
            delegateesRef.push_back(&d->attendees.back());
        }
    }

    std::vector<Kolab::Attendee *> delegatorsRef;
    for (const Attendee &a : delegators) {
        if (Attendee *attendee = getAttendee(a.contact())) {
            delegatorsRef.push_back(attendee);
        } else {
            std::cout << "missing delegator";
        }
    }

    for (Attendee *delegatee : std::as_const(delegateesRef)) {
        std::vector<Kolab::ContactReference> delegatedFrom = delegatee->delegatedFrom();
        for (Attendee *delegator : std::as_const(delegatorsRef)) {
            // Set the delegator on each delegatee
            const ContactReference &delegatorRef = delegator->contact();
            if (!contains(delegatorRef, delegatedFrom)) {
                delegatedFrom.emplace_back(Kolab::ContactReference::EmailReference, delegatorRef.email(), delegatorRef.name());
            }

            // Set the delegatee on each delegator
            std::vector<Kolab::ContactReference> delegatedTo = delegator->delegatedTo();
            const ContactReference &delegaeeRef = delegatee->contact();
            if (!contains(delegaeeRef, delegatedTo)) {
                delegatedTo.emplace_back(Kolab::ContactReference::EmailReference, delegaeeRef.email(), delegaeeRef.name());
            }
            delegator->setDelegatedTo(delegatedTo);
        }
        delegatee->setDelegatedFrom(delegatedFrom);
    }
}

Attendee *Event::getAttendee(const ContactReference &ref)
{
    for (auto it = d->attendees.begin(), end = d->attendees.end(); it != end; ++it) {
        if (it->contact().uid() == ref.uid() || it->contact().email() == ref.email() || it->contact().name() == ref.name()) {
            return &*it;
        }
    }
    return nullptr;
}

Attendee Event::getAttendee(const std::string &s)
{
    const auto atts{attendees()};
    for (const Attendee &a : atts) {
        if (a.contact().uid() == s || a.contact().email() == s || a.contact().name() == s) {
            return a;
        }
    }
    return {};
}

cDateTime Calendaring::Event::getNextOccurence(const cDateTime &date)
{
    KCalendarCore::Event::Ptr event = Kolab::Conversion::toKCalendarCore(*this);
    if (!event->recurs()) {
        return {};
    }
    const QDateTime nextDate = event->recurrence()->getNextDateTime(Kolab::Conversion::toDate(date));
    return Kolab::Conversion::fromDate(nextDate, date.isDateOnly());
}

cDateTime Calendaring::Event::getOccurenceEndDate(const cDateTime &startDate)
{
    KCalendarCore::Event::Ptr event = Kolab::Conversion::toKCalendarCore(*this);
    const QDateTime start = Kolab::Conversion::toDate(startDate);
    return Kolab::Conversion::fromDate(event->endDateForStart(start), event->allDay());
}

cDateTime Calendaring::Event::getLastOccurrence() const
{
    KCalendarCore::Event::Ptr event = Kolab::Conversion::toKCalendarCore(*this);
    if (!event->recurs()) {
        return {};
    }
    const QDateTime endDate = event->recurrence()->endDateTime();
    return Kolab::Conversion::fromDate(endDate, event->allDay());
}
}
}
