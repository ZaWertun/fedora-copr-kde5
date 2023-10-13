/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "calendaringtest.h"

#include <QTest>
#include <calendaring/calendaring.h>
#include <calendaring/datetimeutils.h>
#include <calendaring/event.h>
#include <iostream>
#include <kolabevent.h>

#include "testhelpers.h"
#include "testutils.h"

void compareEvents(const std::vector<Kolab::Event> &list1, const std::vector<Kolab::Event> &list2)
{
    QCOMPARE(list1.size(), list2.size());
    for (std::size_t i = 0; i < list1.size(); i++) {
        const Kolab::Event &e1 = list1.at(i);
        const Kolab::Event &e2 = list2.at(i);
        //         qDebug() << i;
        //         QCOMPARE(e1.uid(), e2.uid());
        QCOMPARE(e1.start(), e2.start());
        QCOMPARE(e1.end(), e2.end());
    }
}

void CalendaringTest::initTestCase()
{
}

void CalendaringTest::testCalendaringEvent()
{
    Kolab::Event event;
    event.setUid("uid");
    event.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
    event.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));
    Kolab::Calendaring::Event calEvent(event);

    QCOMPARE(event.start(), calEvent.start());
    QCOMPARE(event.uid(), calEvent.uid());

    Kolab::Calendaring::Event calEvent2;
    Kolab::Calendaring::Event calEvent3 = calEvent2;
    QVERIFY(!calEvent2.uid().empty());
    QCOMPARE(calEvent2.uid(), calEvent3.uid());
}

void CalendaringTest::testEventConflict_data()
{
    QTest::addColumn<Kolab::Event>("e1");
    QTest::addColumn<Kolab::Event>("e2");
    QTest::addColumn<bool>("result");
    {
        Kolab::Event e1;
        e1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
        e1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));

        Kolab::Event e2;
        e2.setStart(Kolab::cDateTime(2011, 11, 10, 12, 1, 1, true));
        e2.setEnd(Kolab::cDateTime(2011, 11, 11, 12, 1, 1, true));

        QTest::newRow("after") << e1 << e2 << false;
    }

    {
        Kolab::Event e1;
        e1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
        e1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));

        Kolab::Event e2;
        e2.setStart(Kolab::cDateTime(2011, 9, 10, 12, 1, 1, true));
        e2.setEnd(Kolab::cDateTime(2011, 9, 11, 12, 1, 1, true));

        QTest::newRow("before") << e1 << e2 << false;
    }

    {
        Kolab::Event e1;
        e1.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
        e1.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));

        Kolab::Event e2;
        e2.setStart(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true));
        e2.setEnd(Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));

        QTest::newRow("conflict") << e1 << e2 << true;
    }

    {
        Kolab::Event e1;
        e1.setStart(Kolab::cDateTime("Europe/Zurich", 2011, 10, 10, 6, 1, 1));
        e1.setEnd(Kolab::cDateTime("Europe/Zurich", 2011, 10, 10, 6, 1, 2));

        Kolab::Event e2;
        e2.setStart(Kolab::cDateTime("Asia/Dubai", 2011, 10, 10, 6, 1, 1));
        e2.setEnd(Kolab::cDateTime("Asia/Dubai", 2011, 10, 10, 6, 1, 2));

        QTest::newRow("tz non-conflict") << e1 << e2 << false;
    }

    {
        Kolab::Event e1;
        e1.setStart(Kolab::cDateTime("Europe/Berlin", 2011, 10, 10, 6, 1, 1));
        e1.setEnd(Kolab::cDateTime("Europe/Berlin", 2011, 10, 10, 6, 1, 2));

        Kolab::Event e2;
        e2.setStart(Kolab::cDateTime("Europe/Zurich", 2011, 10, 10, 6, 1, 1));
        e2.setEnd(Kolab::cDateTime("Europe/Zurich", 2011, 10, 10, 6, 1, 2));

        QTest::newRow("tz conflict") << e1 << e2 << true;
    }
}

void CalendaringTest::testEventConflict()
{
    QFETCH(Kolab::Event, e1);
    QFETCH(Kolab::Event, e2);
    QFETCH(bool, result);
    QCOMPARE(Kolab::Calendaring::conflicts(e1, e2), result);
}

