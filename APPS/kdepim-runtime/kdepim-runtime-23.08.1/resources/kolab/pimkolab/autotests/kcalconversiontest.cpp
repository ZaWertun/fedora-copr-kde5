/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kcalconversiontest.h"
#include <contact.h>

#include <KCalendarCore/Recurrence>
#include <KContacts/Addressee>
#include <QTest>
#include <kolabcontact.h>

#include "conversion/kabcconversion.h"
#include "conversion/kcalconversion.cpp"
#include "conversion/kcalconversion.h"
#include "testhelpers.h"

using namespace Kolab::Conversion;

template<typename T>
void comparePointerVectors(const QVector<T> &list, const QVector<T> &other)
{
    QCOMPARE(list.size(), other.size());
    for (int i = 0; i < list.size(); i++) {
        QCOMPARE(*list.at(i), *other.at(i));
    }
}

void compareAttendeesVectors(const KCalendarCore::Attendee::List &list, const KCalendarCore::Attendee::List &other)
{
    QCOMPARE(list.size(), other.size());
    for (int i = 0; i < list.size(); i++) {
        KCalendarCore::Attendee at1 = list.at(i);
        at1.setUid(QString());
        KCalendarCore::Attendee at2 = other.at(i);
        at2.setUid(QString());
        QCOMPARE(at1, at2);
    }
}

void KCalConversionTest::initTestCase()
{
}

void KCalConversionTest::testDate_data()
{
    QTest::addColumn<Kolab::cDateTime>("input");
    QTest::addColumn<QDateTime>("result");

    QTest::newRow("datetime with tz") << Kolab::cDateTime("Europe/Zurich", 2006, 1, 8, 12, 0, 0)
                                      << QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), QTimeZone("Europe/Zurich"));
    QTest::newRow("floating datetime") << Kolab::cDateTime(2006, 1, 8, 12, 0, 0, false) << QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0));
    QTest::newRow("utc datetime") << Kolab::cDateTime(2006, 1, 8, 12, 0, 0, true) << QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), Qt::UTC);
    QTest::newRow("date only") << Kolab::cDateTime(2006, 1, 8) << QDateTime(QDate(2006, 1, 8), {});
}

void KCalConversionTest::testDate()
{
    QFETCH(Kolab::cDateTime, input);
    QFETCH(QDateTime, result);

    const QDateTime &r = Kolab::Conversion::toDate(input);
    QCOMPARE(r, result);

    const Kolab::cDateTime &r2 = Kolab::Conversion::fromDate(result, input.isDateOnly());
    QCOMPARE(r2, input);
}

void KCalConversionTest::testDuration_data()
{
    QTest::addColumn<Kolab::Duration>("input");
    QTest::addColumn<KCalendarCore::Duration>("result");
    QTest::addColumn<Kolab::Duration>("fromResult");

    QTest::newRow("seconds") << Kolab::Duration(0, 0, 0, 30, false) << KCalendarCore::Duration(30, KCalendarCore::Duration::Seconds)
                             << Kolab::Duration(0, 0, 0, 30, false);
    QTest::newRow("minutes") << Kolab::Duration(0, 0, 1, 30, false) << KCalendarCore::Duration(90, KCalendarCore::Duration::Seconds)
                             << Kolab::Duration(0, 0, 0, 90, false);
    QTest::newRow("hours") << Kolab::Duration(0, 1, 1, 30, false) << KCalendarCore::Duration(60 * 60 + 90, KCalendarCore::Duration::Seconds)
                           << Kolab::Duration(0, 0, 0, 60 * 60 + 90, false);
    QTest::newRow("days") << Kolab::Duration(1, 1, 1, 30, false) << KCalendarCore::Duration(24 * 60 * 60 + 60 * 60 + 90, KCalendarCore::Duration::Seconds)
                          << Kolab::Duration(0, 0, 0, 24 * 60 * 60 + 60 * 60 + 90, false);
    QTest::newRow("daysonly") << Kolab::Duration(30, 0, 0, 0, false) << KCalendarCore::Duration(30, KCalendarCore::Duration::Days)
                              << Kolab::Duration(30, 0, 0, 0, false);
    QTest::newRow("weeks") << Kolab::Duration(30, false) << KCalendarCore::Duration(30 * 7, KCalendarCore::Duration::Days)
                           << Kolab::Duration(30 * 7, 0, 0, 0, false);
}

