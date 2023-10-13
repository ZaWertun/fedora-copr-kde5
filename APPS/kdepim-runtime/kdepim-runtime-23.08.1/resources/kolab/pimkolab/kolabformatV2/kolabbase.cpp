/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabbase.h"
#include "pimkolab_debug.h"

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

using namespace KolabV2;

KolabBase::KolabBase(const QString &tz)
    : mCreationDate(QDateTime::currentDateTime())
    , mLastModified(QDateTime::currentDateTimeUtc())
    , mSensitivity(Public)
{
    // unlike the previously used KTimeZone here, QTimeZone defaults to local time zone if tz.isEmpty()
    // we therefore force it to invalid, which however is unsafe to use (unlike KTimeZone)
    // therefore all usage of mTimeZone must be preceded by a validity check
    mTimeZone = tz.isEmpty() ? QTimeZone() : QTimeZone(tz.toUtf8());
}

KolabBase::~KolabBase() = default;

void KolabBase::setFields(const KCalendarCore::Incidence::Ptr &incidence)
{
    // So far unhandled KCalendarCore::IncidenceBase fields:
    // mPilotID, mSyncStatus, mFloats

    setUid(incidence->uid());
    setBody(incidence->description());
    setCategories(incidence->categoriesStr());
    setCreationDate(localToUTC(incidence->created()));
    setLastModified(incidence->lastModified());
    setSensitivity(static_cast<Sensitivity>(incidence->secrecy()));
    // TODO: Attachments
}

void KolabBase::saveTo(const KCalendarCore::Incidence::Ptr &incidence) const
{
    incidence->setUid(uid());
    incidence->setDescription(body());
    incidence->setCategories(categories());
    incidence->setCreated(utcToLocal(creationDate()));
    incidence->setLastModified(lastModified());
    switch (sensitivity()) {
    case 1:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyPrivate);
        break;
    case 2:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyConfidential);
        break;
    default:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyPublic);
        break;
    }

    // TODO: Attachments
}

void KolabBase::setFields(const KContacts::Addressee *addressee)
{
    // An addressee does not have a creation date, so somehow we should
    // make one, if this is a new entry

    setUid(addressee->uid());
    setBody(addressee->note());
    setCategories(addressee->categories().join(QLatin1Char(',')));

    // Set creation-time and last-modification-time
    const QString creationString = addressee->custom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate"));
    qCDebug(PIMKOLAB_LOG) << "Creation time string:" << creationString;
    QDateTime creationDate;
    if (creationString.isEmpty() && mTimeZone.isValid()) {
        creationDate = QDateTime::currentDateTime().toTimeZone(mTimeZone);
        qCDebug(PIMKOLAB_LOG) << "Creation date set to current time" << mTimeZone;
    } else {
        creationDate = stringToDateTime(creationString);
        qCDebug(PIMKOLAB_LOG) << "Creation date loaded";
    }
    QDateTime modified;
    if (mTimeZone.isValid()) {
        modified = addressee->revision().toTimeZone(mTimeZone);
    }
    if (!modified.isValid()) {
        modified = QDateTime::currentDateTimeUtc();
    }
    setLastModified(modified);
    if (modified < creationDate) {
        // It's not possible that the modification date is earlier than creation
        creationDate = modified;
        qCDebug(PIMKOLAB_LOG) << "Creation date set to modification date";
    }
    setCreationDate(creationDate);
    const QString newCreationDate = dateTimeToString(creationDate);
    if (creationString != newCreationDate) {
        // We modified the creation date, so store it for future reference
        const_cast<KContacts::Addressee *>(addressee)->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate"), newCreationDate);
        qCDebug(PIMKOLAB_LOG) << "Creation date modified. New one:" << newCreationDate;
    }

    switch (addressee->secrecy().type()) {
    case KContacts::Secrecy::Private:
        setSensitivity(Private);
        break;
    case KContacts::Secrecy::Confidential:
        setSensitivity(Confidential);
        break;
    default:
        setSensitivity(Public);
    }

    // TODO: Attachments
}

