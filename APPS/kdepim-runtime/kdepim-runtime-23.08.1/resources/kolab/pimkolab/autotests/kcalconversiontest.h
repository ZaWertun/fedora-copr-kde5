/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <QObject>

class KCalConversionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testDate_data();
    void testDate();

    void testDuration_data();
    void testDuration();

    void testConversion_data();
    void testConversion();

    void testTodoConversion_data();
    void testTodoConversion();

    void testJournalConversion_data();
    void testJournalConversion();

    void testContactConversion_data();
    void testContactConversion();

    void testDateTZ_data();
    void testDateTZ();
};