void KCalConversionTest::testDuration()
{
    QFETCH(Kolab::Duration, input);
    QFETCH(KCalendarCore::Duration, result);
    QFETCH(Kolab::Duration, fromResult);

    const KCalendarCore::Duration &r = Kolab::Conversion::toDuration(input);
    QCOMPARE(r, result);

    const Kolab::Duration &r2 = Kolab::Conversion::fromDuration(result);
    QCOMPARE(r2, fromResult);
}

void KCalConversionTest::testDateTZ_data()
{
    QTest::addColumn<Kolab::cDateTime>("input");
    QTest::addColumn<QDateTime>("result");

    QTest::newRow("berlin") << Kolab::cDateTime("Europe/Berlin", 2006, 1, 8, 12, 0, 0)
                            << QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), QTimeZone("Europe/Berlin"));
}

void KCalConversionTest::testDateTZ()
{
    QFETCH(Kolab::cDateTime, input);
    QFETCH(QDateTime, result);

    const QDateTime &r = Kolab::Conversion::toDate(input);
    QCOMPARE(QString::fromUtf8(result.timeZone().id()), QString::fromStdString(input.timezone()));
    QCOMPARE(r.timeZone().offsetFromUtc(QDateTime::currentDateTimeUtc()), result.timeZone().offsetFromUtc(QDateTime::currentDateTimeUtc()));

    const Kolab::cDateTime &r2 = Kolab::Conversion::fromDate(result, input.isDateOnly());
    QCOMPARE(QString::fromStdString(r2.timezone()), QString::fromUtf8(result.timeZone().id()));
}

