/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "formattest.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QTest>
#include <kolabcontainers.h>
#include <kolabformat.h>

#include <Akonadi/NoteUtils>

#include <KCalendarCore/ICalFormat>
#include <KContacts/ContactGroupTool>
#include <KContacts/VCardConverter>

#include "kolabformat/errorhandler.h"
#include "kolabformat/kolabdefinitions.h"
#include "kolabformat/kolabobject.h"
#include "pimkolab_debug.h"
#include "testutils.h"
static bool compareMimeMessage(const KMime::Message::Ptr &msg, const KMime::Message::Ptr &expectedMsg)
{
    // headers
    KCOMPARE(msg->subject()->asUnicodeString(), expectedMsg->subject()->asUnicodeString());
    if (msg->from()->isEmpty() || expectedMsg->from()->isEmpty()) {
        KCOMPARE(msg->from()->asUnicodeString(), expectedMsg->from()->asUnicodeString());
    } else {
        KCOMPARE(msg->from()->mailboxes().first().address(),
                 expectedMsg->from()->mailboxes().first().address()); // matching address is enough, we don't need a display name
    }
    KCOMPARE(msg->contentType()->mimeType(), expectedMsg->contentType()->mimeType());
    KCOMPARE(msg->headerByType(X_KOLAB_TYPE_HEADER)->as7BitString(), expectedMsg->headerByType(X_KOLAB_TYPE_HEADER)->as7BitString());
    // date contains conversion time...
    //   KCOMPARE( msg->date()->asUnicodeString(), expectedMsg->date()->asUnicodeString() );

    // body parts
    KCOMPARE(msg->contents().size(), expectedMsg->contents().size());
    for (int i = 0; i < msg->contents().size(); ++i) {
        KMime::Content *part = msg->contents().at(i);
        KMime::Content *expectedPart = expectedMsg->contents().at(i);

        // part headers
        KCOMPARE(part->contentType()->mimeType(), expectedPart->contentType()->mimeType());
        KCOMPARE(part->contentDisposition()->filename(), expectedPart->contentDisposition()->filename());

        KCOMPARE(part->decodedContent().isEmpty(), false);

        QString content(QString::fromUtf8(part->decodedContent()));
        normalizeMimemessage(content);
        QString expected(QString::fromUtf8(expectedPart->decodedContent()));
        normalizeMimemessage(expected);
        //         showDiff(expected, content);

        // part content
        KCOMPARE(content.simplified(), expected.simplified());
    }
    return true;
}

void FormatTest::initTestCase()
{
}

void FormatTest::testIncidence_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("icalFileName");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v2eventSimple") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/simple.ics") << getPath("v2/event/simple.ics.mime");
    QTest::newRow("v2eventComplex") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/complex.ics") << getPath("v2/event/complex.ics.mime");
    QTest::newRow("v2eventAttachment") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/attachment.ics") << getPath("v2/event/attachment.ics.mime");
    QTest::newRow("v2eventAllday") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/allday.ics") << getPath("v2/event/allday.ics.mime");
    QTest::newRow("v2eventUtf8Attachment") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/attachmentUtf8.ics")
                                           << getPath("v2/event/attachmentUtf8.ics.mime");
    // The following test just fails because we have a nicer mime message output than horde
    //     QTest::newRow( "v2eventHorde" ) << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/horde.ics") << getPath("v2/event/horde.ics.mime");
    QTest::newRow("v2todoSimple") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/simple.ics") << getPath("v2/task/simple.ics.mime");
    QTest::newRow("v2todoComplex") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/complex.ics") << getPath("v2/task/complex.ics.mime");
    QTest::newRow("v2todoPrio1") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/prioritytest1.ics") << getPath("v2/task/prioritytest1.ics.mime");
    QTest::newRow("v2todoPrio2") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/prioritytest2.ics") << getPath("v2/task/prioritytest2.ics.mime");
    QTest::newRow("v2journalSimple") << Kolab::KolabV2 << Kolab::JournalObject << getPath("v2/journal/simple.ics") << getPath("v2/journal/simple.ics.mime");
    QTest::newRow("v2journalComplex") << Kolab::KolabV2 << Kolab::JournalObject << getPath("v2/journal/complex.ics") << getPath("v2/journal/complex.ics.mime");

    QTest::newRow("v3eventSimple") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/simple.ics") << getPath("v3/event/simple.ics.mime");
    QTest::newRow("v3eventComplex") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/complex.ics") << getPath("v3/event/complex.ics.mime");
    QTest::newRow("v3todoSimple") << Kolab::KolabV3 << Kolab::TodoObject << getPath("v3/task/simple.ics") << getPath("v3/task/simple.ics.mime");
    QTest::newRow("v3todoComplex") << Kolab::KolabV3 << Kolab::TodoObject << getPath("v3/task/complex.ics") << getPath("v3/task/complex.ics.mime");
    QTest::newRow("v3journalSimple") << Kolab::KolabV3 << Kolab::JournalObject << getPath("v3/journal/simple.ics") << getPath("v3/journal/simple.ics.mime");
    QTest::newRow("v3journalComplex") << Kolab::KolabV3 << Kolab::JournalObject << getPath("v3/journal/complex.ics") << getPath("v3/journal/complex.ics.mime");
    QTest::newRow("v3utf8quotedPrintable") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf8.ics")
                                           << getPath("v3/event/utf8quotedPrintable.ics.mime");
    QTest::newRow("v3utf8base64") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf8.ics") << getPath("v3/event/utf8base64.ics.mime");
    QTest::newRow("v3utf88bit") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf8.ics") << getPath("v3/event/utf88bit.ics.mime");
}