void KolabBase::saveTo(KContacts::Addressee *addressee) const
{
    addressee->setUid(uid());
    addressee->setNote(body());
    addressee->setCategories(categories().split(QLatin1Char(','), Qt::SkipEmptyParts));
    if (mTimeZone.isValid()) {
        addressee->setRevision(lastModified().toTimeZone(mTimeZone));
    }
    addressee->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("CreationDate"), dateTimeToString(creationDate()));

    switch (sensitivity()) {
    case Private:
        addressee->setSecrecy(KContacts::Secrecy(KContacts::Secrecy::Private));
        break;
    case Confidential:
        addressee->setSecrecy(KContacts::Secrecy(KContacts::Secrecy::Confidential));
        break;
    default:
        addressee->setSecrecy(KContacts::Secrecy(KContacts::Secrecy::Public));
        break;
    }
    // TODO: Attachments
}

void KolabBase::setFields(const KContacts::ContactGroup *contactGroup)
{
    // A contactgroup does not have a creation date, so somehow we should
    // make one, if this is a new entry

    setUid(contactGroup->id());

    // Set creation-time and last-modification-time
    QDateTime creationDate;
    if (mTimeZone.isValid()) {
        creationDate = QDateTime::currentDateTime().toTimeZone(mTimeZone);
    }
    qCDebug(PIMKOLAB_LOG) << "Creation date set to current time";

    QDateTime modified = QDateTime::currentDateTimeUtc();
    setLastModified(modified);
    if (modified < creationDate) {
        // It's not possible that the modification date is earlier than creation
        creationDate = modified;
        qCDebug(PIMKOLAB_LOG) << "Creation date set to modification date";
    }
    setCreationDate(creationDate);
}

void KolabBase::saveTo(KContacts::ContactGroup *contactGroup) const
{
    contactGroup->setId(uid());
}

void KolabBase::setUid(const QString &uid)
{
    mUid = uid;
}

QString KolabBase::uid() const
{
    return mUid;
}

void KolabBase::setBody(const QString &body)
{
    mBody = body;
}

QString KolabBase::body() const
{
    return mBody;
}

void KolabBase::setCategories(const QString &categories)
{
    mCategories = categories;
}

QString KolabBase::categories() const
{
    return mCategories;
}

void KolabBase::setCreationDate(const QDateTime &date)
{
    mCreationDate = date;
}

QDateTime KolabBase::creationDate() const
{
    return mCreationDate;
}

void KolabBase::setLastModified(const QDateTime &date)
{
    mLastModified = date;
}

QDateTime KolabBase::lastModified() const
{
    return mLastModified;
}

void KolabBase::setSensitivity(Sensitivity sensitivity)
{
    mSensitivity = sensitivity;
}

KolabBase::Sensitivity KolabBase::sensitivity() const
{
    return mSensitivity;
}

void KolabBase::setPilotSyncId(unsigned long id)
{
    mHasPilotSyncId = true;
    mPilotSyncId = id;
}

bool KolabBase::hasPilotSyncId() const
{
    return mHasPilotSyncId;
}

unsigned long KolabBase::pilotSyncId() const
{
    return mPilotSyncId;
}

void KolabBase::setPilotSyncStatus(int status)
{
    mHasPilotSyncStatus = true;
    mPilotSyncStatus = status;
}

bool KolabBase::hasPilotSyncStatus() const
{
    return mHasPilotSyncStatus;
}

int KolabBase::pilotSyncStatus() const
{
    return mPilotSyncStatus;
}