void KCalConversionTest::testConversion_data()
{
    QTest::addColumn<KCalendarCore::Event>("kcal");
    QTest::addColumn<Kolab::Event>("kolab");

    Kolab::cDateTime date(2011, 2, 2, 12, 11, 10, true);
    Kolab::cDateTime date2(2011, 2, 2, 12, 12, 10, true);
    Kolab::cDateTime date3(2012, 2, 2, 12, 12, 10, true);
    std::vector<int> intVector;
    intVector.push_back(1);
    intVector.push_back(-3);
    intVector.push_back(2);
    std::vector<std::string> stringVector;
    stringVector.emplace_back("cat1");
    stringVector.emplace_back("cat2");
    stringVector.emplace_back("parent/child");

    {
        KCalendarCore::Event kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setRevision(3);
        kcal.setSecrecy(KCalendarCore::Incidence::SecrecyConfidential);
        kcal.setCategories(toStringList(stringVector));
        kcal.setDtStart(toDate(date));
        kcal.setDtEnd(toDate(date2));
        kcal.setAllDay(date.isDateOnly());
        kcal.setTransparency(KCalendarCore::Event::Transparent);

        kcal.setRecurrenceId(toDate(date2)); // TODO THISANDFUTURE
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setDuration(5);
        kcal.recurrence()->addRDateTime(toDate(date2));
        kcal.recurrence()->addRDate(toDate(date2).date());
        kcal.recurrence()->addExDateTime(toDate(date3));
        kcal.recurrence()->addExDate(toDate(date3).date());

        KCalendarCore::RecurrenceRule *rr = kcal.recurrence()->defaultRRule(true);
        QList<int> intList(intVector.cbegin(), intVector.cend());
        rr->setBySeconds(intList);
        rr->setByMinutes(intList);
        rr->setByHours(intList);
        rr->setByDays(QList<KCalendarCore::RecurrenceRule::WDayPos>()
                      << KCalendarCore::RecurrenceRule::WDayPos(3, 1) << KCalendarCore::RecurrenceRule::WDayPos(5, 4));
        rr->setByMonthDays(intList);
        rr->setByYearDays(intList);
        rr->setByMonths(intList);
        rr->setByWeekNumbers(intList);

        kcal.setSummary(QStringLiteral("summary"));
        kcal.setDescription(QStringLiteral("description"));
        kcal.setPriority(3);
        kcal.setStatus(KCalendarCore::Incidence::StatusConfirmed);
        kcal.setLocation(QStringLiteral("location"));
        kcal.setOrganizer(KCalendarCore::Person(QStringLiteral("organizer"), QStringLiteral("organizer@email")));
        // Url
        kcal.setNonKDECustomProperty("X-KOLAB-URL", QStringLiteral("http://test.org"));
        KCalendarCore::Attendee att(QStringLiteral("attendee"),
                                    QStringLiteral("attendee@email"),
                                    false,
                                    KCalendarCore::Attendee::NeedsAction,
                                    KCalendarCore::Attendee::ReqParticipant);
        att.setDelegate(QStringLiteral("mailto:delegatee<delegatee@email>"));
        att.setDelegator(QStringLiteral("mailto:delegator<delegator@email>"));
        kcal.addAttendee(att);
        kcal.addAttachment(KCalendarCore::Attachment(QStringLiteral("uri"), QStringLiteral("mimetype/mime")));
        KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(&kcal));
        KCalendarCore::Person::List addressees;
        addressees.append(KCalendarCore::Person(QStringLiteral("name"), QStringLiteral("email@email")));
        alarm->setEmailAlarm(QStringLiteral("subject"), QStringLiteral("text"), addressees, QStringList()); // No support for attachments
        kcal.addAlarm(alarm);
        // TODO alarms

        kcal.setNonKDECustomProperty("X-KOLAB-key1", QStringLiteral("value1"));
        kcal.setNonKDECustomProperty("X-KOLAB-key2", QStringLiteral("value2"));
        kcal.setCustomProperty("SOMEOTHERAPP", "key2", QStringLiteral("value2"));
        Q_ASSERT(kcal.nonKDECustomProperty("X-KOLAB-key1") == QLatin1String("value1"));

        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setSequence(3);
        kolab.setClassification(Kolab::ClassConfidential);
        kolab.setCategories(stringVector);
        kolab.setStart(date);
        kolab.setEnd(date2);
        kolab.setTransparency(true);

        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setCount(5);
        rrule.setBysecond(intVector);
        rrule.setByminute(intVector);
        rrule.setByhour(intVector);
        rrule.setByday(std::vector<Kolab::DayPos>() << Kolab::DayPos(3, Kolab::Monday) << Kolab::DayPos(5, Kolab::Thursday));
        rrule.setBymonthday(intVector);
        rrule.setByyearday(intVector);
        rrule.setByweekno(intVector);
        rrule.setBymonth(intVector);

        kolab.setRecurrenceRule(rrule);
        kolab.setRecurrenceID(date2, true);
        kolab.setRecurrenceDates(std::vector<Kolab::cDateTime>() << date2 << Kolab::cDateTime(date2.year(), date2.month(), date2.day()));
        kolab.setExceptionDates(std::vector<Kolab::cDateTime>() << date3 << Kolab::cDateTime(date3.year(), date3.month(), date3.day()));

        kolab.setSummary("summary");
        kolab.setDescription("description");
        kolab.setPriority(3);
        kolab.setStatus(Kolab::StatusConfirmed);
        kolab.setLocation("location");
        kolab.setOrganizer(Kolab::ContactReference(Kolab::ContactReference::EmailReference, "organizer@email", "organizer")); // TODO uid
        kolab.setUrl("http://test.org");

        Kolab::Attendee a(Kolab::ContactReference(Kolab::ContactReference::EmailReference, "attendee@email", "attendee")); // TODO uid
        a.setDelegatedTo(std::vector<Kolab::ContactReference>()
                         << Kolab::ContactReference(Kolab::ContactReference::EmailReference, "delegatee@email", "delegatee"));
        a.setDelegatedFrom(std::vector<Kolab::ContactReference>()
                           << Kolab::ContactReference(Kolab::ContactReference::EmailReference, "delegator@email", "delegator"));
        a.setCutype(Kolab::CutypeIndividual);
        kolab.setAttendees(std::vector<Kolab::Attendee>() << a);

        Kolab::Attachment attach;
        attach.setUri("uri", "mimetype/mime");
        kolab.setAttachments(std::vector<Kolab::Attachment>() << attach);

        //     std::vector<std::string> receipents;
        //     receipents.push_back("email@email");
        //     Kolab::Alarm alarm2("summary", "description", receipents);
        //     kolab.setAlarms(std::vector<Kolab::Alarm>() << alarm2);

        // The sorting is random, just sort them here how we think they should arrive so we don't have to sort during compare (due to laziness).
        std::vector<Kolab::CustomProperty> customproperties;
        customproperties.emplace_back("X-KDE-SOMEOTHERAPP-key2", "value2");
        customproperties.emplace_back("key1", "value1");
        customproperties.emplace_back("key2", "value2");

        kolab.setCustomProperties(customproperties);

        QTest::newRow("with endDate and recurrence duration") << kcal << kolab;
    }
    {
        KCalendarCore::Event kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setRevision(3);
        kcal.setDtStart(toDate(date));
        kcal.setAllDay(date.isDateOnly());
        kcal.setDuration(KCalendarCore::Duration(toDate(date), toDate(date2)));
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setEndDateTime(toDate(date3));

        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setSequence(3);
        kolab.setStart(date);
        kolab.setDuration(Kolab::Duration(0, 0, 1, 0));
        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setEnd(date3);
        kolab.setRecurrenceRule(rrule);

        QTest::newRow("with duration and recurrence endDate") << kcal << kolab;
    }
    {
        Kolab::cDateTime start(2011, 1, 1);
        Kolab::cDateTime end(2011, 1, 3);

        KCalendarCore::Event kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setDtStart(toDate(start));
        kcal.setDtEnd(toDate(end));
        kcal.setAllDay(start.isDateOnly());
        kcal.recurrence()->setDaily(3);
        kcal.recurrence()->setEndDateTime(toDate(end));

        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setStart(start);
        kolab.setEnd(end);
        Kolab::RecurrenceRule rrule;
        rrule.setInterval(3);
        rrule.setFrequency(Kolab::RecurrenceRule::Daily);
        rrule.setEnd(end);
        kolab.setRecurrenceRule(rrule);

        QTest::newRow("date only dates") << kcal << kolab;
    }
    {
        KCalendarCore::Event kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setCreated(toDate(date));
        kcal.setLastModified(toDate(date));
        kcal.setDtStart(toDate(date));
        kcal.setAllDay(date.isDateOnly());
        kcal.setSummary(QStringLiteral("äöü%@$£é¤¼²°€Š�"));

        Kolab::Event kolab;
        kolab.setUid("uid");
        kolab.setCreated(date);
        kolab.setLastModified(date);
        kolab.setStart(date);
        kolab.setSummary(std::string(QStringLiteral("äöü%@$£é¤¼²°€Š�").toUtf8().constData()));

        QTest::newRow("latin1+Unicode") << kcal << kolab;
    }
}

