/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once
#include <QObject>

class CalendaringTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testCalendaringEvent();

    void testEventConflict_data();
    void testEventConflict();

    void testEventConflictSet();

    void testTimesInInterval_data();
    void testTimesInInterval();
    void testTimesInIntervalBenchmark();

    void testCalendar_data();
    void testCalendar();

    void delegationTest();

    void testMime();
    void testICal();
    void testIMip();

    void testRecurrence();

    void testDateTimeUtils();
};
