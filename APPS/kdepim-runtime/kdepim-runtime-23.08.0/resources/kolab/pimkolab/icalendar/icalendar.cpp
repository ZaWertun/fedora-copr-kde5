/*

    SPDX-FileCopyrightText: 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "icalendar.h"
#include "imip.h"
#include "libkolab-version.h"
#include "pimkolab_debug.h"

#include <KMime/Message>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>
#include <conversion/commonconversion.h>
#include <conversion/kcalconversion.h>
#include <mime/mimeutils.h>

#include <KLocalizedString>

#include <QTimeZone>

#include <iostream>

namespace Kolab
{
std::string toICal(const std::vector<Event> &events)
{
    KCalendarCore::Calendar::Ptr calendar(new KCalendarCore::MemoryCalendar(Kolab::Conversion::getTimeSpec(true, std::string())));
    for (const Event &event : events) {
        KCalendarCore::Event::Ptr kcalEvent = Conversion::toKCalendarCore(event);
        kcalEvent->setCreated(QDateTime::currentDateTimeUtc()); // sets dtstamp
        calendar->addEvent(kcalEvent);
    }
    KCalendarCore::ICalFormat format;
    format.setApplication(QStringLiteral("libkolab"), QStringLiteral(LIBKOLAB_LIB_VERSION_STRING));
    //     qCDebug(PIMKOLAB_LOG) << format.createScheduleMessage(calendar->events().first(), KCalendarCore::iTIPRequest);

    return Conversion::toStdString(format.toString(calendar));
}

std::vector<Event> fromICalEvents(const std::string &input)
{
    KCalendarCore::Calendar::Ptr calendar(new KCalendarCore::MemoryCalendar(Kolab::Conversion::getTimeSpec(true, std::string())));
    KCalendarCore::ICalFormat format;
    format.setApplication(QStringLiteral("libkolab"), QStringLiteral(LIBKOLAB_LIB_VERSION_STRING));
    format.fromString(calendar, Conversion::fromStdString(input));
    std::vector<Event> events;
    const auto eventsList = calendar->events();
    for (const KCalendarCore::Event::Ptr &event : eventsList) {
        events.push_back(Conversion::fromKCalendarCore(*event));
    }
    return events;
}

ITipHandler::ITipHandler()
    : mMethod(iTIPNoMethod)
{
}

ITipHandler::ITipMethod mapFromKCalendarCore(KCalendarCore::iTIPMethod method)
{
    Q_ASSERT((int)KCalendarCore::iTIPPublish == (int)ITipHandler::iTIPPublish);
    Q_ASSERT((int)KCalendarCore::iTIPNoMethod == (int)ITipHandler::iTIPNoMethod);
    return static_cast<ITipHandler::ITipMethod>(method);
}

KCalendarCore::iTIPMethod mapToKCalendarCore(ITipHandler::ITipMethod method)
{
    Q_ASSERT((int)KCalendarCore::iTIPPublish == (int)ITipHandler::iTIPPublish);
    Q_ASSERT((int)KCalendarCore::iTIPNoMethod == (int)ITipHandler::iTIPNoMethod);
    return static_cast<KCalendarCore::iTIPMethod>(method);
}

std::string ITipHandler::toITip(const Event &event, ITipHandler::ITipMethod method) const
{
    KCalendarCore::ICalFormat format;
    format.setApplication(QStringLiteral("libkolab"), QStringLiteral(LIBKOLAB_LIB_VERSION_STRING));
    KCalendarCore::iTIPMethod m = mapToKCalendarCore(method);
    if (m == KCalendarCore::iTIPNoMethod) {
        return {};
    }
    //     qCDebug(PIMKOLAB_LOG) << event.start().
    /* TODO
     * DTSTAMP is created
     * CREATED is current timestamp
     * LASTMODIFIED is lastModified
     *
     * Double check if that is correct.
     *
     * I think DTSTAMP should be the current timestamp, and CREATED should be the creation date.
     */
    KCalendarCore::Event::Ptr e = Conversion::toKCalendarCore(event);
    return Conversion::toStdString(format.createScheduleMessage(e, m));
}

std::vector<Event> ITipHandler::fromITip(const std::string &string)
{
    KCalendarCore::Calendar::Ptr calendar(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
    KCalendarCore::ICalFormat format;
    KCalendarCore::ScheduleMessage::Ptr msg = format.parseScheduleMessage(calendar, Conversion::fromStdString(string));
    KCalendarCore::Event::Ptr event = msg->event().dynamicCast<KCalendarCore::Event>();
    std::vector<Event> events;
    events.push_back(Conversion::fromKCalendarCore(*event));
    mMethod = mapFromKCalendarCore(msg->method());
    return events;
}

ITipHandler::ITipMethod ITipHandler::method() const
{
    return mMethod;
}

std::string ITipHandler::toIMip(const Event &event, ITipHandler::ITipMethod m, const std::string &from, bool bccMe) const
{
    KCalendarCore::Event::Ptr e = Conversion::toKCalendarCore(event);
    //     e->recurrence()->addRDateTime(e->dtStart()); //FIXME The createScheduleMessage converts everything to utc without a recurrence.
    KCalendarCore::ICalFormat format;
    format.setApplication(QStringLiteral("libkolab"), QStringLiteral(LIBKOLAB_LIB_VERSION_STRING));
    KCalendarCore::iTIPMethod method = mapToKCalendarCore(m);
    const QString &messageText = format.createScheduleMessage(e, method);
    // This code is mostly from MailScheduler::performTransaction
    if (method == KCalendarCore::iTIPRequest || method == KCalendarCore::iTIPCancel || method == KCalendarCore::iTIPAdd
        || method == KCalendarCore::iTIPDeclineCounter) {
        return Conversion::toStdString(QString::fromUtf8(mailAttendees(e, bccMe, messageText)));
    } else {
        QString subject;
        if (e && method == KCalendarCore::iTIPCounter) {
            subject = i18n("Counter proposal: %1", e->summary());
        }
        return Conversion::toStdString(QString::fromUtf8(mailOrganizer(e, Conversion::fromStdString(from), bccMe, messageText, subject)));
    }
}

std::vector<Event> ITipHandler::fromIMip(const std::string &input)
{
    KMime::Message::Ptr msg = KMime::Message::Ptr(new KMime::Message);
    msg->setContent(Conversion::fromStdString(input).toUtf8());
    msg->parse();
    msg->content(KMime::ContentIndex());

    KMime::Content *c = Kolab::Mime::findContentByType(msg, "text/calendar");
    if (!c) {
        qCWarning(PIMKOLAB_LOG) << "could not find text/calendar part";
        return {};
    }
    return fromITip(Conversion::toStdString(QString::fromUtf8(c->decodedContent())));
}
}