void KCalConversionTest::testConversion()
{
    QFETCH(KCalendarCore::Event, kcal);
    QFETCH(Kolab::Event, kolab);

    KCalendarCore::Event::Ptr e = toKCalendarCore(kolab);
    const Kolab::Event &b = fromKCalendarCore(kcal);

    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->created(), kcal.created());
    QCOMPARE(e->lastModified(), kcal.lastModified());
    QCOMPARE(e->revision(), kcal.revision());
    QCOMPARE(e->secrecy(), kcal.secrecy());
    QCOMPARE(e->categories(), kcal.categories());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->dtEnd(), kcal.dtEnd());
    QCOMPARE(e->duration(), kcal.duration());
    QCOMPARE(e->transparency(), kcal.transparency());
    QCOMPARE(*e->recurrence(), *kcal.recurrence());
    QCOMPARE(e->recurrenceId(), kcal.recurrenceId());
    QCOMPARE(e->recurrenceType(), kcal.recurrenceType());
    QCOMPARE(e->summary(), kcal.summary());
    QCOMPARE(e->description(), kcal.description());
    QCOMPARE(e->priority(), kcal.priority());
    QCOMPARE(e->status(), kcal.status());
    QCOMPARE(e->location(), kcal.location());
    QCOMPARE(e->organizer().name(), kcal.organizer().name());
    QCOMPARE(e->organizer().email(), kcal.organizer().email());
    QCOMPARE(e->nonKDECustomProperty("X-KOLAB-URL"), kcal.nonKDECustomProperty("X-KOLAB-URL"));
    // otherwise we'd break the customProperties comparison
    e->removeNonKDECustomProperty("X-KOLAB-URL");
    kcal.removeNonKDECustomProperty("X-KOLAB-URL");
    compareAttendeesVectors(e->attendees(), kcal.attendees());
    QCOMPARE(e->attachments(), kcal.attachments());

    //     QCOMPARE(e->alarms(), kcal.alarms()); //TODO
    QCOMPARE(e->customProperties(), kcal.customProperties());

    //     QBENCHMARK {
    //         toKCalendarCore(kolab);
    //     }

    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.created(), kolab.created());
    QCOMPARE(b.lastModified(), kolab.lastModified());
    QCOMPARE(b.sequence(), kolab.sequence());
    QCOMPARE(b.classification(), kolab.classification());
    QCOMPARE(b.categories(), kolab.categories());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.end(), kolab.end());
    QCOMPARE(b.duration(), kolab.duration());
    QCOMPARE(b.transparency(), kolab.transparency());

    QCOMPARE(b.recurrenceRule(), kolab.recurrenceRule());
    QCOMPARE(b.recurrenceID(), kolab.recurrenceID());
    QCOMPARE(b.recurrenceDates(), kolab.recurrenceDates());
    QCOMPARE(b.exceptionDates(), kolab.exceptionDates());

    QCOMPARE(b.summary(), kolab.summary());
    QCOMPARE(b.description(), kolab.description());
    QCOMPARE(b.status(), kolab.status());
    QCOMPARE(b.location(), kolab.location());
    QCOMPARE(b.organizer(), kolab.organizer());
    QCOMPARE(b.url(), kolab.url());
    QCOMPARE(b.attendees(), kolab.attendees());
    QCOMPARE(b.attachments(), kolab.attachments());
    QCOMPARE(b.customProperties(), kolab.customProperties());
}

