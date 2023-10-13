/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mimeutils.h"
#include "kolabformat/kolabdefinitions.h"
#include <QDateTime>
#include <qdom.h>

#include "libkolab-version.h"
#include "pimkolab_debug.h"
#include <kolabformat.h>
namespace Kolab
{
namespace Mime
{
KMime::Content *findContentByType(const KMime::Message::Ptr &data, const QByteArray &type)
{
    if (type.isEmpty()) {
        qCCritical(PIMKOLAB_LOG) << "Empty type";
        return nullptr;
    }
    Q_ASSERT(!data->contents().isEmpty());
    const auto contents = data->contents();
    for (KMime::Content *c : contents) {
        //         qCDebug(PIMKOLAB_LOG) << c->contentType()->mimeType() << type;
        if (c->contentType()->mimeType() == type) {
            return c;
        }
    }
    return nullptr;
}

KMime::Content *findContentByName(const KMime::Message::Ptr &data, const QString &name, QByteArray &type)
{
    Q_ASSERT(!data->contents().isEmpty());
    const auto contents = data->contents();
    for (KMime::Content *c : contents) {
        //         qCDebug(PIMKOLAB_LOG) << "searching: " << c->contentType()->name().toUtf8();
        if (c->contentType()->name() == name) {
            type = c->contentType()->mimeType();
            return c;
        }
    }
    return nullptr;
}

KMime::Content *findContentById(const KMime::Message::Ptr &data, const QByteArray &id, QByteArray &type, QString &name)
{
    if (id.isEmpty()) {
        qCCritical(PIMKOLAB_LOG) << "looking for empty cid";
        return nullptr;
    }
    Q_ASSERT(!data->contents().isEmpty());
    const auto contents = data->contents();
    for (KMime::Content *c : contents) {
        //         qCDebug(PIMKOLAB_LOG) << "searching: " << c->contentID()->identifier();
        if (c->contentID()->identifier() == id) {
            type = c->contentType()->mimeType();
            name = c->contentType()->name();
            return c;
        }
    }
    return nullptr;
}

QList<QByteArray> getContentMimeTypeList(const KMime::Message::Ptr &data)
{
    QList<QByteArray> typeList;
    Q_ASSERT(!data->contents().isEmpty());
    typeList.reserve(data->contents().count());
    const auto contents = data->contents();
    for (KMime::Content *c : contents) {
        typeList.append(c->contentType()->mimeType());
    }
    return typeList;
}

QString fromCid(const QString &cid)
{
    if (cid.left(4) != QLatin1String("cid:")) { // Don't set if not a cid, happens when serializing format v2
        return {};
    }
    return cid.right(cid.size() - 4);
}

KMime::Message::Ptr createMessage(const QByteArray &mimetype,
                                  const QByteArray &xKolabType,
                                  const QByteArray &xml,
                                  bool v3,
                                  const QByteArray &productId,
                                  const QByteArray &fromEmail,
                                  const QString &fromName,
                                  const QString &subject)
{
    KMime::Message::Ptr message = createMessage(xKolabType, v3, productId);
    message->subject()->fromUnicodeString(subject, "utf-8");
    if (!fromEmail.isEmpty()) {
        KMime::Types::Mailbox mb;
        mb.setName(fromName);
        mb.setAddress(fromEmail);
        message->from()->addAddress(mb);
    }
    message->addContent(createMainPart(mimetype, xml));
    return message;
}

KMime::Message::Ptr createMessage(const std::string &mimetype,
                                  const std::string &xKolabType,
                                  const std::string &xml,
                                  bool v3,
                                  const std::string &productId,
                                  const std::string &fromEmail,
                                  const std::string &fromName,
                                  const std::string &subject)
{
    return createMessage(QByteArray(mimetype.c_str()),
                         QByteArray(xKolabType.c_str()),
                         QByteArray(xml.c_str()),
                         v3,
                         QByteArray(productId.data()),
                         QByteArray(fromEmail.c_str()),
                         QString::fromStdString(fromName),
                         QString::fromStdString(subject));
}

KMime::Message::Ptr
createMessage(const QString &subject, const QString &mimetype, const QString &xKolabType, const QByteArray &xml, bool v3, const QString &prodid)
{
    KMime::Message::Ptr message = createMessage(xKolabType.toLatin1(), v3, prodid.toLatin1());
    if (!subject.isEmpty()) {
        message->subject()->fromUnicodeString(subject, "utf-8");
    }

    KMime::Content *content = createMainPart(mimetype.toLatin1(), xml);
    message->addContent(content);

    message->assemble();
    return message;
}

KMime::Content *createExplanationPart(bool v3)
{
    Q_UNUSED(v3)
    auto content = new KMime::Content();
    content->contentType()->setMimeType("text/plain");
    content->contentType()->setCharset("us-ascii");
    content->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    content->setBody(
        "This is a Kolab Groupware object.\n"
        "To view this object you will need an email client that can understand the Kolab Groupware format.\n"
        "For a list of such email clients please visit\n"
        "http://www.kolab.org/get-kolab\n");
    return content;
}

KMime::Message::Ptr createMessage(const QByteArray &xKolabType, bool v3, const QByteArray &prodid)
{
    KMime::Message::Ptr message(new KMime::Message);
    message->date()->setDateTime(QDateTime::currentDateTimeUtc());
    auto h = new KMime::Headers::Generic(X_KOLAB_TYPE_HEADER);
    h->fromUnicodeString(QString::fromUtf8(xKolabType), "utf-8");
    message->appendHeader(h);
    if (v3) {
        auto hv3 = new KMime::Headers::Generic(X_KOLAB_MIME_VERSION_HEADER);
        hv3->fromUnicodeString(KOLAB_VERSION_V3, "utf-8");
        message->appendHeader(hv3);
    }
    message->userAgent()->from7BitString(prodid);
    message->contentType()->setMimeType("multipart/mixed");
    message->contentType()->setBoundary(KMime::multiPartBoundary());
    message->addContent(createExplanationPart(v3));
    return message;
}

KMime::Content *createMainPart(const QByteArray &mimeType, const QByteArray &decodedContent)
{
    auto content = new KMime::Content();
    content->contentType()->setMimeType(mimeType);
    content->contentType()->setName(KOLAB_OBJECT_FILENAME, "us-ascii");
    content->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
    content->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    content->contentDisposition()->setFilename(KOLAB_OBJECT_FILENAME);
    content->setBody(decodedContent);
    return content;
}

KMime::Content *createAttachmentPart(const QByteArray &cid, const QByteArray &mimeType, const QString &fileName, const QByteArray &base64EncodedContent)
{
    auto content = new KMime::Content();
    if (!cid.isEmpty()) {
        content->contentID()->setIdentifier(cid);
    }
    content->contentType()->setMimeType(mimeType);
    content->contentType()->setName(fileName, "utf-8");
    content->contentTransferEncoding()->setEncoding(KMime::Headers::CEbase64);
    content->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    content->contentDisposition()->setFilename(fileName);
    content->setBody(base64EncodedContent);
    return content;
}

Kolab::Attachment getAttachment(const std::string &id, const KMime::Message::Ptr &mimeData)
{
    if (!QString::fromStdString(id).contains(QLatin1String("cid:"))) {
        qCCritical(PIMKOLAB_LOG) << "not a cid reference";
        return {};
    }
    QByteArray type;
    QString name;
    KMime::Content *content = findContentById(mimeData, fromCid(QString::fromStdString(id)).toLatin1(), type, name);
    if (!content) { // guard against malformed events with non-existent attachments
        qCCritical(PIMKOLAB_LOG) << "could not find attachment: " << name << type;
        return {};
    }
    // Debug() << id << content->decodedContent().toBase64().toStdString();
    Kolab::Attachment attachment;
    attachment.setData(content->decodedContent().toStdString(), type.toStdString());
    attachment.setLabel(name.toStdString());
    return attachment;
}

Kolab::Attachment getAttachmentByName(const QString &name, const KMime::Message::Ptr &mimeData)
{
    QByteArray type;
    KMime::Content *content = findContentByName(mimeData, name, type);
    if (!content) { // guard against malformed events with non-existent attachments
        qCWarning(PIMKOLAB_LOG) << "could not find attachment: " << name.toUtf8() << type;
        return {};
    }
    Kolab::Attachment attachment;
    attachment.setData(content->decodedContent().toStdString(), type.toStdString());
    attachment.setLabel(name.toStdString());
    return attachment;
}
} // Namespace
} // Namespace
