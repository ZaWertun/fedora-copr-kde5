/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once
#include <QObject>

class TimezoneTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    /**
     * If this unittest fails, many others will follow.
     */
    void initTestCase();

    void testFromName();
    void testFromHardcodedList_data();
    void testFromHardcodedList();
    void testKolabObjectWriter();
    // void testKolabObjectReader();
    void testFindLegacyTimezone();
    void testIgnoreInvalidTimezone();
    void testUTCOffset();
    void localTimezone();
};
