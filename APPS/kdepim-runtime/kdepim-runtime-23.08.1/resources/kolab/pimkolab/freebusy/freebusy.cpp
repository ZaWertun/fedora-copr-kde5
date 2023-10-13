/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "freebusy.h"
#include "conversion/commonconversion.h"
#include "conversion/kcalconversion.h"
#include "libkolab-version.h"
#include "pimkolab_debug.h"
#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/ICalFormat>
#include <QTime>
#include <QUuid>

// namespace KCalendarCore {
//     struct KCalFreebusy
// {
//
// void init( const Event::List &eventList, const KDateTime &start, const KDateTime &end )
// {
//     mDtStart = start.toUtc();
//     mDtEnd = end.toUtc();
//
//   // Loops through every event in the calendar
//   Event::List::ConstIterator it;
//   for ( it = eventList.constBegin(); it != eventList.constEnd(); ++it ) {
//     Event::Ptr event = *it;
//
//     // If this event is transparent it shouldn't be in the freebusy list.
//     if ( event->transparency() == Event::Transparent ) {
//       continue;
//     }
//
//     if ( event->hasRecurrenceId() ) {
//       continue; //TODO apply special period exception (duration could be different)
//     }
//
//     const KDateTime eventStart = event->dtStart().toUtc();
//     const KDateTime eventEnd = event->dtEnd().toUtc();
//
//     if ( event->recurs() ) {
//         const KCalendarCore::Duration duration( eventStart, eventEnd );
//         const KCalendarCore::DateTimeList list = event->recurrence()->timesInInterval(start, end);
//         for (const KDateTime &dt : list) {
//             const KDateTime utc = dt.toUtc();
//             addLocalPeriod(utc, duration.end(utc) );
//         }
//     } else {
//         addLocalPeriod( eventStart, eventEnd );
//     }
//   }
//
// //   q->sortList();
// }
//
// bool addLocalPeriod(
//                                         const KDateTime &eventStart,
//                                         const KDateTime &eventEnd )
// {
//   KDateTime tmpStart;
//   KDateTime tmpEnd;
//
//   //Check to see if the start *or* end of the event is
//   //between the start and end of the freebusy dates.
//   if ( !( ( ( mDtStart.secsTo( eventStart ) >= 0 ) &&
//             ( eventStart.secsTo( mDtEnd ) >= 0 ) ) ||
//           ( ( mDtStart.secsTo( eventEnd ) >= 0 ) &&
//             ( eventEnd.secsTo( mDtEnd ) >= 0 ) ) ) ) {
//       qCDebug(PIMKOLAB_LOG) << "out of scope";
//     return false;
//   }
//
// //   qCDebug(PIMKOLAB_LOG) << eventStart.date().toString() << eventStart.time().toString() << mDtStart.toString();
//   if ( eventStart < mDtStart ) { //eventStart is before start
// //       qCDebug(PIMKOLAB_LOG) << "use start";
//     tmpStart = mDtStart;
//   } else {
//     tmpStart = eventStart;
//   }
//
//   qCDebug(PIMKOLAB_LOG) << eventEnd.date().toString() << eventEnd.time().toString() << mDtEnd.toString();
//   if ( eventEnd > mDtEnd ) { //event end is after dtEnd
// //     qCDebug(PIMKOLAB_LOG) << "use end";
//     tmpEnd = mDtEnd;
//   } else {
//     tmpEnd = eventEnd;
//   }
//
// //   qCDebug(PIMKOLAB_LOG) << "########## " << tmpStart.isValid();
//   Q_ASSERT(tmpStart.isValid());
//   Q_ASSERT(tmpEnd.isValid());
// //   qCDebug(PIMKOLAB_LOG) << tmpStart.date().toString() << tmpStart.time().toString() << tmpStart.toString();
//
//   FreeBusyPeriod p( tmpStart, tmpEnd );
//   mBusyPeriods.append( p );
//
//   return true;
// }
//
//     KDateTime mDtStart;
//     KDateTime mDtEnd;                  // end datetime
//     FreeBusyPeriod::List mBusyPeriods; // list of periods
//
// };
//
// } // Namespace

