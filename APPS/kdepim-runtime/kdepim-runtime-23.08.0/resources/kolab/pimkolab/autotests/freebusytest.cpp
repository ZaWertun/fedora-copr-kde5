/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "freebusytest.h"
#include "testhelpers.h"
#include "testutils.h"

#include "freebusy/freebusy.h"
#include <QTest>
#include <kolabfreebusy.h>

#include <iostream>

void FreebusyTest::testFB_data()
{
    QTest::addColumn<Kolab::cDateTime>("start");
    QTest::addColumn<Kolab::cDateTime>("end");
    QTest::addColumn<std::vector<Kolab::Event>>("events");
    QTest::addColumn<std::vector<Kolab::FreebusyPeriod>>("output");

    // UTC check
    {
        Kolab::Period p1(Kolab::cDateTime(2011, 10, 6, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 7, 12, 1, 1, true));
        Kolab::Period p2(Kolab::cDateTime(2011, 10, 8, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 9, 12, 1, 1, true));
        Kolab::Period p3(Kolab::cDateTime(2011, 10, 10, 12, 1, 1, true), Kolab::cDateTime(2011, 10, 11, 12, 1, 1, true));

        std::vector<Kolab::Event> events;
        events.push_back(createEvent(p1.start, p1.end));
        events.push_back(createEvent(p2.start, p2.end));
        events.push_back(createEvent(p3.start, p3.end));

        Kolab::FreebusyPeriod period1;
        period1.setType(Kolab::FreebusyPeriod::Busy);
        period1.setEvent(events.at(0).uid(), events.at(0).summary(), events.at(0).location());
        period1.setPeriods(std::vector<Kolab::Period>() << p1);

        Kolab::FreebusyPeriod period2;
        period2.setType(Kolab::FreebusyPeriod::Busy);
        period2.setEvent(events.at(1).uid(), events.at(1).summary(), events.at(1).location());
        period2.setPeriods(std::vector<Kolab::Period>() << p2);

        Kolab::FreebusyPeriod period3;
        period3.setType(Kolab::FreebusyPeriod::Busy);
        period3.setEvent(events.at(2).uid(), events.at(2).summary(), events.at(2).location());
        period3.setPeriods(std::vector<Kolab::Period>() << p3);

        {
            std::vector<Kolab::FreebusyPeriod> output;
            output.push_back(period1);
            output.push_back(period2);
            output.push_back(period3);

            QTest::newRow("simple utc") << Kolab::cDateTime(2010, 1, 1, 1, 1, 1, true) << Kolab::cDateTime(2012, 1, 1, 1, 1, 1, true) << events << output;
        }

        {
            std::vector<Kolab::FreebusyPeriod> output;
            output.push_back(period2);
            output.push_back(period3);

            QTest::newRow("limit utc below") << Kolab::cDateTime(2011, 10, 8, 12, 1, 1, true) << Kolab::cDateTime(2012, 1, 1, 1, 1, 1, true) << events
                                             << output;
        }
        {
            std::vector<Kolab::FreebusyPeriod> output;
            output.push_back(period1);
            output.push_back(period2);

            QTest::newRow("limit utc above") << Kolab::cDateTime(2010, 1, 1, 1, 1, 1, true) << Kolab::cDateTime(2011, 10, 9, 12, 1, 1, true) << events
                                             << output;
        }
    }
    {
        Kolab::Event event;
        event.setUid(QUuid::createUuid().toString().toStdString());
        event.setStart(Kolab::cDateTime(2011, 1, 1, 0, 0, 0, true));
        event.setEnd(Kolab::cDateTime(2011, 1, 1, 1, 0, 0, true));
        Kolab::RecurrenceRule rrule;
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setInterval(1);
        rrule.setCount(2);
        event.setRecurrenceRule(rrule);

        std::vector<Kolab::Event> events;
        events.push_back(event);

        std::vector<Kolab::FreebusyPeriod> output;
        Kolab::FreebusyPeriod period1;
        period1.setType(Kolab::FreebusyPeriod::Busy);
        period1.setEvent(event.uid(), event.summary(), event.location());
        period1.setPeriods(std::vector<Kolab::Period>()
                           << Kolab::Period(Kolab::cDateTime(2011, 1, 1, 0, 0, 0, true), Kolab::cDateTime(2011, 1, 1, 1, 0, 0, true))
                           << Kolab::Period(Kolab::cDateTime(2011, 1, 2, 0, 0, 0, true), Kolab::cDateTime(2011, 1, 2, 1, 0, 0, true)));
        output.push_back(period1);

        QTest::newRow("fullday recurrence") << Kolab::cDateTime(2010, 1, 1, 1, 1, 1, true) << Kolab::cDateTime(2012, 10, 9, 12, 1, 1, true) << events << output;
    }
}

void FreebusyTest::testFB()
{
    QFETCH(Kolab::cDateTime, start);
    QFETCH(Kolab::cDateTime, end);
    QFETCH(std::vector<Kolab::Event>, events);
    QFETCH(std::vector<Kolab::FreebusyPeriod>, output);

    Kolab::Freebusy fb = Kolab::FreebusyUtils::generateFreeBusy(events, start, end);

    QCOMPARE((int)fb.periods().size(), (int)output.size());
    for (std::size_t i = 0; i < output.size(); i++) {
        std::cout << i;
        QCOMPARE(fb.periods().at(i), output.at(i));
    }

    std::cout << Kolab::FreebusyUtils::toIFB(fb);
}

// void FreebusyTest::testHonorTimeFrame()
// {
//
//     Kolab::Period p1(Kolab::cDateTime(2011,10,6,12,1,1,true), Kolab::cDateTime(2011,10,8,12,1,1,true));
//     Kolab::Period p2(Kolab::cDateTime(2011,10,7,12,1,1,true), Kolab::cDateTime(2011,10,10,12,1,1,true));
//     Kolab::Period p3(Kolab::cDateTime(2011,10,9,12,1,1,true), Kolab::cDateTime(2011,10,11,12,1,1,true));
//
//     std::vector<Kolab::Event> events;
//     events.push_back(createEvent(p1.start, p1.end));
//     events.push_back(createEvent(p2.start, p2.end));
//     events.push_back(createEvent(p3.start, p3.end));
//
//     Kolab::FreebusyPeriod period1;
//     period1.setType(Kolab::FreebusyPeriod::Busy);
//     period1.setPeriods(std::vector<Kolab::Period>() << p1);
//
//     Kolab::FreebusyPeriod period2;
//     period2.setType(Kolab::FreebusyPeriod::Busy);
//     period2.setPeriods(std::vector<Kolab::Period>() << p2);
//
//     Kolab::FreebusyPeriod period3;
//     period3.setType(Kolab::FreebusyPeriod::Busy);
//     period3.setPeriods(std::vector<Kolab::Period>() << p3);
//
//
//     Kolab::Freebusy fb =  Kolab::FreebusyUtils::generateFreeBusy(events, Kolab::cDateTime(2010, 1, 1,1,1,1,true), Kolab::cDateTime(2012, 1, 1,1,1,1,true));
//     QCOMPARE((int)fb.periods().size(), 3);
//     QCOMPARE(fb.periods().at(0), period1);
//     QCOMPARE(fb.periods().at(1), period2);
//     QCOMPARE(fb.periods().at(2), period3);
// }

QTEST_MAIN(FreebusyTest)
