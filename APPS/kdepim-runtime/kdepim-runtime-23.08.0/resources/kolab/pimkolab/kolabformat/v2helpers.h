/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolabdefinitions.h"

#include "kolabformat/errorhandler.h"
#include "kolabformatV2/contact.h"
#include "kolabformatV2/distributionlist.h"
#include "kolabformatV2/event.h"
#include "kolabformatV2/journal.h"
#include "kolabformatV2/kolabbase.h"
#include "kolabformatV2/note.h"
#include "kolabformatV2/task.h"
#include "mime/mimeutils.h"

#include <KCalendarCore/Incidence>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <KMime/Message>

#include <Akonadi/NoteUtils>

#include <qdom.h>

namespace Kolab
{
/*
 * Parse XML, create KCalendarCore container and extract attachments
 */
template<typename KCalPtr, typename Container>
static KCalPtr fromXML(const QByteArray &xmlData, QStringList &attachments)
{
    const QDomDocument xmlDoc = KolabV2::KolabBase::loadDocument(QString::fromUtf8(xmlData)); // TODO extract function from V2 format
    if (xmlDoc.isNull()) {
        Critical() << "Failed to read the xml document";
        return KCalPtr();
    }
    const KCalPtr i = Container::fromXml(xmlDoc, QString()); // For parsing we don't need the timezone, so we don't set one
    Q_ASSERT(i);
    const QDomNodeList nodes = xmlDoc.elementsByTagName(QStringLiteral("inline-attachment"));
    for (int i = 0; i < nodes.size(); i++) {
        attachments.append(nodes.at(i).toElement().text());
    }
    return i;
}

void getAttachments(KCalendarCore::Incidence::Ptr incidence, const QStringList &attachments, const KMime::Message::Ptr &mimeData);

template<typename IncidencePtr, typename Converter>
static inline IncidencePtr incidenceFromKolabImpl(const KMime::Message::Ptr &data, const QByteArray &mimetype, const QString &timezoneId)
{
    Q_UNUSED(timezoneId)
    KMime::Content *xmlContent = Mime::findContentByType(data, mimetype);
    if (!xmlContent) {
        Critical() << "couldn't find part";
        return IncidencePtr();
    }
    const QByteArray &xmlData = xmlContent->decodedContent();

    QStringList attachments;
    IncidencePtr ptr = fromXML<IncidencePtr, Converter>(xmlData, attachments); // TODO do we care about timezone?
    getAttachments(ptr, attachments, data);

    return ptr;
}

KContacts::Addressee addresseeFromKolab(const QByteArray &xmlData, const KMime::Message::Ptr &data);
KContacts::Addressee addresseeFromKolab(const QByteArray &xmlData, QString &pictureAttachmentName, QString &logoAttachmentName, QString &soundAttachmentName);

KMime::Message::Ptr contactToKolabFormat(const KolabV2::Contact &contact, const QString &productId);

KContacts::ContactGroup contactGroupFromKolab(const QByteArray &xmlData);

KMime::Message::Ptr distListToKolabFormat(const KolabV2::DistributionList &distList, const QString &productId);
KMime::Message::Ptr noteFromKolab(const QByteArray &xmlData, const QDateTime &creationDate);

KMime::Message::Ptr noteToKolab(const KMime::Message::Ptr &msg, const QString &productId);
QByteArray noteToKolabXML(const KMime::Message::Ptr &msg);

QStringList readLegacyDictionaryConfiguration(const QByteArray &xmlData, QString &language);
}
