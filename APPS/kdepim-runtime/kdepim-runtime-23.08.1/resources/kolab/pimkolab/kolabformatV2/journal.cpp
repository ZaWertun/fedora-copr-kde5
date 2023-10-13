/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "journal.h"
#include "libkolab-version.h"
#include "pimkolab_debug.h"

using namespace KolabV2;

KCalendarCore::Journal::Ptr Journal::fromXml(const QDomDocument &xmlDoc, const QString &tz)
{
    Journal journal(tz);
    journal.loadXML(xmlDoc);
    KCalendarCore::Journal::Ptr kcalJournal(new KCalendarCore::Journal());
    journal.saveTo(kcalJournal);
    return kcalJournal;
}

QString Journal::journalToXML(const KCalendarCore::Journal::Ptr &kcalJournal, const QString &tz)
{
    Journal journal(tz, kcalJournal);
    return journal.saveXML();
}

Journal::Journal(const QString &tz, const KCalendarCore::Journal::Ptr &journal)
    : KolabBase(tz)
{
    if (journal) {
        setFields(journal);
    }
}

Journal::~Journal() = default;

void Journal::setSummary(const QString &summary)
{
    mSummary = summary;
}

QString Journal::summary() const
{
    return mSummary;
}

void Journal::setStartDate(const QDateTime &startDate)
{
    mStartDate = startDate;
}

QDateTime Journal::startDate() const
{
    return mStartDate;
}

void Journal::setEndDate(const QDateTime &endDate)
{
    mEndDate = endDate;
}

QDateTime Journal::endDate() const
{
    return mEndDate;
}

bool Journal::loadAttribute(QDomElement &element)
{
    const QString tagName = element.tagName();

    if (tagName == QLatin1String("summary")) {
        setSummary(element.text());
    } else if (tagName == QLatin1String("start-date")) {
        const auto t = element.text();
        setStartDate(stringToDateTime(t));
        mDateOnly = t.size() <= 10;
    } else {
        // Not handled here
        return KolabBase::loadAttribute(element);
    }

    // We handled this
    return true;
}

bool Journal::saveAttributes(QDomElement &element) const
{
    // Save the base class elements
    KolabBase::saveAttributes(element);

    writeString(element, QStringLiteral("summary"), summary());
    if (mDateOnly) {
        writeString(element, QStringLiteral("start-date"), dateToString(startDate().date()));
    } else {
        writeString(element, QStringLiteral("start-date"), dateTimeToString(startDate()));
    }

    return true;
}

bool Journal::loadXML(const QDomDocument &document)
{
    QDomElement top = document.documentElement();

    if (top.tagName() != QLatin1String("journal")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected Journal").arg(top.tagName());
        return false;
    }

    for (QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (!loadAttribute(e)) {
                // Unhandled tag - save for later storage
                // qDebug( "Unhandled tag: %s", e.toCString().data() );
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

QString Journal::saveXML() const
{
    QDomDocument document = domTree();
    QDomElement element = document.createElement(QStringLiteral("journal"));
    element.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    saveAttributes(element);
    document.appendChild(element);
    return document.toString();
}

void Journal::saveTo(const KCalendarCore::Journal::Ptr &journal) const
{
    KolabBase::saveTo(journal);

    journal->setSummary(summary());
    journal->setDtStart(utcToLocal(startDate()));
    journal->setAllDay(mDateOnly);
}

void Journal::setFields(const KCalendarCore::Journal::Ptr &journal)
{
    // Set baseclass fields
    KolabBase::setFields(journal);

    // Set our own fields
    setSummary(journal->summary());
    setStartDate(localToUTC(journal->dtStart()));
}

QString Journal::productID() const
{
    return QLatin1String(LIBKOLAB_LIB_VERSION_STRING) + QLatin1String(", Kolab resource");
}