void CalendaringTest::testEventConflictSet()
{
    std::vector<Kolab::Event> events;
    events.push_back(createEvent(Kolab::cDateTime(2011, 10, 6, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 8, 12, 1, 1, true)));
    events.push_back(createEvent(Kolab::cDateTime(2011, 10, 7, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true)));
    events.push_back(createEvent(Kolab::cDateTime(2011, 10, 9, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true)));
    const std::vector<std::vector<Kolab::Event>> &result = Kolab::Calendaring::getConflictingSets(events);

    std::vector<std::vector<Kolab::Event>> expectedResult;
    std::vector<Kolab::Event> r1;
    r1.push_back(createEvent(Kolab::cDateTime(2011, 10, 6, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 8, 12, 1, 1, true)));
    r1.push_back(createEvent(Kolab::cDateTime(2011, 10, 7, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true)));
    expectedResult.push_back(r1);
    std::vector<Kolab::Event> r2;
    r2.push_back(createEvent(Kolab::cDateTime(2011, 10, 7, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true)));
    r2.push_back(createEvent(Kolab::cDateTime(2011, 10, 9, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true)));
    expectedResult.push_back(r2);

    for (std::size_t i = 0; i < result.size(); i++) {
        const std::vector<Kolab::Event> &list = result.at(i);

        qDebug() << "---------_Set--------------";
        for (const Kolab::Event &event : list) {
            qDebug() << QTest::toString(event.start()) << QTest::toString(event.end());
        }
        compareEvents(result.at(i), expectedResult.at(i));
    }
}

void CalendaringTest::testTimesInInterval_data()
{
    QTest::addColumn<Kolab::Event>("event");
    QTest::addColumn<Kolab::cDateTime>("start");
    QTest::addColumn<Kolab::cDateTime>("end");
    QTest::addColumn<std::vector<Kolab::cDateTime>>("result");
    {
        {
            Kolab::Event event;
            event.setStart(Kolab::cDateTime(2011, 1, 1, 1, 1, 1, true));
            event.setEnd(Kolab::cDateTime(2011, 1, 1, 2, 1, 1, true));
            Kolab::RecurrenceRule rrule;
            rrule.setFrequency(Kolab::RecurrenceRule::Daily);
            rrule.setInterval(1);
            rrule.setCount(5);
            event.setRecurrenceRule(rrule);

            std::vector<Kolab::cDateTime> result;
            result.emplace_back(2011, 1, 1, 1, 1, 1, true);
            result.emplace_back(2011, 1, 2, 1, 1, 1, true);
            result.emplace_back(2011, 1, 3, 1, 1, 1, true);
            result.emplace_back(2011, 1, 4, 1, 1, 1, true);
            result.emplace_back(2011, 1, 5, 1, 1, 1, true);
            QTest::newRow("simple") << event << Kolab::cDateTime(2011, 1, 1, 1, 1, 1, true) << Kolab::cDateTime(2011, 1, 5, 1, 1, 1, true) << result;
        }
    }
}

void CalendaringTest::testTimesInInterval()
{
    QFETCH(Kolab::Event, event);
    QFETCH(Kolab::cDateTime, start);
    QFETCH(Kolab::cDateTime, end);
    QFETCH(std::vector<Kolab::cDateTime>, result);
    QCOMPARE(Kolab::Calendaring::timeInInterval(event, start, end), result);
}

void CalendaringTest::testTimesInIntervalBenchmark()
{
    Kolab::Event event;
    event.setStart(Kolab::cDateTime(2011, 1, 1, 1, 1, 1));
    event.setEnd(Kolab::cDateTime(2011, 1, 1, 2, 1, 1));
    Kolab::RecurrenceRule rrule;
    rrule.setFrequency(Kolab::RecurrenceRule::Daily);
    rrule.setInterval(1);
    rrule.setCount(500);
    event.setRecurrenceRule(rrule);

    QBENCHMARK {
        Kolab::Calendaring::timeInInterval(event, Kolab::cDateTime(2011, 1, 1, 1, 1, 1), Kolab::cDateTime(2013, 1, 1, 1, 1, 1));
    }
    const std::vector<Kolab::cDateTime> &result =
        Kolab::Calendaring::timeInInterval(event, Kolab::cDateTime(2011, 1, 1, 1, 1, 1), Kolab::cDateTime(2013, 1, 1, 1, 1, 1));
    QVERIFY(result.size() == 500);
    //     qDebug() << QTest::toString(result);
}