void FormatTest::testIncidence()
{
    QFETCH(Kolab::Version, version);
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, icalFileName); // To compare
    QFETCH(QString, mimeFileName); // For parsing

    // Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(mimeFileName, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), version);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KCalendarCore::Incidence::Ptr convertedIncidence = reader.getIncidence();

    // Parse ICalFile for comparison
    QFile icalFile(icalFileName);
    QVERIFY(icalFile.open(QFile::ReadOnly));
    KCalendarCore::ICalFormat format;
    KCalendarCore::Incidence::Ptr realIncidence(format.fromString(QString::fromUtf8(icalFile.readAll())));

    // fix up the converted incidence for comparison
    normalizeIncidence(convertedIncidence);
    normalizeIncidence(realIncidence);

    // recurrence objects are created on demand, but KCalendarCore::Incidence::operator==() doesn't take that into account
    // so make sure both incidences have one
    realIncidence->recurrence();
    convertedIncidence->recurrence();

    realIncidence->setLastModified(convertedIncidence->lastModified());

    // The following test is just for debugging and not really relevant
    if (*(realIncidence.data()) != *(convertedIncidence.data())) {
        showDiff(format.toString(realIncidence), format.toString(convertedIncidence));
    }
    QVERIFY(*(realIncidence.data()) == *(convertedIncidence.data()));

    // Write
    Kolab::overrideTimestamp(Kolab::cDateTime(2012, 5, 5, 5, 5, 5, true));
    KMime::Message::Ptr convertedMime = Kolab::KolabObjectWriter::writeIncidence(realIncidence, version);

    if (!compareMimeMessage(convertedMime, msg)) {
        showDiff(QString::fromUtf8(msg->encodedContent()), QString::fromUtf8(convertedMime->encodedContent()));
        QVERIFY(false);
    }
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

enum TestMode {
    ReadOnly,
    ReadWrite,
};
Q_DECLARE_METATYPE(TestMode)

