/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "timezonetest.h"
#include "testutils.h"
#include <conversion/commonconversion.h>
#include <conversion/timezoneconverter.h>
#include <kolabformat/errorhandler.h>
#include <kolabformat/kolabobject.h>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <QTest>

void TimezoneTest::initTestCase()
{
}

void TimezoneTest::testFromName()
{
    TimezoneConverter converter;
    const QString timezone = converter.normalizeTimezone(QStringLiteral("(GMT+01.00) Sarajevo/Warsaw/Zagreb"));
    QCOMPARE(timezone, QLatin1String("Europe/Sarajevo"));
}

void TimezoneTest::testFromHardcodedList_data()
{
    QTest::addColumn<QString>("timezone");

    QTest::newRow("1") << QStringLiteral("(GMT+01:00) West Central Africa");
    QTest::newRow("2") << QStringLiteral("(GMT-04:00) Atlantic Time (Canada)");
    QTest::newRow("3") << QStringLiteral("(GMT-06:00) Saskatchewan");
    QTest::newRow("4") << QStringLiteral("(GMT-01:00) Cape Verde Islands");
    QTest::newRow("5") << QStringLiteral("(GMT-06:00) Central America");
    QTest::newRow("6") << QStringLiteral("(GMT-06:00) Central Time (US and Canada)");
    //     QTest::newRow( "7" ) << QString::fromLatin1("(GMT-12:00) International Date Line West"); //Not mappable
    QTest::newRow("8") << QStringLiteral("(GMT-05:00) Eastern Time (US and Canada)");
    //     QTest::newRow( "9" ) << QString::fromLatin1("(GMT-02:00) Mid-Atlantic"); //Not mappable
    QTest::newRow("10") << QStringLiteral("(GMT-07:00) Mountain Time (US and Canada)");
    QTest::newRow("11") << QStringLiteral("(GMT-03:30) Newfoundland and Labrador");
    QTest::newRow("12") << QStringLiteral("(GMT-08:00) Pacific Time (US and Canada); Tijuana");
    QTest::newRow("13") << QStringLiteral("(GMT-11:00) Midway Island, Samoa");
    QTest::newRow("14") << QStringLiteral("W. Europe Standard Time");
    QTest::newRow("15") << QStringLiteral("(GMT+1.00) Sarajevo/Warsaw/Zagreb");
    // Lotus notes uses it's own set of specifiers
    //     QTest::newRow( "Lotus Notes" ) << QString::fromLatin1("W. Europe");
    //     QTest::newRow( "Google UTC offset" ) << QString::fromLatin1("2013-10-23T04:00:00+02:00");
}

void TimezoneTest::testFromHardcodedList()
{
    TimezoneConverter converter;
    QFETCH(QString, timezone);
    const QString tz = converter.normalizeTimezone(timezone);
    qDebug() << tz;
    QVERIFY(!tz.isEmpty());
    QVERIFY(tz != timezone);
}

void TimezoneTest::testKolabObjectWriter()
{
    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(QDate(2012, 11, 11), QTime(1, 1), QTimeZone(QTimeZone::windowsIdToDefaultIanaId("(GMT+01:00) West Central Africa"))));
    KMime::Message::Ptr msg = Kolab::KolabObjectWriter::writeEvent(event);
    Kolab::KolabObjectReader reader(msg);
    KCalendarCore::Event::Ptr result = reader.getEvent();
    qDebug() << result->dtStart().timeZone().id();
    QCOMPARE(result->dtStart().timeZone().id(), QTimeZone("Africa/Lagos").id());
}

