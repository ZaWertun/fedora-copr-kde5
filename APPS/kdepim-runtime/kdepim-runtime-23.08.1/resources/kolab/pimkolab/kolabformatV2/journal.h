/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Journal>

#include "kolabbase.h"

class QDomElement;

namespace KolabV2
{
/**
 * This class represents a journal entry, and knows how to load/save it
 * from/to XML, and from/to a KCalendarCore::Journal.
 * The instances of this class are temporary, only used to convert
 * one to the other.
 */
class Journal : public KolabBase
{
public:
    /// Use this to parse an xml string to a journal entry
    /// The caller is responsible for deleting the returned journal
    static KCalendarCore::Journal::Ptr fromXml(const QDomDocument &xmlDoc, const QString &tz);

    /// Use this to get an xml string describing this journal entry
    static QString journalToXML(const KCalendarCore::Journal::Ptr &, const QString &tz);

    explicit Journal(const QString &tz, const KCalendarCore::Journal::Ptr &journal = KCalendarCore::Journal::Ptr());
    ~Journal() override;

    QString type() const override
    {
        return QStringLiteral("Journal");
    }

    void saveTo(const KCalendarCore::Journal::Ptr &journal) const;

    virtual void setSummary(const QString &summary);
    virtual QString summary() const;

    virtual void setStartDate(const QDateTime &startDate);
    virtual QDateTime startDate() const;

    virtual void setEndDate(const QDateTime &endDate);
    virtual QDateTime endDate() const;

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

    // Load this journal by reading the XML file
    bool loadXML(const QDomDocument &xml) override;

    // Serialize this journal to an XML string
    QString saveXML() const override;

protected:
    // Read all known fields from this ical journal
    void setFields(const KCalendarCore::Journal::Ptr &);

    QString productID() const override;

    QString mSummary;
    QDateTime mStartDate;
    QDateTime mEndDate;
    bool mDateOnly = false;
};
}
