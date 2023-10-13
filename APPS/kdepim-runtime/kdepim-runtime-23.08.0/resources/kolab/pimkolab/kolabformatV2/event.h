/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidence.h"

#include <KCalendarCore/Event>

class QDomElement;

namespace KolabV2
{
/**
 * This class represents an event, and knows how to load/save it
 * from/to XML, and from/to a KCalendarCore::Event.
 * The instances of this class are temporary, only used to convert
 * one to the other.
 */
class Event : public Incidence
{
public:
    /// Use this to parse an xml string to a event entry
    /// The caller is responsible for deleting the returned event
    static KCalendarCore::Event::Ptr fromXml(const QDomDocument &xmlDoc, const QString &tz);

    /// Use this to get an xml string describing this event entry
    static QString eventToXML(const KCalendarCore::Event::Ptr &, const QString &tz);

    /// Create a event object and
    explicit Event(const QString &tz, const KCalendarCore::Event::Ptr &event = KCalendarCore::Event::Ptr());
    ~Event() override;

    void saveTo(const KCalendarCore::Event::Ptr &event);

    QString type() const override
    {
        return QStringLiteral("Event");
    }

    virtual void setTransparency(KCalendarCore::Event::Transparency transparency);
    virtual KCalendarCore::Event::Transparency transparency() const;

    virtual void setEndDate(const QDateTime &date);
    virtual void setEndDate(const QDate &date);
    virtual void setEndDate(const QString &date);
    virtual QDateTime endDate() const;

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

    // Load this event by reading the XML file
    bool loadXML(const QDomDocument &xml) override;

    // Serialize this event to an XML string
    QString saveXML() const override;

protected:
    // Read all known fields from this ical incidence
    void setFields(const KCalendarCore::Event::Ptr &);

    KCalendarCore::Event::Transparency mShowTimeAs;
    QDateTime mEndDate;
    bool mHasEndDate;
};
}