void FormatTest::testContact_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("vcardFileName");
    QTest::addColumn<QString>("mimeFileName");
    QTest::addColumn<TestMode>("mode");

    QTest::newRow("v2contactSimple") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/simple.vcf") << getPath("v2/contacts/simple.vcf.mime")
                                     << ReadWrite;
    // FIXME Reference files needs to be adjusted due to fix in how pictures are stored
    //     QTest::newRow( "v2contactComplex" ) << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/complex.vcf") <<
    //     getPath("v2/contacts/complex.vcf.mime") << ReadWrite;
    QTest::newRow("v2contactAddress") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/address.vcf") << getPath("v2/contacts/address.vcf.mime")
                                      << ReadWrite;
    QTest::newRow("v2contactBug238996") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/bug238996.vcf")
                                        << getPath("v2/contacts/bug238996.vcf.mime") << ReadWrite;
    QTest::newRow("v2contactDisplayname") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/displayname.vcf")
                                          << getPath("v2/contacts/displayname.vcf.mime") << ReadWrite;
    QTest::newRow("v2contactEmails") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/emails.vcf") << getPath("v2/contacts/emails.vcf.mime")
                                     << ReadWrite;
    QTest::newRow("v2contactPhonenumbers") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/phonenumbers.vcf")
                                           << getPath("v2/contacts/phonenumbers.vcf.mime") << ReadWrite;
    // FIXME Reference files needs to be adjusted due to fix in how pictures are stored
    //     QTest::newRow( "v2contactPicture" ) << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/picture.vcf") <<
    //     getPath("v2/contacts/picture.vcf.mime") << ReadWrite;
    // FIXME the following test fails because the vcard implementation always writes jpeg (which is lossy). The reference vcf file is therefore probably also
    // not really useful
    //     QTest::newRow( "v2pictureJPGHorde" ) << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/pictureJPGHorde.vcf") <<
    //     getPath("v2/contacts/pictureJPGHorde.vcf.mime");

    QTest::newRow("v2contactRelated") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/related.vcf") << getPath("v2/contacts/related.vcf.mime")
                                      << ReadWrite;

    QTest::newRow("v3contactSimple") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/contacts/simple.vcf") << getPath("v3/contacts/simple.vcf.mime")
                                     << ReadWrite;
    QTest::newRow("v3contactComplex") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/contacts/complex.vcf") << getPath("v3/contacts/complex.vcf.mime")
                                      << ReadWrite;
    QTest::newRow("v3contactPng") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/readonly/png.vcf") << getPath("v3/readonly/png.vcf.mime")
                                  << ReadOnly;
}

bool comparePictureToReference(const QImage &picture)
{
    QImage img(getPath("picture.jpg"));
    QByteArray pic;
    QBuffer buffer(&pic);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "JPEG");
    buffer.close();

    QByteArray pic2;
    QBuffer buffer2(&pic2);
    buffer2.open(QIODevice::WriteOnly);
    picture.save(&buffer2, "JPEG");
    buffer2.close();

    if (pic.toBase64() != pic2.toBase64()) {
        qDebug() << pic.toBase64();
        qDebug() << pic2.toBase64();
        return false;
    }
    return true;
}

void FormatTest::testContact()
{
    QFETCH(Kolab::Version, version);
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, vcardFileName); // To compare
    QFETCH(QString, mimeFileName); // For parsing
    QFETCH(TestMode, mode); // For parsing

    // Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(mimeFileName, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), version);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KContacts::Addressee convertedAddressee = reader.getContact();
    QVERIFY(!convertedAddressee.isEmpty());

    // Parse vcard
    QFile vcardFile(vcardFileName);
    QVERIFY(vcardFile.open(QFile::ReadOnly));
    KContacts::VCardConverter converter;
    const QByteArray &c = vcardFile.readAll();
    KContacts::Addressee realAddressee = converter.parseVCard(c);

    // fix up the converted addressee for comparison
    convertedAddressee.setName(realAddressee.name()); // name() apparently is something strange
    if (version == Kolab::KolabV2) { // No creation date in xcal
        QVERIFY(!convertedAddressee.custom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate")).isEmpty());
        convertedAddressee.removeCustom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate")); // that's conversion time !?
    } else {
        normalizeContact(convertedAddressee);
        normalizeContact(realAddressee);
    }
    QVERIFY(normalizePhoneNumbers(convertedAddressee, realAddressee)); // phone number ids are random
    QVERIFY(normalizeAddresses(convertedAddressee, realAddressee)); // same here
    QCOMPARE(realAddressee.photo().type(), convertedAddressee.photo().type());
    if (realAddressee != convertedAddressee) {
        showDiff(normalizeVCardMessage(QString::fromUtf8(converter.createVCard(realAddressee))),
                 normalizeVCardMessage(QString::fromUtf8(converter.createVCard(convertedAddressee))));
    }
    QEXPECT_FAIL("v2contactBug238996", "Currently fails due to missing type=pref attribute of preferred email address. Requires fix in KContacts.", Continue);
    QEXPECT_FAIL("v2contactEmails", "Currently fails due to missing type=pref attribute of preferred email address. Requires fix in KContacts.", Continue);
    QEXPECT_FAIL("v3contactComplex", "Currently fails due to missing type=pref attribute of preferred email address. Requires fix in KContacts.", Continue);
    QCOMPARE(realAddressee, convertedAddressee);

    // Write
    if (mode == ReadWrite) {
        Kolab::overrideTimestamp(Kolab::cDateTime(2012, 5, 5, 5, 5, 5, true));
        const KMime::Message::Ptr &convertedMime = Kolab::KolabObjectWriter::writeContact(realAddressee, version);

        if (!compareMimeMessage(convertedMime, msg)) {
            QString expected = QString::fromUtf8(msg->encodedContent());
            normalizeMimemessage(expected);
            QString converted = QString::fromUtf8(convertedMime->encodedContent());
            normalizeMimemessage(converted);
            showDiff(expected, converted);
            QEXPECT_FAIL("v2contactSimple", "The kolab v3 containers don't support postbox, and we therefore loose it in the transformations.", Continue);
            QEXPECT_FAIL("v2contactAddress", "The kolab v3 containers don't support postbox, and we therefore loose it in the transformations.", Continue);
            QEXPECT_FAIL("v2contactBug238996", "The kolab v3 containers don't support postbox, and we therefore loose it in the transformations.", Continue);
            QVERIFY(false);
        }
    }
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

void FormatTest::testDistlist_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("vcardFileName");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v3distlistSimple") << Kolab::KolabV3 << Kolab::DistlistObject << getPath("v3/contacts/distlist.vcf")
                                      << getPath("v3/contacts/distlist.vcf.mime");
}