void KCalConversionTest::testTodoConversion_data()
{
    QTest::addColumn<KCalendarCore::Todo>("kcal");
    QTest::addColumn<Kolab::Todo>("kolab");

    Kolab::cDateTime date(2011, 2, 2, 12, 11, 10, true);
    Kolab::cDateTime date2(2011, 2, 2, 12, 12, 10, true);

    {
        KCalendarCore::Todo kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setDtStart(toDate(date));
        kcal.setDtDue(toDate(date2));
        kcal.setAllDay(date.isDateOnly());
        kcal.setRelatedTo(QStringLiteral("uid2"), KCalendarCore::Incidence::RelTypeParent);

        Kolab::Todo kolab;
        kolab.setUid("uid");
        kolab.setStart(date);
        kolab.setDue(date2);
        std::vector<std::string> relateds;
        relateds.emplace_back("uid2");
        kolab.setRelatedTo(relateds);

        QTest::newRow("todo") << kcal << kolab;
    }
}

void KCalConversionTest::testTodoConversion()
{
    QFETCH(KCalendarCore::Todo, kcal);
    QFETCH(Kolab::Todo, kolab);

    const KCalendarCore::Todo::Ptr e = toKCalendarCore(kolab);

    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->dtDue(), kcal.dtDue());
    QCOMPARE(e->relatedTo(KCalendarCore::Incidence::RelTypeParent), kcal.relatedTo(KCalendarCore::Incidence::RelTypeParent));

    const Kolab::Todo &b = fromKCalendarCore(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.due(), kolab.due());
    QCOMPARE(b.relatedTo(), kolab.relatedTo());
}

void KCalConversionTest::testJournalConversion_data()
{
    QTest::addColumn<KCalendarCore::Journal>("kcal");
    QTest::addColumn<Kolab::Journal>("kolab");

    Kolab::cDateTime date(2011, 2, 2, 12, 11, 10, true);
    Kolab::cDateTime date2(2011, 2, 2, 12, 12, 10, true);

    {
        KCalendarCore::Journal kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setDtStart(toDate(date));
        kcal.setSummary(QStringLiteral("summary"));

        Kolab::Journal kolab;
        kolab.setUid("uid");
        kolab.setStart(date);
        kolab.setSummary("summary");

        QTest::newRow("journal") << kcal << kolab;
    }
}