namespace Kolab
{
namespace FreebusyUtils
{
static QString createUuid()
{
    const QString uuid = QUuid::createUuid().toString();
    return uuid.mid(1, uuid.size() - 2);
}

Kolab::Period addLocalPeriod(const QDateTime &eventStart, const QDateTime &eventEnd, const QDateTime &mDtStart, const QDateTime &mDtEnd, bool allDay)
{
    QDateTime tmpStart;
    QDateTime tmpEnd;

    // Check to see if the start *or* end of the event is
    // between the start and end of the freebusy dates.
    if (!(((mDtStart <= eventStart) && (eventStart <= mDtEnd)) || ((mDtStart <= eventEnd) && (eventEnd <= mDtEnd)))) {
        qCDebug(PIMKOLAB_LOG) << "event is not within the fb range, skipping";
        return {};
    }

    if (eventStart < mDtStart) { // eventStart is before start
        tmpStart = mDtStart;
    } else {
        tmpStart = eventStart;
    }

    //   qCDebug(PIMKOLAB_LOG) << eventEnd.date().toString() << eventEnd.time().toString() << mDtEnd.toString();
    if (eventEnd > mDtEnd) { // event end is after dtEnd
        tmpEnd = mDtEnd;
    } else {
        tmpEnd = eventEnd;
    }
    Q_ASSERT(tmpStart.isValid());
    Q_ASSERT(tmpEnd.isValid());
    if (allDay) {
        tmpStart.setTime(QTime(0, 0, 0, 0));
        tmpEnd.setTime(QTime(23, 59, 59, 999)); // The window is inclusive
    }
    return Kolab::Period(Kolab::Conversion::fromDate(tmpStart, allDay), Kolab::Conversion::fromDate(tmpEnd, allDay));
}

Freebusy generateFreeBusy(const std::vector<Event> &events, const cDateTime &startDate, const cDateTime &endDate)
{
    QVector<KCalendarCore::Event::Ptr> list;
    list.reserve(events.size());
    for (const Kolab::Event &e : events) {
        list.append(Kolab::Conversion::toKCalendarCore(e));
    }
    KCalendarCore::Person person(QStringLiteral("dummyname"), QStringLiteral("dummyemail"));
    return generateFreeBusy(list, Kolab::Conversion::toDate(startDate), Kolab::Conversion::toDate(endDate), person, startDate.isDateOnly());
}

Freebusy generateFreeBusy(const QVector<KCalendarCore::Event::Ptr> &events,
                          const QDateTime &startDate,
                          const QDateTime &endDate,
                          const KCalendarCore::Person &organizer,
                          bool allDay)
{
    /*
     * TODO the conversion of date-only values to date-time is only necessary because xCal doesn't allow date only. iCalendar doesn't seem to make this
     * restriction so it looks like a bug.
     */
    QDateTime start = startDate.toUTC();
    if (allDay) {
        start.setTime(QTime(0, 0, 0, 0));
    }
    QDateTime end = endDate.toUTC();
    if (allDay) {
        end = end.addDays(1);
        end.setTime(QTime(0, 0, 0, 0)); // The window is inclusive
    }

    // TODO try to merge that with KCalendarCore::Freebusy
    std::vector<Kolab::FreebusyPeriod> freebusyPeriods;
    for (const KCalendarCore::Event::Ptr &event : events) {
        // If this event is transparent it shouldn't be in the freebusy list.
        if (event->transparency() == KCalendarCore::Event::Transparent) {
            continue;
        }

        if (event->hasRecurrenceId()) {
            continue; // TODO apply special period exception (duration could be different)
        }

        const QDateTime eventStart = event->dtStart().toUTC();
        const QDateTime eventEnd = event->dtEnd().toUTC();

        std::vector<Kolab::Period> periods;
        if (event->recurs()) {
            const KCalendarCore::Duration duration(eventStart, eventEnd);
            const auto list = event->recurrence()->timesInInterval(start, end);
            for (const auto &dt : list) {
                const auto utc = dt.toUTC();
                const Kolab::Period &period = addLocalPeriod(utc, duration.end(utc), start, end, allDay);
                if (period.isValid()) {
                    periods.push_back(period);
                }
            }
        } else {
            const Kolab::Period &period = addLocalPeriod(eventStart, eventEnd, start, end, allDay);
            if (period.isValid()) {
                periods.push_back(period);
            }
        }
        if (!periods.empty()) {
            Kolab::FreebusyPeriod period;
            period.setPeriods(periods);
            // TODO get busy type from event (out-of-office, tentative)
            period.setType(Kolab::FreebusyPeriod::Busy);
            period.setEvent(Kolab::Conversion::toStdString(event->uid()),
                            Kolab::Conversion::toStdString(event->summary()),
                            Kolab::Conversion::toStdString(event->location()));
            freebusyPeriods.push_back(period);
        }
    }

    Kolab::Freebusy freebusy;

    freebusy.setStart(Kolab::Conversion::fromDate(start, allDay));
    freebusy.setEnd(Kolab::Conversion::fromDate(end, allDay));
    freebusy.setPeriods(freebusyPeriods);
    freebusy.setUid(createUuid().toStdString());
    freebusy.setTimestamp(Kolab::Conversion::fromDate(QDateTime::currentDateTimeUtc(), false));
    if (!organizer.isEmpty()) {
        freebusy.setOrganizer(ContactReference(Kolab::ContactReference::EmailReference,
                                               Kolab::Conversion::toStdString(organizer.email()),
                                               Kolab::Conversion::toStdString(organizer.name())));
    }

    return freebusy;
}

Freebusy aggregateFreeBusy(const std::vector<Freebusy> &fbList, const std::string &organizerEmail, const std::string &organizerName, bool simple)
{
    std::vector<Kolab::FreebusyPeriod> periods;

    QDateTime start;
    QDateTime end;
    bool allDay = false;
    for (const Freebusy &fb : fbList) {
        const QDateTime &tmpStart = Kolab::Conversion::toDate(fb.start());
        if (!start.isValid() || tmpStart < start) {
            start = tmpStart;
            allDay |= fb.start().isDateOnly();
        }
        const QDateTime &tmpEnd = Kolab::Conversion::toDate(fb.end());
        if (!end.isValid() || tmpEnd > end) {
            end = tmpEnd;
            allDay |= fb.start().isDateOnly();
        }

        const auto fbPeriods{fb.periods()};
        for (const Kolab::FreebusyPeriod &period : fbPeriods) {
            Kolab::FreebusyPeriod simplifiedPeriod;
            simplifiedPeriod.setPeriods(period.periods());
            simplifiedPeriod.setType(period.type());
            if (!simple) { // Don't copy and reset to avoid unintentional information leaking into simple lists
                simplifiedPeriod.setEvent(period.eventSummary(), period.eventUid(), period.eventLocation());
            }
            periods.push_back(simplifiedPeriod);
        }
    }

    Freebusy aggregateFB;

    aggregateFB.setStart(Kolab::Conversion::fromDate(start, allDay));
    aggregateFB.setEnd(Kolab::Conversion::fromDate(end, allDay));
    aggregateFB.setPeriods(periods);
    aggregateFB.setUid(createUuid().toStdString());
    aggregateFB.setTimestamp(Kolab::Conversion::fromDate(QDateTime::currentDateTimeUtc(), false));
    aggregateFB.setOrganizer(ContactReference(Kolab::ContactReference::EmailReference, organizerEmail, organizerName));
    return aggregateFB;
}

std::string toIFB(const Kolab::Freebusy &freebusy)
{
    KCalendarCore::FreeBusy::Ptr fb(new KCalendarCore::FreeBusy(Kolab::Conversion::toDate(freebusy.start()), Kolab::Conversion::toDate(freebusy.end())));
    KCalendarCore::FreeBusyPeriod::List list;
    const auto freePeriods{freebusy.periods()};
    for (const Kolab::FreebusyPeriod &fbPeriod : freePeriods) {
        const auto fbPeriodPeriods{fbPeriod.periods()};
        for (const Kolab::Period &p : fbPeriodPeriods) {
            KCalendarCore::FreeBusyPeriod period(Kolab::Conversion::toDate(p.start), Kolab::Conversion::toDate(p.end));
            //             period.setSummary("summary"); Doesn't even work. X-SUMMARY is read though (just not written out)s
            // TODO
            list.append(period);
        }
    }
    fb->addPeriods(list);

    fb->setUid(QString::fromStdString(freebusy.uid()));
    fb->setOrganizer(KCalendarCore::Person(Conversion::fromStdString(freebusy.organizer().name()), Conversion::fromStdString(freebusy.organizer().email())));
    fb->setLastModified(Kolab::Conversion::toDate(freebusy.timestamp()));

    KCalendarCore::ICalFormat format;
    format.setApplication(QStringLiteral("libkolab"), QStringLiteral(LIBKOLAB_LIB_VERSION_STRING));
    QString data = format.createScheduleMessage(fb, KCalendarCore::iTIPPublish);
    return Conversion::toStdString(data);
}
}
}
