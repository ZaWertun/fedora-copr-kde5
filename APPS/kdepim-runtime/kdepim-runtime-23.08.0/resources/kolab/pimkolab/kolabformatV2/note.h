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
 * This class represents a note, and knows how to load/save it
 * from/to XML, and from/to a KCalendarCore::Journal.
 * The instances of this class are temporary, only used to convert
 * one to the other.
 */
class Note : public KolabBase
{
public:
    /// Use this to parse an xml string to a journal entry
    /// The caller is responsible for deleting the returned journal
    static KCalendarCore::Journal::Ptr xmlToJournal(const QString &xml);

    /// Use this to get an xml string describing this journal entry
    static QString journalToXML(const KCalendarCore::Journal::Ptr &);

    /// Create a note object and
    explicit Note(const KCalendarCore::Journal::Ptr &journal = KCalendarCore::Journal::Ptr());
    ~Note() override;

    void saveTo(const KCalendarCore::Journal::Ptr &journal) const;

    QString type() const override
    {
        return QStringLiteral("Note");
    }

    virtual void setSummary(const QString &summary);
    virtual QString summary() const;

    virtual void setBackgroundColor(const QColor &bgColor);
    virtual QColor backgroundColor() const;

    virtual void setForegroundColor(const QColor &fgColor);
    virtual QColor foregroundColor() const;

    virtual void setRichText(bool richText);
    virtual bool richText() const;

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

    // Load this note by reading the XML file
    bool loadXML(const QDomDocument &xml) override;

    // Serialize this note to an XML string
    QString saveXML() const override;

protected:
    // Read all known fields from this ical incidence
    void setFields(const KCalendarCore::Journal::Ptr &);

    // Save all known fields into this ical incidence
    void saveTo(const KCalendarCore::Incidence::Ptr &) const;

    QString productID() const override;

    QString mSummary;
    QColor mBackgroundColor;
    QColor mForegroundColor;
    bool mRichText = false;
};
}
