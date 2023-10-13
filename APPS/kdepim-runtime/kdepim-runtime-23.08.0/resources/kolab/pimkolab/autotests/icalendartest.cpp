/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "icalendartest.h"

#include <QTest>
#include <kolabevent.h>

#include "icalendar/icalendar.h"

#include "testhelpers.h"

void ICalendarTest::testFromICalEvent()
{
    std::vector<Kolab::Event> events;
    Kolab::Event ev1;
    ev1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
    ev1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));
    events.push_back(ev1);
    events.push_back(ev1);
    const std::vector<Kolab::Event> &result = Kolab::fromICalEvents(Kolab::toICal(events));
    qDebug() << QString::fromStdString(Kolab::toICal(result));
}

void ICalendarTest::testToICal()
{
    std::vector<Kolab::Event> events;
    Kolab::Event ev1;
    ev1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
    ev1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));
    events.push_back(ev1);
    events.push_back(ev1);
    qDebug() << QString::fromStdString(Kolab::toICal(events));
}

void ICalendarTest::testToITip()
{
    Kolab::ITipHandler handler;
    Kolab::Event ev1;
    ev1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
    ev1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));
    ev1.setLastModified(Kolab::cDateTime(2011, 10, 11, 12, 1, 2, true));
    ev1.setCreated(Kolab::cDateTime(2011, 10, 11, 12, 1, 3, true));

    qDebug() << QString::fromStdString(handler.toITip(ev1, Kolab::ITipHandler::iTIPRequest));
}

void ICalendarTest::testToIMip()
{
    Kolab::ITipHandler handler;
    Kolab::Event ev1;
    ev1.setStart(Kolab::cDateTime("Europe/Zurich", 2011, 10, 10, 12, 1, 1));
    ev1.setEnd(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
    ev1.setLastModified(Kolab::cDateTime(2011, 10, 11, 12, 1, 2, true));
    ev1.setCreated(Kolab::cDateTime(2011, 10, 11, 12, 1, 3, true));

    std::vector<Kolab::Attendee> attendees;
    attendees.emplace_back(Kolab::ContactReference("email1@test.org", "name1", "uid1"));
    attendees.emplace_back(Kolab::ContactReference("email2@test.org", "name2", "uid2"));
    ev1.setAttendees(attendees);

    ev1.setOrganizer(Kolab::ContactReference("organizer@test.org", "organizer", "uid3"));
    ev1.setSummary("summary");

    const std::string mimeResult = handler.toIMip(ev1, Kolab::ITipHandler::iTIPRequest, "test@test.com");
    qDebug() << QString::fromStdString(mimeResult);
    qDebug() << QString::fromStdString(handler.toIMip(ev1, Kolab::ITipHandler::iTIPReply, "test@test.com"));

    const std::vector<Kolab::Event> &eventResult = handler.fromIMip(mimeResult);

    //     qDebug() << QString::fromStdString(Kolab::toICal(eventResult));

    QCOMPARE((int)eventResult.size(), 1);
    QEXPECT_FAIL("", "to imip converts dates to utc", Continue);
    QCOMPARE(eventResult.front().start(), ev1.start());
    QEXPECT_FAIL("", "to imip converts dates to utc", Continue);
    QCOMPARE(eventResult.front().end(), ev1.end());
}

QTEST_MAIN(ICalendarTest)