void KCalConversionTest::testJournalConversion()
{
    QFETCH(KCalendarCore::Journal, kcal);
    QFETCH(Kolab::Journal, kolab);

    const KCalendarCore::Journal::Ptr e = toKCalendarCore(kolab);

    QCOMPARE(e->uid(), kcal.uid());
    QCOMPARE(e->dtStart(), kcal.dtStart());
    QCOMPARE(e->summary(), kcal.summary());

    const Kolab::Journal &b = fromKCalendarCore(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.start(), kolab.start());
    QCOMPARE(b.summary(), kolab.summary());
}

void KCalConversionTest::testContactConversion_data()
{
    QTest::addColumn<KContacts::Addressee>("kcal");
    QTest::addColumn<Kolab::Contact>("kolab");

    {
        KContacts::Addressee kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setFormattedName(QStringLiteral("name"));

        Kolab::Contact kolab;
        kolab.setUid("uid");
        kolab.setName("name");

        QTest::newRow("basic") << kcal << kolab;
    }
    {
        KContacts::Addressee kcal;
        kcal.setUid(QStringLiteral("uid"));
        kcal.setFormattedName(QStringLiteral("name"));
        kcal.setBirthday(QDate(2012, 2, 2).startOfDay());

        // Because QDateTime doesn't know date-only values we always end up with a date-time
        Kolab::Contact kolab;
        kolab.setUid("uid");
        kolab.setName("name");
        kolab.setBDay(Kolab::cDateTime(2012, 2, 2, 0, 0, 0));

        QTest::newRow("bday") << kcal << kolab;
    }
    {
        KContacts::Addressee kcal;
        kcal.setUid(QStringLiteral("uid"));
        // The first address is always the preferred
        kcal.setEmails(QStringList() << QStringLiteral("email1@example.org") << QStringLiteral("email2@example.org"));
        kcal.insertCustom(QStringLiteral("KOLAB"), QStringLiteral("EmailTypesemail1@example.org"), QStringLiteral("home,work"));

        Kolab::Contact kolab;
        kolab.setUid("uid");

        Kolab::Email email1("email1@example.org", Kolab::Email::Work | Kolab::Email::Home);
        Kolab::Email email2("email2@example.org");

        std::vector<Kolab::Email> emails;
        emails.push_back(email1);
        emails.push_back(email2);
        kolab.setEmailAddresses(emails, 0);

        QTest::newRow("emailTypesAndPreference") << kcal << kolab;
    }
}

void KCalConversionTest::testContactConversion()
{
    QFETCH(KContacts::Addressee, kcal);
    QFETCH(Kolab::Contact, kolab);

    const KContacts::Addressee &e = toKABC(kolab);

    QCOMPARE(e.uid(), kcal.uid());
    QCOMPARE(e.formattedName(), kcal.formattedName());
    QCOMPARE(e.emails(), kcal.emails());
    QCOMPARE(e.preferredEmail(), kcal.preferredEmail());
    const auto mails{e.emails()};
    for (const QString &mail : mails) {
        QCOMPARE(e.custom(QLatin1String("KOLAB"), QString::fromLatin1("EmailTypes%1").arg(mail)),
                 kcal.custom(QLatin1String("KOLAB"), QString::fromLatin1("EmailTypes%1").arg(mail)));
    }
    QCOMPARE(e.birthday(), kcal.birthday());

    const Kolab::Contact &b = fromKABC(kcal);
    QCOMPARE(b.uid(), kolab.uid());
    QCOMPARE(b.name(), kolab.name());
    QCOMPARE(b.emailAddresses(), kolab.emailAddresses());
    QCOMPARE(b.emailAddressPreferredIndex(), kolab.emailAddressPreferredIndex());
    QCOMPARE(b.bDay(), kolab.bDay());
}

// void KCalConversionTest::BenchmarkRoundtripKCAL()
// {
//     const Kolab::Event &event = Kolab::readEvent(TEST_DATA_PATH "/testfiles/icalEvent.xml", true);
//     std::string result = Kolab::writeEvent(event);
//     QBENCHMARK {
//         Kolab::Conversion::toKCalendarCore(Kolab::readEvent(result, false));
//     }
// }

QTEST_MAIN(KCalConversionTest)
