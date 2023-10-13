/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "v2helpers.h"

#include "pimkolab_debug.h"

#include <QBuffer>

namespace Kolab
{
void getAttachments(KCalendarCore::Incidence::Ptr incidence, const QStringList &attachments, const KMime::Message::Ptr &mimeData)
{
    if (!incidence) {
        qCCritical(PIMKOLAB_LOG) << "Invalid incidence";
        return;
    }
    for (const QString &name : attachments) {
        QByteArray type;
        KMime::Content *content = Mime::findContentByName(mimeData, name, type);
        if (!content) { // guard against malformed events with non-existent attachments
            qCWarning(PIMKOLAB_LOG) << "could not find attachment: " << name.toUtf8() << type;
            continue;
        }
        const QByteArray c = content->decodedContent().toBase64();
        KCalendarCore::Attachment attachment(c, QString::fromLatin1(type));
        attachment.setLabel(name);
        incidence->addAttachment(attachment);
        qCDebug(PIMKOLAB_LOG) << "ATTACHMENT NAME" << name << type;
    }
}

static QImage getPicture(const QString &pictureAttachmentName, const KMime::Message::Ptr &data, QByteArray &type)
{
    if (!data) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    KMime::Content *imgContent = Mime::findContentByName(data, pictureAttachmentName /*"kolab-picture.png"*/, type);
    if (!imgContent) {
        qCWarning(PIMKOLAB_LOG) << "could not find picture: " << pictureAttachmentName;
        return {};
    }
    QByteArray imgData = imgContent->decodedContent();
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::ReadOnly);
    QImage image;
    bool success = false;
    if (type == "image/jpeg") {
        success = image.load(&buffer, "JPEG");
        // FIXME I tried getting the code to interpret the picture as PNG, but the VCard implementation writes it as JPEG anyways...
        //         if (success) {
        //             QByteArray pic;
        //             QBuffer b(&pic);
        //             b.open(QIODevice::ReadWrite);
        //             Q_ASSERT(image.save(&b, "PNG"));
        //             b.close();
        //             Debug() << pic.toBase64();
        //             QBuffer b2(&pic);
        //             b2.open(QIODevice::ReadOnly);
        //             success = image.load(&b2, "PNG");
        //             b2.close();
        //             Q_ASSERT(success);
        //         }
    } else {
        type = "image/png";
        success = image.load(&buffer, "PNG");
    }
    buffer.close();
    if (!success) {
        qCWarning(PIMKOLAB_LOG) << "failed to load picture";
    }
    return image;
}

KContacts::Addressee addresseeFromKolab(const QByteArray &xmlData, const KMime::Message::Ptr &data)
{
    if (!data) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    KContacts::Addressee addressee;
    //     qCDebug(PIMKOLAB_LOG) << "xmlData " << xmlData;
    KolabV2::Contact contact(QString::fromUtf8(xmlData));
    QByteArray type;
    const QString &pictureAttachmentName = contact.pictureAttachmentName();
    if (!pictureAttachmentName.isEmpty()) {
        const QImage &img = getPicture(pictureAttachmentName, data, type);
        contact.setPicture(img, QString::fromLatin1(type));
    }

    const QString &logoAttachmentName = contact.logoAttachmentName();
    if (!logoAttachmentName.isEmpty()) {
        contact.setLogo(getPicture(logoAttachmentName, data, type), QString::fromLatin1(type));
    }

    const QString &soundAttachmentName = contact.soundAttachmentName();
    if (!soundAttachmentName.isEmpty()) {
        KMime::Content *content = Mime::findContentByName(data, soundAttachmentName /*"sound"*/, type);
        if (content) {
            const QByteArray &sData = content->decodedContent();
            contact.setSound(sData);
        } else {
            qCWarning(PIMKOLAB_LOG) << "could not find sound: " << soundAttachmentName;
        }
    }
    contact.saveTo(&addressee);
    return addressee;
}

KContacts::Addressee addresseeFromKolab(const QByteArray &xmlData, QString &pictureAttachmentName, QString &logoAttachmentName, QString &soundAttachmentName)
{
    KContacts::Addressee addressee;
    KolabV2::Contact contact(QString::fromUtf8(xmlData));
    pictureAttachmentName = contact.pictureAttachmentName();
    logoAttachmentName = contact.logoAttachmentName();
    soundAttachmentName = contact.soundAttachmentName();
    contact.saveTo(&addressee);
    return addressee;
}

static QByteArray createPicture(const QImage &img, const QString & /*format*/, QByteArray &type)
{
    QByteArray pic;
    QBuffer buffer(&pic);
    buffer.open(QIODevice::WriteOnly);
    type = "image/png";
    // FIXME it's not possible to save jpegs lossless, so we always use png. otherwise we would compress the image on every write.
    //     if (format == "image/jpeg") {
    //         type = "image/jpeg";
    //         img.save(&buffer, "JPEG");
    //     } else {
    img.save(&buffer, "PNG");
    //     }
    buffer.close();
    return pic;
}

