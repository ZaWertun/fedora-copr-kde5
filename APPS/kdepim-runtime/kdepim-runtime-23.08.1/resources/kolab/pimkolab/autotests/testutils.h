/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <kolabevent.h>

#include "kolabformat/kolabobject.h"

#include <KMime/Message>

#include <QFile>
#include <QRegularExpression>
#include <QUuid>

Q_DECLARE_METATYPE(Kolab::ObjectType)
Q_DECLARE_METATYPE(Kolab::Version)

#define KCOMPARE(actual, expected)                                                                                                                             \
    do {                                                                                                                                                       \
        if (!(actual == expected)) {                                                                                                                           \
            qDebug() << __FILE__ << ':' << __LINE__ << "Actual: " #actual ": " << actual << "\nExpected: " #expected ": " << expected;                         \
            return false;                                                                                                                                      \
        }                                                                                                                                                      \
    } while (0)

#endif

#define DIFFCOMPARE(actual, expected)                                                                                                                          \
    do {                                                                                                                                                       \
        if (!(actual.simplified() == expected.simplified())) {                                                                                                 \
            qDebug() << "Content not the same.";                                                                                                               \
            qDebug() << "actual." << actual.simplified() << "\n";                                                                                              \
            qDebug() << "expected." << expected.simplified();                                                                                                  \
            showDiff(expected, actual);                                                                                                                        \
            QTest::qFail("Compared versions differ.", __FILE__, __LINE__);                                                                                     \
            return;                                                                                                                                            \
        }                                                                                                                                                      \
    } while (0)

#define TESTVALUE(type, name) *static_cast<type *>(QTest::qData(#name, ::qMetaTypeId<type>()))

const QString TESTFILEDIR = QString::fromLatin1(TEST_DATA_PATH "/testfiles/");

QString getPath(const char *file)
{
    return TESTFILEDIR + QString::fromLatin1(file);
}

void showDiff(const QString &expected, const QString &converted)
{
    if (expected.isEmpty() || converted.isEmpty()) {
        qWarning() << "files are empty";
        return;
    }
    if (expected == converted) {
        qWarning() << "contents are the same";
        return;
    }

    qDebug() << "EXPECTED: " << expected;
    qDebug() << "CONVERTED: " << converted;
}

KMime::Message::Ptr readMimeFile(const QString &fileName, bool &ok)
{
    //   qDebug() << fileName;
    QFile file(fileName);
    ok = file.open(QFile::ReadOnly);
    if (!ok) {
        qWarning() << "failed to open file: " << fileName;
        return {};
    }
    const QByteArray data = file.readAll();

    KMime::Message::Ptr msg = KMime::Message::Ptr(new KMime::Message);
    msg->setContent(data);
    msg->parse();

    return msg;
}

void normalizeMimemessage(QString &content)
{
    content.replace(QRegularExpression(QStringLiteral("\\bLibkolab-\\d.\\d.\\d\\b")), QStringLiteral("Libkolab-x.x.x"));
    content.replace(QRegularExpression(QStringLiteral("\\bLibkolabxml-\\d.\\d.\\d\\b")), QStringLiteral("Libkolabxml-x.x.x"));
    content.replace(QRegularExpression(QStringLiteral("\\bLibkolab-\\d.\\d\\b")), QStringLiteral("Libkolab-x.x.x"));
    content.replace(QRegularExpression(QStringLiteral("\\bkdepim-runtime-\\d.\\d\\b")), QStringLiteral("Libkolab-x.x.x"));
    content.replace(QRegularExpression(QStringLiteral("\\bLibkolabxml-\\d.\\d\\b")), QStringLiteral("Libkolabxml-x.x.x"));

    content.replace(QRegularExpression(QStringLiteral("<uri>cid:.*@kolab.resource.akonadi</uri>")), QStringLiteral("<uri>cid:id@kolab.resource.akonadi</uri>"));

    content.replace(QRegularExpression(QStringLiteral("Content-ID: <.*@kolab.resource.akonadi>")), QStringLiteral("Content-ID: <id@kolab.resource.akonadi>"));

    content.replace(QRegularExpression(QStringLiteral("<uri>mailto:.*</uri>")), QStringLiteral("<uri>mailto:</uri>"));
    content.replace(QRegularExpression(QStringLiteral("<cal-address>mailto:.*</cal-address>")), QStringLiteral("<cal-address>mailto:</cal-address>"));

    content.replace(QRegularExpression(QStringLiteral("<uri>data:.*</uri>")), QStringLiteral("<uri>data:</uri>"));

    content.replace(QRegularExpression(QStringLiteral("<last-modification-date>.*</last-modification-date>")),
                    QStringLiteral("<last-modification-date></last-modification-date>"));
    // We no longer support pobox, so remove pobox lines
    content.remove(QRegularExpression(QStringLiteral("<pobox>.*</pobox>")));

    content.replace(QRegularExpression(QStringLiteral("<timestamp>.*</timestamp>")), QStringLiteral("<timestamp></timestamp>"));

    // DotMatchesEverythingOption because <x-koblab-version> spans multiple lines
    content.replace(QRegularExpression(QStringLiteral("<x-kolab-version>.*</x-kolab-version>"), QRegularExpression::DotMatchesEverythingOption),
                    QStringLiteral("<x-kolab-version></x-kolab-version>"));

    content.replace(QRegularExpression(QStringLiteral("--nextPart\\S*")), QStringLiteral("--part"));
    content.replace(QRegularExpression(QStringLiteral("\\bboundary=\"nextPart[^\\n]*")), QStringLiteral("boundary"));

    content.replace(QRegularExpression(QStringLiteral("Date[^\\n]*")), QStringLiteral("Date"));
    // The sort order of the attributes in kolabV2 is unpredictable
    content.replace(QRegularExpression(QStringLiteral("<x-custom.*/>")), QStringLiteral("<x-custom/>"));
    // quoted-printable encoding changes where the linebreaks are every now and then (an all are valid), so we remove the linebreaks
    content.remove(QLatin1String("=\n"));
}

