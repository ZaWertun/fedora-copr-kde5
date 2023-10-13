/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Incidence>

#include "kolabbase.h"

class QDomElement;

namespace KolabV2
{
/**
 * This abstract class represents an incidence which has the shared
 * fields, of events and tasks and knows how to load/save these
 * from/to XML, and from/to a KCalendarCore::Incidence.
 */
class Incidence : public KolabBase
{
public:
    struct Recurrence {
        QString cycle;
        QString type;
        int interval;
        QStringList days; // list of days-of-the-week
        QString dayNumber;
        QString month;
        QString rangeType;
        QString range; // date or number or nothing
        QList<QDate> exclusions;
    };

    struct Attendee : Email {
        Attendee()
            : requestResponse(true)
            , invitationSent(false)
        {
        }

        QString status;
        bool requestResponse;
        bool invitationSent;
        QString role;
        QString delegate;
        QString delegator;
    };

    explicit Incidence(const QString &tz, const KCalendarCore::Incidence::Ptr &incidence = KCalendarCore::Incidence::Ptr());

public:
    ~Incidence() override;

    void saveTo(const KCalendarCore::Incidence::Ptr &incidence);

    virtual void setPriority(int priority);
    virtual int priority() const;

    virtual void setSummary(const QString &summary);
    virtual QString summary() const;

    virtual void setLocation(const QString &location);
    virtual QString location() const;

    virtual void setOrganizer(const Email &organizer);
    virtual Email organizer() const;

    virtual void setStartDate(const QDateTime &startDate);
    virtual void setStartDate(const QDate &startDate);
    virtual void setStartDate(const QString &startDate);
    virtual QDateTime startDate() const;

    virtual void setAlarm(float alarm);
    virtual float alarm() const;

    virtual void setRecurrence(KCalendarCore::Recurrence *recur);
    virtual Recurrence recurrence() const;

    virtual void addAttendee(const Attendee &attendee);
    QList<Attendee> &attendees();
    const QList<Attendee> &attendees() const;

    QString type() const override
    {
        return QStringLiteral("Incidence");
    }

    /**
     * The internal uid is used as the uid inside KOrganizer whenever
     * two or more events with the same uid appear, which KOrganizer
     * can't handle. To avoid keep that internal uid from changing all the
     * time, it is persisted in the XML between a save and the next load.
     */
    void setInternalUID(const QString &iuid);
    QString internalUID() const;

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

protected:
    enum FloatingStatus {
        Unset,
        AllDay,
        HasTime,
    };

    // Read all known fields from this ical incidence
    void setFields(const KCalendarCore::Incidence::Ptr &);

    bool loadAttendeeAttribute(QDomElement &, Attendee &);
    void saveAttendeeAttribute(QDomElement &element, const Attendee &attendee) const;
    void saveAttendees(QDomElement &element) const;
    void saveAttachments(QDomElement &element) const;

    void loadAlarms(const QDomElement &element);
    void saveAlarms(QDomElement &element) const;

    void loadRecurrence(const QDomElement &element);
    void saveRecurrence(QDomElement &element) const;
    void saveCustomAttributes(QDomElement &element) const;
    void loadCustomAttributes(QDomElement &element);

    QString productID() const override;

    QString mSummary;
    QString mLocation;
    Email mOrganizer;
    QDateTime mStartDate;
    FloatingStatus mFloatingStatus = Unset;
    float mAlarm = 0.0;
    bool mHasAlarm = false;
    Recurrence mRecurrence;
    QList<Attendee> mAttendees;
    QList<KCalendarCore::Alarm::Ptr> mAlarms;
    QList<KCalendarCore::Attachment> mAttachments;
    QString mInternalUID;

    struct Custom {
        QByteArray key;
        QString value;
    };
    QList<Custom> mCustomList;

    // This is the KCal priority, not the Kolab priority.
    // See kcalPriorityToKolab() and kolabPrioritytoKCal().
    int mPriority = 0;
};
}