void CalendaringTest::testCalendar_data()
{
    QTest::addColumn<std::vector<Kolab::Event>>("inputevents");
    QTest::addColumn<Kolab::cDateTime>("start");
    QTest::addColumn<Kolab::cDateTime>("end");
    QTest::addColumn<std::vector<Kolab::Event>>("expectedResult");

    {
        std::vector<Kolab::Event> inputevents;
        for (int day = 1; day < 28; day++) {
            for (int hour = 1; hour < 20; hour += 2) {
                inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, day, hour, 4, 4, true), Kolab::cDateTime(2012, 5, day, hour + 1, 4, 4, true)));
            }
        }
        std::vector<Kolab::Event> expectedResult;
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 3, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 3 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 5, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 5 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 7, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 7 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 9, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 9 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 11, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 11 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 13, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 13 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 15, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 15 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 17, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 17 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 19, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 19 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 6, 1, 4, 4, true), Kolab::cDateTime(2012, 5, 6, 1 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 6, 3, 4, 4, true), Kolab::cDateTime(2012, 5, 6, 3 + 1, 4, 4, true)));
        QTest::newRow("simple") << inputevents << Kolab::cDateTime(2012, 5, 5, 4, 4, 4, true) << Kolab::cDateTime(2012, 5, 6, 4, 4, 4, true) << expectedResult;
    }

    { // Start and end time inclusive
        std::vector<Kolab::Event> inputevents;
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 1, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 1 + 1, 4, 4, true)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 3, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 3 + 1, 4, 4, true)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 5, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 5 + 1, 4, 4, true)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 7, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 7 + 1, 4, 4, true)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 9, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 9 + 1, 4, 4, true)));

        std::vector<Kolab::Event> expectedResult;
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 3, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 3 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 5, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 5 + 1, 4, 4, true)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 7, 4, 4, true), Kolab::cDateTime(2012, 5, 5, 7 + 1, 4, 4, true)));
        QTest::newRow("startEndTimeInclusive") << inputevents << Kolab::cDateTime(2012, 5, 5, 3, 4, 4, true) << Kolab::cDateTime(2012, 5, 5, 7, 4, 4, true)
                                               << expectedResult;
    }

    { // Start and end time inclusive (floating time)
        std::vector<Kolab::Event> inputevents;
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 1, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 1 + 1, 4, 4, false)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 3, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 3 + 1, 4, 4, false)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 5, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 5 + 1, 4, 4, false)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 7, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 7 + 1, 4, 4, false)));
        inputevents.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 9, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 9 + 1, 4, 4, false)));

        std::vector<Kolab::Event> expectedResult;
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 3, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 3 + 1, 4, 4, false)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 5, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 5 + 1, 4, 4, false)));
        expectedResult.push_back(createEvent(Kolab::cDateTime(2012, 5, 5, 7, 4, 4, false), Kolab::cDateTime(2012, 5, 5, 7 + 1, 4, 4, false)));
        QTest::newRow("startEndTimeInclusive") << inputevents << Kolab::cDateTime(2012, 5, 5, 3, 4, 4, false) << Kolab::cDateTime(2012, 5, 5, 7, 4, 4, false)
                                               << expectedResult;
    }

    { // Start and end time inclusive (timezone)
        std::vector<Kolab::Event> inputevents;
        inputevents.push_back(createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 1, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 1 + 1, 4, 4)));
        inputevents.push_back(createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3 + 1, 4, 4)));
        inputevents.push_back(createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 5, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 5 + 1, 4, 4)));
        inputevents.push_back(createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 7, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 7 + 1, 4, 4)));
        inputevents.push_back(createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 9, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 9 + 1, 4, 4)));

        std::vector<Kolab::Event> expectedResult;
        expectedResult.push_back(
            createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3 + 1, 4, 4)));
        expectedResult.push_back(
            createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 5, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 5 + 1, 4, 4)));
        expectedResult.push_back(
            createEvent(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 7, 4, 4), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 7 + 1, 4, 4)));
        QTest::newRow("startEndTimeInclusive") << inputevents << Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4)
                                               << Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 7, 4, 4) << expectedResult;
    }
}

void CalendaringTest::testCalendar()
{
    QFETCH(std::vector<Kolab::Event>, inputevents);
    QFETCH(Kolab::cDateTime, start);
    QFETCH(Kolab::cDateTime, end);
    QFETCH(std::vector<Kolab::Event>, expectedResult);

    Kolab::Calendaring::Calendar cal;
    for (const Kolab::Event &event : std::as_const(inputevents)) {
        cal.addEvent(event);
    }
    const std::vector<Kolab::Event> result = cal.getEvents(start, end, true);
    for (const Kolab::Event &event : result) {
        qDebug() << QTest::toString(event.start()) << QTest::toString(event.end());
    }
    compareEvents(result, expectedResult);
}

