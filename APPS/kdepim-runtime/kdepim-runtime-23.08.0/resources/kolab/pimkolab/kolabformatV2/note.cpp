/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "note.h"
#include "libkolab-version.h"
#include "pimkolab_debug.h"

using namespace KolabV2;

KCalendarCore::Journal::Ptr Note::xmlToJournal(const QString &xml)
{
    Note note;
    note.load(xml);
    KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal());
    note.saveTo(journal);
    return journal;
}

QString Note::journalToXML(const KCalendarCore::Journal::Ptr &journal)
{
    Note note(journal);
    return note.saveXML();
}

Note::Note(const KCalendarCore::Journal::Ptr &journal)
{
    if (journal) {
        setFields(journal);
    }
}

Note::~Note() = default;

void Note::setSummary(const QString &summary)
{
    mSummary = summary;
}

QString Note::summary() const
{
    return mSummary;
}

void Note::setBackgroundColor(const QColor &bgColor)
{
    mBackgroundColor = bgColor;
}

QColor Note::backgroundColor() const
{
    return mBackgroundColor;
}

void Note::setForegroundColor(const QColor &fgColor)
{
    mForegroundColor = fgColor;
}

QColor Note::foregroundColor() const
{
    return mForegroundColor;
}

void Note::setRichText(bool richText)
{
    mRichText = richText;
}

bool Note::richText() const
{
    return mRichText;
}

bool Note::loadAttribute(QDomElement &element)
{
    const QString tagName = element.tagName();
    if (tagName == QLatin1String("summary")) {
        setSummary(element.text());
    } else if (tagName == QLatin1String("foreground-color")) {
        setForegroundColor(stringToColor(element.text()));
    } else if (tagName == QLatin1String("background-color")) {
        setBackgroundColor(stringToColor(element.text()));
    } else if (tagName == QLatin1String("knotes-richtext")) {
        mRichText = (element.text() == QLatin1String("true"));
    } else {
        return KolabBase::loadAttribute(element);
    }

    // We handled this
    return true;
}

bool Note::saveAttributes(QDomElement &element) const
{
    // Save the base class elements
    KolabBase::saveAttributes(element);

    // Save the elements
#if 0
    QDomComment c = element.ownerDocument().createComment("Note specific attributes");
    element.appendChild(c);
#endif

    writeString(element, QStringLiteral("summary"), summary());
    if (foregroundColor().isValid()) {
        writeString(element, QStringLiteral("foreground-color"), colorToString(foregroundColor()));
    }
    if (backgroundColor().isValid()) {
        writeString(element, QStringLiteral("background-color"), colorToString(backgroundColor()));
    }
    writeString(element, QStringLiteral("knotes-richtext"), mRichText ? QStringLiteral("true") : QStringLiteral("false"));

    return true;
}

bool Note::loadXML(const QDomDocument &document)
{
    QDomElement top = document.documentElement();

    if (top.tagName() != QLatin1String("note")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected note").arg(top.tagName());
        return false;
    }

    for (QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (!loadAttribute(e)) {
                // TODO: Unhandled tag - save for later storage
                qCDebug(PIMKOLAB_LOG) << "Warning: Unhandled tag" << e.tagName();
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

QString Note::saveXML() const
{
    QDomDocument document = domTree();
    QDomElement element = document.createElement(QStringLiteral("note"));
    element.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    saveAttributes(element);
    document.appendChild(element);
    return document.toString();
}

void Note::setFields(const KCalendarCore::Journal::Ptr &journal)
{
    KolabBase::setFields(journal);

    setSummary(journal->summary());

    QString property = journal->customProperty("KNotes", "BgColor");
    if (!property.isEmpty()) {
        setBackgroundColor(property);
    } else {
        setBackgroundColor(QStringLiteral("yellow"));
    }

    property = journal->customProperty("KNotes", "FgColor");
    if (!property.isEmpty()) {
        setForegroundColor(property);
    } else {
        setForegroundColor(QStringLiteral("black"));
    }

    property = journal->customProperty("KNotes", "RichText");
    if (!property.isEmpty()) {
        setRichText(property == QLatin1String("true") ? true : false);
    } else {
        setRichText(false);
    }
}

void Note::saveTo(const KCalendarCore::Journal::Ptr &journal) const
{
    KolabBase::saveTo(journal);

    // TODO: background and foreground
    journal->setSummary(summary());
    if (foregroundColor().isValid()) {
        journal->setCustomProperty("KNotes", "FgColor", colorToString(foregroundColor()));
    }
    if (backgroundColor().isValid()) {
        journal->setCustomProperty("KNotes", "BgColor", colorToString(backgroundColor()));
    }
    journal->setCustomProperty("KNotes", "RichText", richText() ? QStringLiteral("true") : QStringLiteral("false"));
}

QString Note::productID() const
{
    return QStringLiteral("KNotes %1, Kolab resource").arg(QLatin1String(LIBKOLAB_LIB_VERSION_STRING));
}