void FormatTest::testDistlist()
{
    QFETCH(Kolab::Version, version);
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, vcardFileName); // To compare
    QFETCH(QString, mimeFileName); // For parsing

    // Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(mimeFileName, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), version);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KContacts::ContactGroup convertedAddressee = reader.getDistlist();

    // Parse vcard
    QFile vcardFile(vcardFileName);
    QVERIFY(vcardFile.open(QFile::ReadOnly));
    KContacts::VCardConverter converter;
    QByteArray c = vcardFile.readAll();
    QBuffer data(&c);
    data.open(QIODevice::ReadOnly);

    KContacts::ContactGroup realAddressee;
    KContacts::ContactGroupTool::convertFromXml(&data, realAddressee);

    {
        QBuffer expected;
        expected.open(QIODevice::WriteOnly);
        KContacts::ContactGroupTool::convertToXml(realAddressee, &expected);

        QBuffer converted;
        converted.open(QIODevice::WriteOnly);
        KContacts::ContactGroupTool::convertToXml(convertedAddressee, &converted);

        showDiff(QString::fromUtf8(expected.buffer()), QString::fromUtf8(converted.buffer()));
    }
    QCOMPARE(realAddressee, convertedAddressee);

    // Write
    const KMime::Message::Ptr &convertedMime = Kolab::KolabObjectWriter::writeDistlist(realAddressee, version);

    if (!compareMimeMessage(convertedMime, msg)) {
        QString expected = QString::fromUtf8(msg->encodedContent());
        normalizeMimemessage(expected);
        QString converted = QString::fromUtf8(convertedMime->encodedContent());
        normalizeMimemessage(converted);
        showDiff(expected, converted);
        QVERIFY(false);
    }
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

void FormatTest::testNote_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("noteFileName");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v3noteSimple") << Kolab::KolabV3 << Kolab::NoteObject << getPath("v3/note/note.mime") << getPath("v3/note/note.mime.mime");
}

void FormatTest::testNote()
{
    QFETCH(Kolab::Version, version);
    QFETCH(Kolab::ObjectType, type);
    QFETCH(QString, noteFileName); // To compare
    QFETCH(QString, mimeFileName); // For parsing

    // Parse mime message
    bool ok = false;
    const KMime::Message::Ptr &msg = readMimeFile(mimeFileName, ok);
    QVERIFY(ok);
    Kolab::KolabObjectReader reader;
    Kolab::ObjectType t = reader.parseMimeMessage(msg);
    QCOMPARE(t, type);
    QCOMPARE(reader.getVersion(), version);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);

    KMime::Message::Ptr convertedNote = reader.getNote();
    QVERIFY(convertedNote.data());

    // Parse note
    const KMime::Message::Ptr &realNote = readMimeFile(noteFileName, ok);
    QVERIFY(ok);
    QVERIFY(realNote.data());

    QString expected = QString::fromUtf8(realNote->encodedContent());
    normalizeMimemessage(expected);
    QString converted = QString::fromUtf8(convertedNote->encodedContent());
    normalizeMimemessage(converted);
    QEXPECT_FAIL("", "Header sorting is off", Continue);
    QCOMPARE(expected, converted);
    // showDiff(expected, converted);

    // Write
    const KMime::Message::Ptr &convertedMime = Kolab::KolabObjectWriter::writeNote(realNote, version);
    QVERIFY(convertedMime.data());
    QVERIFY(msg.data());

    QString expected2 = QString::fromUtf8(msg->encodedContent());
    normalizeMimemessage(expected2);
    QString converted2 = QString::fromUtf8(convertedMime->encodedContent());
    normalizeMimemessage(converted2);
    QEXPECT_FAIL("", "Header sorting is off", Continue);
    QCOMPARE(expected2, converted2);
    // showDiff(expected2, converted2);

    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

