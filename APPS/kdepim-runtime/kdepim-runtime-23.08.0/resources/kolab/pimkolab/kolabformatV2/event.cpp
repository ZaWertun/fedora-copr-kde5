/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"
#include "pimkolab_debug.h"

using namespace KolabV2;

KCalendarCore::Event::Ptr Event::fromXml(const QDomDocument &xmlDoc, const QString &tz)
{
    Event event(tz);
    event.loadXML(xmlDoc);
    KCalendarCore::Event::Ptr kcalEvent(new KCalendarCore::Event());
    event.saveTo(kcalEvent);
    return kcalEvent;
}

QString Event::eventToXML(const KCalendarCore::Event::Ptr &kcalEvent, const QString &tz)
{
    Event event(tz, kcalEvent);
    return event.saveXML();
}

Event::Event(const QString &tz, const KCalendarCore::Event::Ptr &event)
    : Incidence(tz, event)
    , mShowTimeAs(KCalendarCore::Event::Opaque)
    , mHasEndDate(false)
{
    if (event) {
        setFields(event);
    }
}

Event::~Event() = default;

void Event::setTransparency(KCalendarCore::Event::Transparency transparency)
{
    mShowTimeAs = transparency;
}

KCalendarCore::Event::Transparency Event::transparency() const
{
    return mShowTimeAs;
}

void Event::setEndDate(const QDateTime &date)
{
    mEndDate = date;
    mHasEndDate = true;
    if (mFloatingStatus == AllDay) {
        qCDebug(PIMKOLAB_LOG) << "ERROR: Time on end date but no time on the event";
    }
    mFloatingStatus = HasTime;
}

void Event::setEndDate(const QDate &date)
{
    mEndDate = QDateTime(date, QTime());
    mHasEndDate = true;
    if (mFloatingStatus == HasTime) {
        qCDebug(PIMKOLAB_LOG) << "ERROR: No time on end date but time on the event";
    }
    mFloatingStatus = AllDay;
}

void Event::setEndDate(const QString &endDate)
{
    if (endDate.length() > 10) {
        // This is a date + time
        setEndDate(stringToDateTime(endDate));
    } else {
        // This is only a date
        setEndDate(stringToDate(endDate));
    }
}

QDateTime Event::endDate() const
{
    return mEndDate;
}

bool Event::loadAttribute(QDomElement &element)
{
    // This method doesn't handle the color-label tag yet
    QString tagName = element.tagName();

    if (tagName == QLatin1String("show-time-as")) {
        // TODO: Support tentative and outofoffice
        if (element.text() == QLatin1String("free")) {
            setTransparency(KCalendarCore::Event::Transparent);
        } else {
            setTransparency(KCalendarCore::Event::Opaque);
        }
    } else if (tagName == QLatin1String("end-date")) {
        setEndDate(element.text());
    } else {
        return Incidence::loadAttribute(element);
    }

    // We handled this
    return true;
}

bool Event::saveAttributes(QDomElement &element) const
{
    // Save the base class elements
    Incidence::saveAttributes(element);

    // TODO: Support tentative and outofoffice
    if (transparency() == KCalendarCore::Event::Transparent) {
        writeString(element, QStringLiteral("show-time-as"), QStringLiteral("free"));
    } else {
        writeString(element, QStringLiteral("show-time-as"), QStringLiteral("busy"));
    }
    if (mHasEndDate) {
        if (mFloatingStatus == HasTime) {
            writeString(element, QStringLiteral("end-date"), dateTimeToString(endDate()));
        } else {
            writeString(element, QStringLiteral("end-date"), dateToString(endDate().date()));
        }
    }

    return true;
}

bool Event::loadXML(const QDomDocument &document)
{
    QDomElement top = document.documentElement();

    if (top.tagName() != QLatin1String("event")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected event").arg(top.tagName());
        return false;
    }

    for (QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            loadAttribute(e);
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

QString Event::saveXML() const
{
    QDomDocument document = domTree();
    QDomElement element = document.createElement(QStringLiteral("event"));
    element.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    saveAttributes(element);
    document.appendChild(element);
    return document.toString();
}

void Event::setFields(const KCalendarCore::Event::Ptr &event)
{
    Incidence::setFields(event);

    // note: if hasEndDate() is false and hasDuration() is true
    // dtEnd() returns start+duration
    if (event->hasEndDate() || event->hasDuration()) {
        if (event->allDay()) {
            // This is an all-day event. Don't timezone move this one
            mFloatingStatus = AllDay;
            setEndDate(event->dtEnd().date());
        } else {
            mFloatingStatus = HasTime;
            setEndDate(localToUTC(event->dtEnd()));
        }
    } else {
        mHasEndDate = false;
    }
    setTransparency(event->transparency());
}

void Event::saveTo(const KCalendarCore::Event::Ptr &event)
{
    Incidence::saveTo(event);

    // PORT KF5 ? method removed event->setHasEndDate( mHasEndDate );
    if (mHasEndDate) {
        if (mFloatingStatus == AllDay) {
            // This is an all-day event. Don't timezone move this one
            event->setDtEnd(endDate());
        } else {
            event->setDtEnd(utcToLocal(endDate()));
        }
    }
    event->setTransparency(transparency());
}
