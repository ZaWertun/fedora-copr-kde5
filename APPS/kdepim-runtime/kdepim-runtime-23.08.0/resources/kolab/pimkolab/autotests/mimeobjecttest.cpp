/*
 * SPDX-FileCopyrightText: 2012 Sofia Balicka <balicka@kolabsys.com>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mimeobjecttest.h"
#include "conversion/commonconversion.h"
#include "kolabformat/mimeobject.h"
#include "testutils.h"
#include <QString>
#include <QTest>
#include <fstream>
#include <sstream>

static std::string readFile(const QString &path)
{
    std::ifstream t(path.toStdString().c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

static QString normalizeMimemessage(const std::string &path)
{
    QString qString = Kolab::Conversion::fromStdString(path);
    normalizeMimemessage(qString);
    return qString;
}

template<class T>
void testFunction(const QString &filename,
                  Kolab::Version version,
                  Kolab::ObjectType type,
                  T (Kolab::MIMEObject::*readFunction)(const std::string &),
                  std::string (Kolab::MIMEObject::*writeFunction)(const T &, Kolab::Version, const std::string &))
{
    const std::string input = readFile(filename);
    Kolab::MIMEObject mimeobject;
    const T object = (mimeobject.*readFunction)(input);
    QCOMPARE(mimeobject.getType(), type);
    const std::string message = (mimeobject.*writeFunction)(object, version, std::string());
    DIFFCOMPARE(normalizeMimemessage(message), normalizeMimemessage(input));
}

template<class T>
void testFunction(T (Kolab::MIMEObject::*readFunction)(const std::string &),
                  std::string (Kolab::MIMEObject::*writeFunction)(const T &, Kolab::Version, const std::string &))
{
    testFunction<T>(TESTVALUE(QString, mimeFileName), TESTVALUE(Kolab::Version, version), TESTVALUE(Kolab::ObjectType, type), readFunction, writeFunction);
}

void MIMEObjectTest::testEvent_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v2eventSimple") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/simple.ics.mime");
    QTest::newRow("v2eventComplex") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/complex.ics.mime");
    QTest::newRow("v2eventAttachment") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/attachment.ics.mime");
    QTest::newRow("v2eventAllday") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/allday.ics.mime");
    QTest::newRow("v2eventUtf8Attachment") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/attachmentUtf8.ics.mime");
    QTest::newRow("v3eventSimple") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/simple.ics.mime");
    QTest::newRow("v3eventComplex") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/complex.ics.mime");
    QTest::newRow("v3utf8quotedPrintable") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf8quotedPrintable.ics.mime");
    // TODO move to read-only
    // QTest::newRow("v2eventHorde") << Kolab::KolabV2 << Kolab::EventObject << getPath("v2/event/horde.ics") << getPath("v2/event/horde.ics.mime");
    // TODO read-only test, we never write base64
    // QTest::newRow("v3utf8base64") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf8base64.ics.mime");
    // TODO read-only test, we never write 8-bit
    // QTest::newRow("v3utf88bit") << Kolab::KolabV3 << Kolab::EventObject << getPath("v3/event/utf88bit.ics.mime");
}

void MIMEObjectTest::testEvent()
{
    testFunction<Kolab::Event>(&Kolab::MIMEObject::readEvent, &Kolab::MIMEObject::writeEvent);
}

void MIMEObjectTest::testTodo_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v2todoSimple") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/simple.ics.mime");
    QTest::newRow("v2todoComplex") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/complex.ics.mime");
    QTest::newRow("v2todoPrio1") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/prioritytest1.ics.mime");
    QTest::newRow("v2todoPrio2") << Kolab::KolabV2 << Kolab::TodoObject << getPath("v2/task/prioritytest2.ics.mime");
    QTest::newRow("v3todoSimple") << Kolab::KolabV3 << Kolab::TodoObject << getPath("v3/task/simple.ics.mime");
    QTest::newRow("v3todoComplex") << Kolab::KolabV3 << Kolab::TodoObject << getPath("v3/task/complex.ics.mime");
}

void MIMEObjectTest::testTodo()
{
    testFunction<Kolab::Todo>(&Kolab::MIMEObject::readTodo, &Kolab::MIMEObject::writeTodo);
}

void MIMEObjectTest::testJournal_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v2journalSimple") << Kolab::KolabV2 << Kolab::JournalObject << getPath("v2/journal/simple.ics.mime");
    QTest::newRow("v2journalComplex") << Kolab::KolabV2 << Kolab::JournalObject << getPath("v2/journal/complex.ics.mime");
    QTest::newRow("v3journalSimple") << Kolab::KolabV3 << Kolab::JournalObject << getPath("v3/journal/simple.ics.mime");
    QTest::newRow("v3journalComplex") << Kolab::KolabV3 << Kolab::JournalObject << getPath("v3/journal/complex.ics.mime");
}

void MIMEObjectTest::testJournal()
{
    testFunction<Kolab::Journal>(&Kolab::MIMEObject::readJournal, &Kolab::MIMEObject::writeJournal);
}

void MIMEObjectTest::testNote_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v3noteSimple") << Kolab::KolabV3 << Kolab::NoteObject << getPath("v3/note/note.mime.mime");
}

void MIMEObjectTest::testNote()
{
    testFunction<Kolab::Note>(&Kolab::MIMEObject::readNote, &Kolab::MIMEObject::writeNote);
}

void MIMEObjectTest::testContact_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v2contactSimple") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/simple.vcf.mime");
    QTest::newRow("v2contactAddress") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/address.vcf.mime");
    // FIXME Figure out why this is broken
    // QTest::newRow("v2contactBug238996") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/bug238996.vcf.mime");
    QTest::newRow("v2contactDisplayname") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/displayname.vcf.mime");
    QTest::newRow("v2contactEmails") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/emails.vcf.mime");
    QTest::newRow("v2contactPhonenumbers") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/phonenumbers.vcf.mime");
    QTest::newRow("v3contactSimple") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/contacts/simple.vcf.mime");
    QTest::newRow("v3contactComplex") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/contacts/complex.vcf.mime");

    // FIXME Tested in read-only mode in formattest, perhaps move to read-only test that compares with ical?
    // QTest::newRow("v3contactPng") << Kolab::KolabV3 << Kolab::ContactObject << getPath("v3/readonly/png.vcf.mime");

    // FIXME Reference files needs to be adjusted due to fix in how pictures are stored
    // QTest::newRow("v2contactComplex") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/complex.vcf.mime");
    // FIXME Reference files needs to be adjusted due to fix in how pictures are stored
    // QTest::newRow("v2contactPicture") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/picture.vcf.mime");
    // FIXME the following test fails because the vcard implementation always writes jpeg (which is lossy).
    // The reference vcf file is therefore probably also not really useful
    // QTest::newRow("v2pictureJPGHorde") << Kolab::KolabV2 << Kolab::ContactObject << getPath("v2/contacts/pictureJPGHorde.vcf.mime");
}

void MIMEObjectTest::testContact()
{
    testFunction<Kolab::Contact>(&Kolab::MIMEObject::readContact, &Kolab::MIMEObject::writeContact);
}

void MIMEObjectTest::testDistlist_data()
{
    QTest::addColumn<Kolab::Version>("version");
    QTest::addColumn<Kolab::ObjectType>("type");
    QTest::addColumn<QString>("mimeFileName");

    QTest::newRow("v3distlistSimple") << Kolab::KolabV3 << Kolab::DistlistObject << getPath("v3/contacts/distlist.vcf.mime");
}

void MIMEObjectTest::testDistlist()
{
    testFunction<Kolab::DistList>(&Kolab::MIMEObject::readDistlist, &Kolab::MIMEObject::writeDistlist);
}

QTEST_MAIN(MIMEObjectTest)