QString normalizeVCardMessage(QString content)
{
    // The encoding changes every now and then
    content.replace(QRegularExpression(QStringLiteral("ENCODING=b;TYPE=png:.*")), QStringLiteral("ENCODING=b;TYPE=png:picturedata"));
    return content;
}

// Normalize incidences for comparison
void normalizeIncidence(KCalendarCore::Incidence::Ptr incidence)
{
    // The UID is not persistent (it's just the internal pointer), therefore we clear it
    // TODO make sure that the UID does really not need to be persistent
    auto attendees = incidence->attendees();
    for (auto &attendee : attendees) {
        attendee.setUid(attendee.email());
    }
    incidence->setAttendees(attendees);

    // FIXME even if hasDueDate can differ, it shouldn't because it breaks equality. Check why they differ in the first place.
    if (incidence->type() == KCalendarCore::IncidenceBase::TypeTodo) {
        KCalendarCore::Todo::Ptr todo = incidence.dynamicCast<KCalendarCore::Todo>();
        Q_ASSERT(todo.data());
        if (!todo->hasDueDate() && !todo->hasStartDate()) {
            todo->setAllDay(false); // all day has no meaning if there are no start and due dates but may differ nevertheless
        }
    }
}

template<template<typename> class Op, typename T>
static bool LexicographicalCompare(const T &_x, const T &_y)
{
    T x(_x);
    x.setId(QString());
    T y(_y);
    y.setId(QString());
    Op<QString> op;
    return op(x.toString(), y.toString());
}

bool normalizePhoneNumbers(KContacts::Addressee &addressee, KContacts::Addressee &refAddressee)
{
    KContacts::PhoneNumber::List phoneNumbers = addressee.phoneNumbers();
    KContacts::PhoneNumber::List refPhoneNumbers = refAddressee.phoneNumbers();
    if (phoneNumbers.size() != refPhoneNumbers.size()) {
        return false;
    }
    std::sort(phoneNumbers.begin(), phoneNumbers.end(), LexicographicalCompare<std::less, KContacts::PhoneNumber>);
    std::sort(refPhoneNumbers.begin(), refPhoneNumbers.end(), LexicographicalCompare<std::less, KContacts::PhoneNumber>);

    for (int i = 0; i < phoneNumbers.size(); ++i) {
        KContacts::PhoneNumber phoneNumber = phoneNumbers.at(i);
        const KContacts::PhoneNumber refPhoneNumber = refPhoneNumbers.at(i);
        KCOMPARE(LexicographicalCompare<std::equal_to>(phoneNumber, refPhoneNumber), true);
        addressee.removePhoneNumber(phoneNumber);
        phoneNumber.setId(refPhoneNumber.id());
        addressee.insertPhoneNumber(phoneNumber);
        // Make sure that both have the same sorted order
        refAddressee.removePhoneNumber(refPhoneNumber);
        refAddressee.insertPhoneNumber(refPhoneNumber);
    }
    //     for ( int i = 0; i < phoneNumbers.size(); ++i ) {
    //         qDebug() << "--------------------------------------";
    //         qDebug() << addressee.phoneNumbers().at(i).toString();
    //         qDebug() << refAddressee.phoneNumbers().at(i).toString();
    //     }

    return true;
}

bool normalizeAddresses(KContacts::Addressee &addressee, const KContacts::Addressee &refAddressee)
{
    KContacts::Address::List addresses = addressee.addresses();
    KContacts::Address::List refAddresses = refAddressee.addresses();
    if (addresses.size() != refAddresses.size()) {
        return false;
    }
    std::sort(addresses.begin(), addresses.end(), LexicographicalCompare<std::less, KContacts::Address>);
    std::sort(refAddresses.begin(), refAddresses.end(), LexicographicalCompare<std::less, KContacts::Address>);

    for (int i = 0; i < addresses.size(); ++i) {
        KContacts::Address address = addresses.at(i);
        const KContacts::Address refAddress = refAddresses.at(i);
        KCOMPARE(LexicographicalCompare<std::equal_to>(address, refAddress), true);
        addressee.removeAddress(address);
        address.setId(refAddress.id());
        addressee.insertAddress(address);
    }

    return true;
}

void normalizeContact(KContacts::Addressee &addressee)
{
    const KContacts::Address::List addresses = addressee.addresses();

    for (KContacts::Address a : addresses) {
        addressee.removeAddress(a);
        a.setPostOfficeBox(QString()); // Not supported anymore
        addressee.insertAddress(a);
    }
    addressee.setSound(KContacts::Sound()); // Sound is not supported

    addressee.removeCustom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate")); // The creation date is no longer existing

    // Attachment names are no longer required because we identify the parts by cid and no longer by name
    addressee.removeCustom(QStringLiteral("KOLAB"), QStringLiteral("LogoAttachmentName"));
    addressee.removeCustom(QStringLiteral("KOLAB"), QStringLiteral("PictureAttachmentName"));
    addressee.removeCustom(QStringLiteral("KOLAB"), QStringLiteral("SoundAttachmentName"));
}

Kolab::Event createEvent(const Kolab::cDateTime &start, const Kolab::cDateTime &end)
{
    Kolab::Event event;
    event.setUid(QUuid::createUuid().toString().toStdString());
    event.setStart(start);
    event.setEnd(end);
    return event;
}