bool KolabBase::loadEmailAttribute(QDomElement &element, Email &email)
{
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            const QString tagName = e.tagName();

            if (tagName == QLatin1String("display-name")) {
                email.displayName = e.text();
            } else if (tagName == QLatin1String("smtp-address")) {
                email.smtpAddress = e.text();
            } else {
                // TODO: Unhandled tag - save for later storage
                qCDebug(PIMKOLAB_LOG) << "Warning: Unhandled tag" << e.tagName();
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

void KolabBase::saveEmailAttribute(QDomElement &element, const Email &email, const QString &tagName) const
{
    QDomElement e = element.ownerDocument().createElement(tagName);
    element.appendChild(e);
    writeString(e, QStringLiteral("display-name"), email.displayName);
    writeString(e, QStringLiteral("smtp-address"), email.smtpAddress);
}

bool KolabBase::loadAttribute(QDomElement &element)
{
    const QString tagName = element.tagName();
    switch (tagName[0].toLatin1()) {
    case 'u':
        if (tagName == QLatin1String("uid")) {
            setUid(element.text());
            return true;
        }
        break;
    case 'b':
        if (tagName == QLatin1String("body")) {
            setBody(element.text());
            return true;
        }
        break;
    case 'c':
        if (tagName == QLatin1String("categories")) {
            setCategories(element.text());
            return true;
        }
        if (tagName == QLatin1String("creation-date")) {
            setCreationDate(stringToDateTime(element.text()));
            return true;
        }
        break;
    case 'l':
        if (tagName == QLatin1String("last-modification-date")) {
            setLastModified(stringToDateTime(element.text()));
            return true;
        }
        break;
    case 's':
        if (tagName == QLatin1String("sensitivity")) {
            setSensitivity(stringToSensitivity(element.text()));
            return true;
        }
        break;
    case 'p':
        if (tagName == QLatin1String("product-id")) {
            return true; // ignore this field
        }
        if (tagName == QLatin1String("pilot-sync-id")) {
            setPilotSyncId(element.text().toULong());
            return true;
        }
        if (tagName == QLatin1String("pilot-sync-status")) {
            setPilotSyncStatus(element.text().toInt());
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

bool KolabBase::saveAttributes(QDomElement &element) const
{
    writeString(element, QStringLiteral("product-id"), productID());
    writeString(element, QStringLiteral("uid"), uid());
    writeString(element, QStringLiteral("body"), body());
    writeString(element, QStringLiteral("categories"), categories());
    writeString(element, QStringLiteral("creation-date"), dateTimeToString(creationDate().toUTC()));
    writeString(element, QStringLiteral("last-modification-date"), dateTimeToString(lastModified().toUTC()));
    writeString(element, QStringLiteral("sensitivity"), sensitivityToString(sensitivity()));
    if (hasPilotSyncId()) {
        writeString(element, QStringLiteral("pilot-sync-id"), QString::number(pilotSyncId()));
    }
    if (hasPilotSyncStatus()) {
        writeString(element, QStringLiteral("pilot-sync-status"), QString::number(pilotSyncStatus()));
    }
    return true;
}

bool KolabBase::load(const QString &xml)
{
    const QDomDocument document = loadDocument(xml);
    if (document.isNull()) {
        return false;
    }
    // XML file loaded into tree. Now parse it
    return loadXML(document);
}

QDomDocument KolabBase::loadDocument(const QString &xmlData)
{
    QString errorMsg;
    int errorLine, errorColumn;
    QDomDocument document;
    bool ok = document.setContent(xmlData, &errorMsg, &errorLine, &errorColumn);

    if (!ok) {
        qWarning("Error loading document: %s, line %d, column %d", qPrintable(errorMsg), errorLine, errorColumn);
        return {};
    }

    return document;
}

QDomDocument KolabBase::domTree()
{
    QDomDocument document;

    const QString p = QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"");
    document.appendChild(document.createProcessingInstruction(QStringLiteral("xml"), p));

    return document;
}

QString KolabBase::dateTimeToString(const QDateTime &time)
{
    return time.toString(Qt::ISODate);
}

QString KolabBase::dateToString(QDate date)
{
    return date.toString(Qt::ISODate);
}

QDateTime KolabBase::stringToDateTime(const QString &time)
{
    return QDateTime::fromString(time, Qt::ISODate);
}

QDate KolabBase::stringToDate(const QString &date)
{
    return QDate::fromString(date, Qt::ISODate);
}

QString KolabBase::sensitivityToString(Sensitivity s)
{
    switch (s) {
    case Private:
        return QStringLiteral("private");
    case Confidential:
        return QStringLiteral("confidential");
    case Public:
        return QStringLiteral("public");
    }

    return QStringLiteral("What what what???");
}

KolabBase::Sensitivity KolabBase::stringToSensitivity(const QString &s)
{
    if (s == QLatin1String("private")) {
        return Private;
    }
    if (s == QLatin1String("confidential")) {
        return Confidential;
    }
    return Public;
}

QString KolabBase::colorToString(const QColor &color)
{
    // Color is in the format "#RRGGBB"
    return color.name();
}

QColor KolabBase::stringToColor(const QString &s)
{
    return QColor(s);
}

void KolabBase::writeString(QDomElement &element, const QString &tag, const QString &tagString)
{
    if (!tagString.isEmpty()) {
        QDomElement e = element.ownerDocument().createElement(tag);
        QDomText t = element.ownerDocument().createTextNode(tagString);
        e.appendChild(t);
        element.appendChild(e);
    }
}

QDateTime KolabBase::localToUTC(const QDateTime &time) const
{
    return time.toUTC();
}

QDateTime KolabBase::utcToLocal(const QDateTime &time) const
{
    QDateTime dt = time;
    dt.setTimeSpec(Qt::UTC);
    return dt;
}
