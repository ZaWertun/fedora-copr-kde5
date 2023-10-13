/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <KMime/Message>

namespace Kolab
{
class Attachment;
namespace Mime
{
KMime::Content *findContentByName(const KMime::Message::Ptr &data, const QString &name, QByteArray &type);
KMime::Content *findContentByType(const KMime::Message::Ptr &data, const QByteArray &type);
QList<QByteArray> getContentMimeTypeList(const KMime::Message::Ptr &data);

/**
 * Get Attachments from a Mime message
 *
 * Set the attachments listed in @param attachments on @param incidence from @param mimeData
 */
Kolab::Attachment getAttachment(const std::string &id, const KMime::Message::Ptr &mimeData);
Kolab::Attachment getAttachmentByName(const QString &name, const KMime::Message::Ptr &mimeData);

KMime::Message::Ptr createMessage(const QByteArray &mimetype,
                                  const QByteArray &xKolabType,
                                  const QByteArray &xml,
                                  bool v3,
                                  const QByteArray &productId,
                                  const QByteArray &fromEmail,
                                  const QString &fromName,
                                  const QString &subject);
KMime::Message::Ptr createMessage(const std::string &mimetype,
                                  const std::string &xKolabType,
                                  const std::string &xml,
                                  bool v3,
                                  const std::string &productId,
                                  const std::string &fromEmail,
                                  const std::string &fromName,
                                  const std::string &subject);

/// Generic serializing functions
KMime::Message::Ptr
createMessage(const QString &subject, const QString &mimetype, const QString &xKolabType, const QByteArray &xml, bool v3, const QString &prodid);
KMime::Message::Ptr createMessage(const QByteArray &mimeType, bool v3, const QByteArray &prodid);

KMime::Content *createExplanationPart();
KMime::Content *createMainPart(const QByteArray &mimeType, const QByteArray &decodedContent);
KMime::Content *createAttachmentPart(const QByteArray &cid, const QByteArray &mimeType, const QString &fileName, const QByteArray &decodedContent);
QString fromCid(const QString &cid);
}
} // Namespace
