/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "messagehelper.h"

#include <Akonadi/MessageFlags>

#include "resourcetask.h"

#include "imapresource_debug.h"

MessageHelper::~MessageHelper() = default;

Akonadi::Item MessageHelper::createItemFromMessage(const KMime::Message::Ptr &message,
                                                   const qint64 uid,
                                                   const qint64 size,
                                                   const QMap<QByteArray, QVariant> &attrs,
                                                   const QList<QByteArray> &flags,
                                                   const KIMAP::FetchJob::FetchScope &scope,
                                                   bool &ok) const
{
    Q_UNUSED(attrs)

    Akonadi::Item i;
    if (scope.mode == KIMAP::FetchJob::FetchScope::Flags) {
        i.setRemoteId(QString::number(uid));
        i.setMimeType(KMime::Message::mimeType());
        i.setFlags(ResourceTask::toAkonadiFlags(flags));
    } else {
        if (!message) {
            qCWarning(IMAPRESOURCE_LOG) << "Got empty message: " << uid;
            ok = false;
            return {};
        }
        // Sometimes messages might not have a body at all
        if (message->body().isEmpty() && (scope.mode == KIMAP::FetchJob::FetchScope::Full || scope.mode == KIMAP::FetchJob::FetchScope::Content)) {
            // In that case put a space in as body so that it gets cached
            // otherwise we'll wrongly believe the body part is missing from the cache
            message->setBody(" ");
        }
        i.setRemoteId(QString::number(uid));
        i.setMimeType(KMime::Message::mimeType());
        i.setPayload(KMime::Message::Ptr(message));
        i.setSize(size);

        Akonadi::MessageFlags::copyMessageFlags(*message, i);
        const auto flagsLst = ResourceTask::toAkonadiFlags(flags);
        for (const QByteArray &flag : flagsLst) {
            i.setFlag(flag);
        }
    }
    ok = true;
    return i;
}
