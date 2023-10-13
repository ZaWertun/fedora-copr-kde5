/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <contact.h>

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Recurrence>
#include <KCalendarCore/Todo>
#include <QTest>

#include <kolabformat.h>

Q_DECLARE_METATYPE(Kolab::Duration)
Q_DECLARE_METATYPE(Kolab::cDateTime)
Q_DECLARE_METATYPE(std::vector<Kolab::cDateTime>)
Q_DECLARE_METATYPE(Kolab::Event)
Q_DECLARE_METATYPE(std::vector<Kolab::Event>)
Q_DECLARE_METATYPE(Kolab::Todo)
Q_DECLARE_METATYPE(Kolab::Journal)
Q_DECLARE_METATYPE(Kolab::Contact)
Q_DECLARE_METATYPE(Kolab::Period)
Q_DECLARE_METATYPE(std::vector<Kolab::FreebusyPeriod>)

Q_DECLARE_METATYPE(KCalendarCore::Event)
Q_DECLARE_METATYPE(KCalendarCore::Todo)
Q_DECLARE_METATYPE(KCalendarCore::Journal)

namespace QTest
{
template<>
char *toString(const Kolab::cDateTime &dt)
{
    QByteArray ba = "Kolab::cDateTime(";
    ba += QByteArray::number(dt.year()) + ", " + QByteArray::number(dt.month()) + ", " + QByteArray::number(dt.day()) + ", ";
    ba += QByteArray::number(dt.hour()) + ", " + QByteArray::number(dt.minute()) + ", " + QByteArray::number(dt.second()) + ", ";
    ba += QByteArray(dt.isUTC() ? QByteArray("UTC") : QByteArray("TZ: " + QByteArray::fromStdString(dt.timezone())));
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const KCalendarCore::Attendee &at)
{
    QByteArray ba = "Attendee(";
    ba += at.name().toLatin1() + ", ";
    ba += at.email().toLatin1() + ", ";
    ba += QByteArray::number(at.role()) + ", ";
    ba += QByteArray::number(at.status()) + ", ";
    ba += QByteArray::number(at.RSVP()) + ", ";
    ba += at.delegate().toLatin1() + ", ";
    ba += at.delegator().toLatin1() + ", ";
    ba += at.uid().toLatin1() + ", ";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const QList<int> &l)
{
    QByteArray ba = "QList<int>(";
    for (int i : l) {
        ba += QByteArray::number(i) + ", ";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const QList<KCalendarCore::RecurrenceRule::WDayPos> &l)
{
    QByteArray ba = "QList<int>(";
    for (const KCalendarCore::RecurrenceRule::WDayPos &i : l) {
        ba += QByteArray::number(i.pos()) + " ";
        ba += QByteArray::number(i.day()) + ", ";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const KCalendarCore::DateList &l)
{
    QByteArray ba = "KCalendarCore::DateList(";
    for (const QDate &i : l) {
        ba += i.toString().toLatin1();
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const KCalendarCore::DateTimeList &l)
{
    QByteArray ba = "KCalendarCore::DateTimeList(";
    for (const QDateTime &i : l) {
        ba += toString(i);
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const KCalendarCore::Recurrence &at)
{
    // at.dump();
    KCalendarCore::RecurrenceRule *r = at.defaultRRule();
    QByteArray ba;
    if (!r) {
        ba += "Recurrence( )";
    } else {
        Q_ASSERT(r);
        Q_ASSERT(at.rRules().size() == 1);

        ba += "Recurrence(";
        ba += QByteArray::number(r->recurrenceType()) + "\n";
        ba += QByteArray::number(r->frequency()) + "\n";
        ba += QByteArray::number(r->duration()) + "\n";
        ba += QByteArray(toString(r->startDt())) + "\n";
        ba += QByteArray(toString(r->endDt())) + "\n";
        ba += QByteArray(toString(r->bySeconds())) + "\n";
        ba += QByteArray(toString(r->byMinutes())) + "\n";
        ba += QByteArray(toString(r->byHours())) + "\n";
        ba += QByteArray(toString(r->byDays())) + "\n";
        ba += QByteArray(toString(r->byMonthDays())) + "\n";
        ba += QByteArray(toString(r->byYearDays())) + "\n";
        ba += QByteArray(toString(r->byMonths())) + "\n";
        ba += ")\n";
        ba += QByteArray(toString(at.exDates())) + "\n";
        ba += QByteArray(toString(at.exDateTimes())) + "\n";
        ba += QByteArray(toString(at.rDates())) + "\n";
        ba += QByteArray(toString(at.rDateTimes())) + "\n";
    }
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::RecurrenceRule &at)
{
    QByteArray ba;
    ba += "KolabRecurrenceRule(";
    ba += QByteArray::number(at.weekStart()) + "\n";
    ba += QByteArray::number(at.frequency()) + "\n";
    ba += QByteArray::number(at.interval()) + "\n";
    ba += QByteArray::number(at.count()) + "\n";
    ba += QByteArray(toString(at.end())) + "\n";
    ba += QByteArray(toString(at.bysecond())) + "\n";
    ba += QByteArray(toString(at.byminute())) + "\n";
    ba += QByteArray(toString(at.byhour())) + "\n";
    ba += QByteArray(toString(at.byday())) + "\n";
    ba += QByteArray(toString(at.bymonthday())) + "\n";
    ba += QByteArray(toString(at.byyearday())) + "\n";
    ba += QByteArray(toString(at.byweekno())) + "\n";
    ba += QByteArray(toString(at.bymonth())) + "\n";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const KCalendarCore::Duration &d)
{
    QByteArray ba;
    ba += "KCalendarCore::Duration(";
    ba += QByteArray::number(d.isDaily()) + ", ";
    ba += QByteArray::number(d.value()) + " ";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::ContactReference &a)
{
    QByteArray ba = "Kolab::ContactReference(";
    ba += QByteArray::fromStdString(a.email()) + ", ";
    ba += QByteArray::fromStdString(a.name()) + ", ";
    ba += QByteArray::fromStdString(a.uid());
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const std::vector<Kolab::ContactReference> &v)
{
    QByteArray ba = "vector<Kolab::ContactReference>(";
    for (std::size_t i = 0; i < v.size(); i++) {
        ba += QByteArray(toString(v.at(i))) + "\n";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::Attendee &a)
{
    QByteArray ba = "Kolab::Attendee(";
    ba += QByteArray::fromStdString(a.contact().email()) + "\n";
    ba += QByteArray::fromStdString(a.contact().name()) + "\n";
    ba += QByteArray::number(a.partStat()) + "\n";
    ba += QByteArray::number(a.role()) + "\n";
    ba += QByteArray::number(a.rsvp()) + "\n";
    ba += QByteArray::fromStdString(a.contact().uid()) + "\n";
    ba += QByteArray(toString(a.delegatedTo())) + "\n";
    ba += QByteArray(toString(a.delegatedFrom())) + "\n";
    ba += QByteArray::number(a.cutype()) + "\n";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const std::vector<Kolab::Attendee> &v)
{
    QByteArray ba = "vector<Kolab::Attendee>(";
    for (std::size_t i = 0; i < v.size(); i++) {
        ba += QByteArray(toString(v.at(i))) + "\n";
        ba += QByteArray("#######################") + "\n";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::CustomProperty &a)
{
    QByteArray ba = "Kolab::CustomProperty(";
    ba += QByteArray::fromStdString(a.identifier) + ", ";
    ba += QByteArray::fromStdString(a.value);
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const std::vector<Kolab::CustomProperty> &v)
{
    QByteArray ba = "vector<Kolab::CustomProperty>(";
    for (std::size_t i = 0; i < v.size(); i++) {
        ba += QByteArray(toString(v.at(i))) + "\n";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::Period &p)
{
    QByteArray ba = "Kolab::Period(";
    ba += QByteArray(toString(p.start)) + "\n";
    ba += QByteArray(toString(p.end)) + "\n";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const std::vector<Kolab::Period> &v)
{
    QByteArray ba = "vector<Kolab::Period>(";
    for (std::size_t i = 0; i < v.size(); i++) {
        ba += QByteArray(toString(v.at(i))) + "\n";
    }
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::FreebusyPeriod &p)
{
    QByteArray ba = "Kolab::FreebusyPeriod(";
    ba += QByteArray::number(p.type()) + "\n";
    ba += QByteArray::fromStdString(p.eventUid()) + "\n";
    ba += QByteArray::fromStdString(p.eventLocation()) + "\n";
    ba += QByteArray::fromStdString(p.eventSummary()) + "\n";
    ba += QByteArray(toString(p.periods())) + "\n";
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const Kolab::Duration &p)
{
    QByteArray ba = "Kolab::Duration";
    ba += p.isNegative() ? "-" : "+";
    ba += "(";
    ba += QByteArray::number(p.weeks()) + ", ";
    ba += QByteArray::number(p.days()) + ", ";
    ba += QByteArray::number(p.hours()) + ", ";
    ba += QByteArray::number(p.minutes()) + ", ";
    ba += QByteArray::number(p.seconds());
    ba += ")";
    return qstrdup(ba.data());
}
}
