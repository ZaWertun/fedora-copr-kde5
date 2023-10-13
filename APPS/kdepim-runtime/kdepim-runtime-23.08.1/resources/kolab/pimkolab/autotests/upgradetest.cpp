/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "upgradetest.h"

#include <QTest>
#include <kolabcontainers.h>
#include <kolabformat.h>
#include <kolabformat/errorhandler.h>

#include "kolabformat/kolabobject.h"
#include "testutils.h"
#include <KCalendarCore/ICalFormat>
#include <KContacts/VCardConverter>
#include <conversion/commonconversion.h>

void UpgradeTest::testIncidence_data()
{
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("forceType");

    //     QTest::newRow( "v2contactSimple" ) << Kolab::KolabV2 << Kolab::ContactObject << TESTFILEDIR+QString::fromLatin1("v2/contacts/simple.vcf") <<
    //     TESTFILEDIR+QString::fromLatin1("v2/contacts/simple.vcf.mime");
    QTest::newRow("v2eventSimple") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/simple.ics.mime") << false;
    QTest::newRow("v2eventComplex") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/complex.ics.mime") << false;
    QTest::newRow("v2eventAllday") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/allday.ics.mime") << false;
    QTest::newRow("v2eventAttachment") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/attachment.ics.mime") << false;
    QTest::newRow("v2eventHorde") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/horde.ics.mime") << false;

    // Still broken, although it seems that rather the event comparison is implemented wrong (not using equals)
    QTest::newRow("v2taskSimple") << Kolab::TodoObject << TESTFILEDIR + QLatin1String("v2/task/simple.ics.mime") << false;
    QTest::newRow("v2taskComplex") << Kolab::TodoObject << TESTFILEDIR + QLatin1String("v2/task/complex.ics.mime") << false;
    QTest::newRow("v2taskPrio1") << Kolab::TodoObject << TESTFILEDIR + QLatin1String("v2/task/prioritytest1.ics.mime") << false;
    QTest::newRow("v2taskPrio2") << Kolab::TodoObject << TESTFILEDIR + QLatin1String("v2/task/prioritytest2.ics.mime") << false;

    QTest::newRow("v2journalSimple") << Kolab::JournalObject << TESTFILEDIR + QLatin1String("v2/journal/simple.ics.mime") << false;
    QTest::newRow("v2journalComplex") << Kolab::JournalObject << TESTFILEDIR + QLatin1String("v2/journal/complex.ics.mime") << false;

    QTest::newRow("v2eventSimple_missingTypeHeader") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/simple_missingTypeHeader.ics.mime") << true;
    QTest::newRow("v2eventis8859-1") << Kolab::EventObject << TESTFILEDIR + QLatin1String("v2/event/iso8859-1.mime") << false;
}

void UpgradeTest::testIncidence()
{
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, filename); // To compare
    QFETCH(bool, forceType);

    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(filename, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    if (forceType) {
        reader.setObjectType(type);
    }
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), Kolab::KolabV2);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KCalendarCore::Incidence::Ptr v2result = reader.getIncidence();
    QVERIFY(!v2result.isNull());

    // write KCalendarCore V3
    KMime::Message::Ptr v3message = Kolab::KolabObjectWriter::writeIncidence(v2result, Kolab::KolabV3);
    QVERIFY(Kolab::error() == Kolab::NoError);
    //     qDebug() << v3message->encodedContent();
    // load KCalendarCore V3
    Kolab::KolabObjectReader reader2;
    QCOMPARE(reader2.parseMimeMessage(v3message), type);
    KCalendarCore::Incidence::Ptr v3result = reader2.getIncidence();
    QVERIFY(!v3result.isNull());
    normalizeIncidence(v2result);
    // We have to override the last modified time with a specific value, so we normalize the input to the same
    normalizeIncidence(v3result);
    qDebug() << "--------------------------------------------------------";
    KCalendarCore::ICalFormat format;
    const QString v2 = format.toString(v2result);
    const QString v3 = format.toString(v3result);
    if (*v2result != *v3result) {
        showDiff(v2, v3);
        //         qDebug() << "V2: " << v2;
        //         qDebug() << "V3: " << v3;
    }
    QVERIFY(!v2.isEmpty());
    QVERIFY(!v3.isEmpty());
    QCOMPARE(v2, v3);
    qDebug() << "--------------------------------------------------------";

    //     if (v2result->type() == KCalendarCore::IncidenceBase::TypeTodo) {
    //         KCalendarCore::Todo::Ptr t1 = v2result.dynamicCast<KCalendarCore::Todo>();
    //         KCalendarCore::Todo::Ptr t2 = v3result.dynamicCast<KCalendarCore::Todo>();
    //         QCOMPARE(t1->percentComplete(), t2->percentComplete());
    //         QCOMPARE(t1->priority(), t2->priority());
    //         QCOMPARE(t1->uid(), t2->uid());
    //         QCOMPARE(t1->dtStart(), t2->dtStart());
    //         QCOMPARE(t1->dtDue(), t2->dtDue());
    //         QCOMPARE(*t1->organizer().data(), *t2->organizer().data());
    //         QCOMPARE(t1->uid(), t2->uid());
    //         QCOMPARE(t1->allDay(), t2->allDay());
    //         QCOMPARE(t1->duration(), t2->duration());
    //         QCOMPARE(t1->hasDuration(), t2->hasDuration());
    //         qDebug() << "V2: " << v2;
    //     }
    // Test comparator
    QCOMPARE(*v2result, *v3result);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

void UpgradeTest::testContact_data()
{
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("filename");

    QTest::newRow("v2contactSimple") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/simple.vcf.mime");
    QTest::newRow("v2contactComplex") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/complex.vcf.mime");
    QTest::newRow("v2contactAddress") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/address.vcf.mime");
    QTest::newRow("v2contactBug238996") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/bug238996.vcf.mime");
    QTest::newRow("v2contactDisplayname") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/displayname.vcf.mime");
    QTest::newRow("v2contactEmails") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/emails.vcf.mime");
    QTest::newRow("v2contactPhonenumber") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/phonenumbers.vcf.mime");
    QTest::newRow("v2contactPicture") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/picture.vcf.mime");
    QTest::newRow("v2iso8859-1") << Kolab::ContactObject << TESTFILEDIR + QLatin1String("v2/contacts/iso8859-1.mime");
}

void UpgradeTest::testContact()
{
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, filename);

    // Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(filename, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), Kolab::KolabV2);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KContacts::Addressee v2Addressee = reader.getContact();
    QVERIFY(!v2Addressee.isEmpty());

    // Write
    const KMime::Message::Ptr &v3message = Kolab::KolabObjectWriter::writeContact(v2Addressee, Kolab::KolabV3);

    // load V3
    Kolab::KolabObjectReader reader2;
    QCOMPARE(reader2.parseMimeMessage(v3message), type);
    KContacts::Addressee v3result = reader2.getContact();
    QVERIFY(!v3result.isEmpty());

    normalizeContact(v2Addressee);
    QVERIFY(normalizePhoneNumbers(v3result, v2Addressee)); // phone number ids are random
    QVERIFY(normalizeAddresses(v3result, v2Addressee)); // same here

    KContacts::VCardConverter converter;
    if (v2Addressee != v3result) {
        showDiff(QString::fromUtf8(converter.createVCard(v2Addressee)), QString::fromUtf8(converter.createVCard(v3result)));
        QVERIFY(false);
    }
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

QTEST_MAIN(UpgradeTest)