KMime::Message::Ptr contactToKolabFormat(const KolabV2::Contact &contact, const QString &productId)
{
    KMime::Message::Ptr message = Mime::createMessage(QByteArray(KOLAB_TYPE_CONTACT), false, productId.toLatin1());
    if (!message) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    message->subject()->fromUnicodeString(contact.uid(), "utf-8");
    message->from()->fromUnicodeString(contact.fullEmail(), "utf-8");

    KMime::Content *content = Mime::createMainPart(KOLAB_TYPE_CONTACT, contact.saveXML().toUtf8());
    message->addContent(content);

    if (!contact.picture().isNull()) {
        QByteArray type;
        const QByteArray &pic = createPicture(contact.picture(), contact.pictureFormat(), type);
        content = Mime::createAttachmentPart(QByteArray(), type, /*"kolab-picture.png"*/ contact.pictureAttachmentName(), pic);
        message->addContent(content);
    }

    if (!contact.logo().isNull()) {
        QByteArray type;
        const QByteArray &pic = createPicture(contact.logo(), contact.logoFormat(), type);
        content = Mime::createAttachmentPart(QByteArray(), type, /*"kolab-logo.png"*/ contact.logoAttachmentName(), pic);
        message->addContent(content);
    }

    if (!contact.sound().isEmpty()) {
        content = Mime::createAttachmentPart(QByteArray(), "audio/unknown", /*"sound"*/ contact.soundAttachmentName(), contact.sound());
        message->addContent(content);
    }

    message->assemble();
    return message;
}

KContacts::ContactGroup contactGroupFromKolab(const QByteArray &xmlData)
{
    KContacts::ContactGroup contactGroup;
    //     qCDebug(PIMKOLAB_LOG) << "xmlData " << xmlData;
    KolabV2::DistributionList distList(QString::fromUtf8(xmlData));
    distList.saveTo(&contactGroup);
    return contactGroup;
}

KMime::Message::Ptr distListToKolabFormat(const KolabV2::DistributionList &distList, const QString &productId)
{
    KMime::Message::Ptr message = Mime::createMessage(KOLAB_TYPE_DISTLIST_V2, false, productId.toLatin1());
    if (!message) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    message->subject()->fromUnicodeString(distList.uid(), "utf-8");
    message->from()->fromUnicodeString(distList.uid(), "utf-8");

    KMime::Content *content = Mime::createMainPart(KOLAB_TYPE_DISTLIST_V2, distList.saveXML().toUtf8());
    message->addContent(content);

    message->assemble();
    return message;
}

KMime::Message::Ptr noteFromKolab(const QByteArray &xmlData, const QDateTime &creationDate)
{
    KolabV2::Note j;
    if (!j.load(QString::fromUtf8(xmlData))) {
        qCWarning(PIMKOLAB_LOG) << "failed to read note";
        return {};
    }

    Akonadi::NoteUtils::NoteMessageWrapper note;
    note.setTitle(j.summary());
    note.setText(j.body());
    note.setFrom(QStringLiteral("kolab@kde4"));
    note.setCreationDate(creationDate);
    return note.message();
}

KMime::Message::Ptr noteToKolab(const KMime::Message::Ptr &msg, const QString &productId)
{
    if (!msg) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    Akonadi::NoteUtils::NoteMessageWrapper note(msg);
    return Mime::createMessage(note.title(), QStringLiteral(KOLAB_TYPE_NOTE), QStringLiteral(KOLAB_TYPE_NOTE), noteToKolabXML(msg), false, productId);
}

QByteArray noteToKolabXML(const KMime::Message::Ptr &msg)
{
    if (!msg) {
        qCCritical(PIMKOLAB_LOG) << "empty message";
        return {};
    }
    Akonadi::NoteUtils::NoteMessageWrapper note(msg);
    KolabV2::Note j;
    j.setSummary(note.title());
    j.setBody(note.text());
    return j.saveXML().toUtf8();
}

QStringList readLegacyDictionaryConfiguration(const QByteArray &xmlData, QString &language)
{
    QStringList dictionary;
    const QDomDocument xmlDoc = KolabV2::KolabBase::loadDocument(QString::fromUtf8(xmlData)); // TODO extract function from V2 format
    if (xmlDoc.isNull()) {
        qCCritical(PIMKOLAB_LOG) << "Failed to read the xml document";
        return {};
    }

    QDomElement top = xmlDoc.documentElement();

    if (top.tagName() != QLatin1String("configuration")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected configuration").arg(top.tagName());
        return {};
    }

    for (QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment() || !n.isElement()) {
            continue;
        }
        const QDomElement e = n.toElement();
        const QString tagName = e.tagName();
        if (tagName == QLatin1String("language")) {
            language = e.text();
        } else if (tagName == QLatin1Char('e')) {
            dictionary.append(e.text());
        }
    }
    return dictionary;
}
}