void CalendaringTest::delegationTest()
{
    Kolab::Calendaring::Event event;
    event.setStart(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));

    Kolab::Attendee att1(Kolab::ContactReference("email1", "name1", "uid1"));
    att1.setCutype(Kolab::CutypeIndividual);
    Kolab::Attendee att2(Kolab::ContactReference("email2", "name2", "uid2"));
    Kolab::Attendee att3(Kolab::ContactReference("email3", "name3", "uid3"));
    Kolab::Attendee att4(Kolab::ContactReference("email4", "name4", "uid4"));

    std::vector<Kolab::Attendee> attendees;
    attendees.push_back(att1);
    attendees.push_back(att2);
    attendees.push_back(att3);
    event.setAttendees(attendees);

    std::vector<Kolab::Attendee> delegators;
    delegators.push_back(att1);
    delegators.push_back(att2);

    std::vector<Kolab::Attendee> delegatees;
    delegatees.push_back(att3);
    delegatees.push_back(att4);

    event.delegate(delegators, delegatees);

    std::cout << event.write();
    // TODO write an actual test
}

void CalendaringTest::testICal()
{
    Kolab::Calendaring::Event event;
    event.setStart(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
    const std::string &result = event.toICal();
    // TODO write an actual test
    event.setStart(Kolab::cDateTime(1, 1, 1));
    event.fromICal(result);
    QCOMPARE(event.start(), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
}

void CalendaringTest::testMime()
{
    Kolab::Calendaring::Event event;
    event.setStart(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
    std::cout << event.toMime();
    const std::string &result = event.toMime();
    event.setStart(Kolab::cDateTime(1, 1, 1));
    event.fromMime(result);
    QCOMPARE(event.start(), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
}

void CalendaringTest::testIMip()
{
    Kolab::Calendaring::Event event;
    Kolab::Attendee att1(Kolab::ContactReference("email1", "name1", "uid1"));

    std::vector<Kolab::Attendee> attendees;
    attendees.push_back(att1);
    event.setAttendees(attendees);
    event.setOrganizer(Kolab::ContactReference("organizer@test.org", "organizer", "uid3"));
    event.setSummary("summary");
    event.setStart(Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
    const std::string &result = event.toIMip(Kolab::Calendaring::Event::iTIPRequest);
    event.setStart(Kolab::cDateTime(1, 1, 1));
    event.fromIMip(result);
    QEXPECT_FAIL("", "returns UTC instead of local timezone", Continue);
    QCOMPARE(event.start(), Kolab::cDateTime("Europe/Zurich", 2012, 5, 5, 3, 4, 4));
    QCOMPARE(event.getSchedulingMethod(), Kolab::Calendaring::Event::iTIPRequest);
}

void CalendaringTest::testRecurrence()
{
    Kolab::Calendaring::Event event;
    event.setStart(Kolab::cDateTime(2011, 1, 1, 1, 1, 1));
    event.setEnd(Kolab::cDateTime(2011, 1, 1, 2, 1, 1));
    Kolab::RecurrenceRule rrule;
    rrule.setFrequency(Kolab::RecurrenceRule::Daily);
    rrule.setInterval(1);
    rrule.setCount(10);
    event.setRecurrenceRule(rrule);

    Kolab::cDateTime previousDate = event.start();
    for (int i = 0; i < 9; i++) {
        const Kolab::cDateTime nextDate = event.getNextOccurence(previousDate);
        //         qDebug() << QTest::toString(nextDate);
        QCOMPARE(nextDate,
                 Kolab::cDateTime(previousDate.year(),
                                  previousDate.month(),
                                  previousDate.day() + 1,
                                  previousDate.hour(),
                                  previousDate.minute(),
                                  previousDate.second()));
        const Kolab::cDateTime endDate = event.getOccurenceEndDate(nextDate);
        //         qDebug() << QTest::toString(endDate);
        QCOMPARE(endDate, Kolab::cDateTime(nextDate.year(), nextDate.month(), nextDate.day(), event.end().hour(), event.end().minute(), event.end().second()));
        previousDate = nextDate;
    }

    Kolab::cDateTime outOfScopeDate = event.getNextOccurence(previousDate);
    QVERIFY(!outOfScopeDate.isValid());
}

void CalendaringTest::testDateTimeUtils()
{
    std::cout << Kolab::DateTimeUtils::getLocalTimezone() << std::endl;
}

QTEST_MAIN(CalendaringTest)