// This function exists only to generate the reference files, it's not a real test.
void FormatTest::generateMimefile()
{
    //     QFile icalFile( getPath("v3/journal/complex.ics") );
    //     QVERIFY( icalFile.open( QFile::ReadOnly ) );
    //     KCalendarCore::ICalFormat format;
    //     const KCalendarCore::Incidence::Ptr realIncidence( format.fromString( QString::fromUtf8( icalFile.readAll() ) ) );
    //
    //     QString result;
    //     QTextStream s(&result);
    //     Kolab::overrideTimestamp(Kolab::cDateTime(2012, 5, 5, 5,5,5, true));
    //     Kolab::KolabObjectWriter::writeIncidence(realIncidence, Kolab::KolabV3)->toStream(s);

    //     QFile vcardFile( getPath("v3/contacts/complex.vcf") );
    //     QVERIFY( vcardFile.open( QFile::ReadOnly ) );
    //     KContacts::VCardConverter converter;
    //     const KContacts::Addressee realAddressee = converter.parseVCard( vcardFile.readAll() );
    //
    //     qDebug() << realAddressee.photo().data();
    //
    //     QString result;
    //     QTextStream s(&result);
    //     Kolab::overrideTimestamp(Kolab::cDateTime(2012, 5, 5, 5,5,5, true));
    //     Kolab::KolabObjectWriter::writeContact(realAddressee, Kolab::KolabV3)->toStream(s);

    //     qDebug() << result;
}

void FormatTest::generateVCard()
{
    //     bool ok = false;
    //     const KMime::Message::Ptr &msg = readMimeFile( QString::fromLatin1("../")+getPath("v2/contacts/pictureJPGHorde.vcf.mime"), ok );
    //     qDebug() << msg->encodedContent();
    //     Kolab::KolabObjectReader reader;
    //     Kolab::ObjectType t = reader.parseMimeMessage(msg);
    //
    //     KContacts::Addressee convertedAddressee = reader.getContact();
    //     KContacts::VCardConverter converter;
    //     qDebug() << converter.createVCard(convertedAddressee);

    //     bool ok = false;
    //     const KMime::Message::Ptr &msg = readMimeFile( getPath("v3/contacts/distlist.vcf.mime"), ok );
    //     qDebug() << msg->encodedContent();
    //     Kolab::KolabObjectReader reader;
    //     Kolab::ObjectType t = reader.parseMimeMessage(msg);
    //
    //     KContacts::ContactGroup convertedAddressee = reader.getDistlist();
    //     QBuffer buf;
    //     buf.open(QIODevice::WriteOnly);
    //     KContacts::ContactGroupTool::convertToXml(convertedAddressee, &buf);
    //     qDebug() << buf.buffer();
}

// Pseudo test to show that JPG is always lossy, even with quality set to 100
void FormatTest::proveJPGisLossy()
{
    //     QImage img(getPath("picture.jpg"));
    //     QByteArray pic;
    //     QBuffer buffer(&pic);
    //     buffer.open(QIODevice::WriteOnly);
    //     img.save(&buffer, "JPEG");
    //     buffer.close();
    //     qDebug() << pic.toBase64();
    //
    //     QImage img2;
    //     QByteArray pic2;
    //     QBuffer buffer2(&pic2);
    //     img2.loadFromData(pic);
    //     img2.save(&buffer2, "JPEG");
    //     buffer2.close();
    //     qDebug() << pic2.toBase64();
}

QTEST_MAIN(FormatTest)
