/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once
#include <QObject>

class ICalendarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    //     void testEventConflict_data();
    void testToICal();
    void testFromICalEvent();

    void testToITip();
    void testToIMip();
};