// void TimezoneTest::testKolabObjectReader()
// {
//     const Kolab::Version version = Kolab::KolabV3;
//     const Kolab::ObjectType type = Kolab::EventObject;
//     QString icalFileName = TESTFILEDIR+QString::fromLatin1("timezone/windowsTimezone.ics"); //To compare
//     QString mimeFileName = TESTFILEDIR+QString::fromLatin1("timezone/windowsTimezoneV3.mime"); //For parsing
//
//     //Parse mime message
//     bool ok = false;
//     const KMime::Message::Ptr &msg = readMimeFile( mimeFileName, ok );
//     QVERIFY(ok);
//     Kolab::KolabObjectReader reader;
//     Kolab::ObjectType t = reader.parseMimeMessage(msg);
//     QCOMPARE(t, type);
//     QCOMPARE(reader.getVersion(), version);
//     QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
//
//     KCalendarCore::Incidence::Ptr convertedIncidence = reader.getIncidence();
//     qDebug() << "read incidence";
//
//     //Parse ICalFile for comparison
//     QFile icalFile( icalFileName );
//     QVERIFY( icalFile.open( QFile::ReadOnly ) );
//     KCalendarCore::ICalFormat format;
//     KCalendarCore::Incidence::Ptr realIncidence( format.fromString( QString::fromUtf8( icalFile.readAll() ) ) );
//
//     // fix up the converted incidence for comparison
//     normalizeIncidence(convertedIncidence);
//     normalizeIncidence(realIncidence);
//
//     // recurrence objects are created on demand, but KCalendarCore::Incidence::operator==() doesn't take that into account
//     // so make sure both incidences have one
//     realIncidence->recurrence();
//     convertedIncidence->recurrence();
//
//     realIncidence->setLastModified(convertedIncidence->lastModified());
//
//     //The following test is just for debugging and not really relevant
//     if ( *(realIncidence.data()) != *(convertedIncidence.data()) ) {
//         showDiff(format.toString( realIncidence ), format.toString( convertedIncidence ));
//     }
//     QVERIFY( *(realIncidence.data()) ==  *(convertedIncidence.data()) );
// }

void TimezoneTest::testFindLegacyTimezone()
{
    const QString normalized = TimezoneConverter::normalizeTimezone(QStringLiteral("US/Pacific"));
    qDebug() << normalized;
    QEXPECT_FAIL("", "Currently broken", Continue);
    QVERIFY(!normalized.isEmpty());
}

void TimezoneTest::testIgnoreInvalidTimezone()
{
    const QString normalized = TimezoneConverter::normalizeTimezone(QStringLiteral("FOOOOBAR"));
    qDebug() << normalized;
    QVERIFY(normalized.isEmpty());
}

void TimezoneTest::testUTCOffset()
{
    const Kolab::cDateTime expected(2013, 10, 23, 2, 0, 0, true);
    const QDateTime input(QDateTime::fromString(QStringLiteral("2013-10-23T04:00:00+02:00"), Qt::ISODate));
    const Kolab::cDateTime result = Kolab::Conversion::fromDate(input, false);
    QVERIFY(!Kolab::ErrorHandler::instance().errorOccured());
    QCOMPARE(result, expected);
}

void TimezoneTest::localTimezone()
{
    {
        const Kolab::cDateTime result = Kolab::Conversion::fromDate(QDateTime(QDate(2013, 10, 10), QTime(2, 0, 0), Qt::LocalTime), false);
        QVERIFY(!result.timezone().empty());
        QVERIFY(!Kolab::ErrorHandler::instance().errorOccured());
    }
    {
        const Kolab::cDateTime result = Kolab::Conversion::fromDate(QDateTime(QDate(2013, 10, 10), QTime(2, 0, 0)), false);
        QVERIFY(!Kolab::ErrorHandler::instance().errorOccured());
    }
    {
        const Kolab::cDateTime result = Kolab::Conversion::fromDate(QDateTime(QDate(2013, 10, 10), QTime(2, 0, 0), QTimeZone()), false);
        QVERIFY(result.timezone().empty());
        QVERIFY(!Kolab::ErrorHandler::instance().errorOccured());
    }
    {
        QDateTime dt(QDate(2013, 10, 10), QTime(2, 0, 0), QTimeZone("/etc/localzone"));
        const Kolab::cDateTime result = Kolab::Conversion::fromDate(dt, false);
        qDebug() << result.timezone();
        QVERIFY(result.timezone().empty());
        QVERIFY(!Kolab::ErrorHandler::instance().errorOccured());
    }
}

QTEST_MAIN(TimezoneTest)
