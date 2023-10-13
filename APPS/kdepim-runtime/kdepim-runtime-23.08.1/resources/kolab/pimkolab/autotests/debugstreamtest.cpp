/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "debugstreamtest.h"

#include "kolabformat/errorhandler.h"

#include <QTest>

void DebugStreamTest::testDebugstream()
{
    Error() << "test1";
    Error() << "test2"
            << "bla" << 3 << QMap<QString, int>();
    QCOMPARE(Kolab::ErrorHandler::instance().getErrors().size(), 2);
    QVERIFY(Kolab::ErrorHandler::instance().getErrors().first().message.contains(QLatin1String("test1")));
    QCOMPARE(Kolab::ErrorHandler::instance().getErrors().first().severity, Kolab::ErrorHandler::Error);
    QVERIFY(Kolab::ErrorHandler::instance().getErrors().last().message.contains(QLatin1String("bla")));
}

void DebugStreamTest::testDebugNotLogged()
{
    Kolab::ErrorHandler::instance().clear();
    Debug() << "test1";
    QCOMPARE(Kolab::ErrorHandler::instance().getErrors().size(), 0);
}

void DebugStreamTest::testHasError()
{
    Debug() << "test1";
    QCOMPARE(Kolab::ErrorHandler::errorOccured(), false);
    Warning() << "test1";
    QCOMPARE(Kolab::ErrorHandler::errorOccured(), false);
    Error() << "test1";
    QCOMPARE(Kolab::ErrorHandler::errorOccured(), true);
    Kolab::ErrorHandler::clearErrors();
    QCOMPARE(Kolab::ErrorHandler::errorOccured(), false);
}

QTEST_MAIN(DebugStreamTest)
