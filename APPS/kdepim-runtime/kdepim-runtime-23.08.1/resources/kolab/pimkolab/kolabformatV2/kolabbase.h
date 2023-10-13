/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Incidence>

#include <QColor>
#include <QDateTime>
#include <QTimeZone>
#include <qdom.h>

namespace KContacts
{
class Addressee;
class ContactGroup;
}

namespace KolabV2
{
class KolabBase
{
public:
    struct Email {
    public:
        Email(const QString &name = QString(), const QString &email = QString())
            : displayName(name)
            , smtpAddress(email)
        {
        }

        QString displayName;
        QString smtpAddress;
    };

    enum Sensitivity {
        Public = 0,
        Private = 1,
        Confidential = 2,
    };

    explicit KolabBase(const QString &time_zone = QString());
    virtual ~KolabBase();

    // Return a string identifying this type
    virtual QString type() const = 0;

    virtual void setUid(const QString &uid);
    virtual QString uid() const;

    virtual void setBody(const QString &body);
    virtual QString body() const;

    virtual void setCategories(const QString &categories);
    virtual QString categories() const;

    virtual void setCreationDate(const QDateTime &date);
    virtual QDateTime creationDate() const;

    virtual void setLastModified(const QDateTime &date);
    virtual QDateTime lastModified() const;

    virtual void setSensitivity(Sensitivity sensitivity);
    virtual Sensitivity sensitivity() const;

    virtual void setPilotSyncId(unsigned long id);
    virtual bool hasPilotSyncId() const;
    virtual unsigned long pilotSyncId() const;

    virtual void setPilotSyncStatus(int status);
    virtual bool hasPilotSyncStatus() const;
    virtual int pilotSyncStatus() const;

    // String - Date conversion methods
    static QString dateTimeToString(const QDateTime &time);
    static QString dateToString(QDate date);
    static QDateTime stringToDateTime(const QString &time);
    static QDate stringToDate(const QString &date);

    // String - Sensitivity conversion methods
    static QString sensitivityToString(Sensitivity);
    static Sensitivity stringToSensitivity(const QString &);

    // String - Color conversion methods
    static QString colorToString(const QColor &);
    static QColor stringToColor(const QString &);

    // Load this object by reading the XML file
    bool load(const QString &xml);
    static QDomDocument loadDocument(const QString &xmlData);

    // Load this QDomDocument
    virtual bool loadXML(const QDomDocument &xml) = 0;

    // Serialize this object to an XML string
    virtual QString saveXML() const = 0;

protected:
    /// Read all known fields from this ical incidence
    void setFields(const KCalendarCore::Incidence::Ptr &);

    /// Save all known fields into this ical incidence
    void saveTo(const KCalendarCore::Incidence::Ptr &) const;

    /// Read all known fields from this contact
    void setFields(const KContacts::Addressee *);

    /// Save all known fields into this contact
    void saveTo(KContacts::Addressee *) const;

    /// Read all known fields from this contact group
    void setFields(const KContacts::ContactGroup *);

    /// Save all known fields into this contact group
    void saveTo(KContacts::ContactGroup *) const;

    // This just makes the initial dom tree with version and doctype
    static QDomDocument domTree();

    bool loadEmailAttribute(QDomElement &element, Email &email);

    void saveEmailAttribute(QDomElement &element, const Email &email, const QString &tagName = QStringLiteral("email")) const;

    // Load the attributes of this class
    virtual bool loadAttribute(QDomElement &);

    // Save the attributes of this class
    virtual bool saveAttributes(QDomElement &) const;

    // Return the product ID
    virtual QString productID() const = 0;

    // Write a string tag
    static void writeString(QDomElement &, const QString &, const QString &);

    QDateTime localToUTC(const QDateTime &time) const;
    QDateTime utcToLocal(const QDateTime &time) const;

    QString mUid;
    QString mBody;
    QString mCategories;
    QDateTime mCreationDate;
    QDateTime mLastModified;
    Sensitivity mSensitivity;
    QTimeZone mTimeZone;

    // KPilot synchronization stuff
    bool mHasPilotSyncId = false;
    bool mHasPilotSyncStatus = false;
    unsigned long mPilotSyncId;
    int mPilotSyncStatus;
};
}
