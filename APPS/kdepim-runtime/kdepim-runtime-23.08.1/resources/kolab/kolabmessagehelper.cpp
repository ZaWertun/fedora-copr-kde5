/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabmessagehelper.h"

#include <Akonadi/CollectionAnnotationsAttribute>

#include "kolabhelpers.h"
#include "kolabresource_debug.h"

KolabMessageHelper::KolabMessageHelper(const Akonadi::Collection &col)
    : mCollection(col)
{
}

KolabMessageHelper::~KolabMessageHelper() = default;

Akonadi::Item KolabMessageHelper::createItemFromMessage(const KMime::Message::Ptr &message,
                                                        const qint64 uid,
                                                        const qint64 size,
                                                        const QMap<QByteArray, QVariant> &attrs,
                                                        const QList<QByteArray> &flags,
                                                        const KIMAP::FetchJob::FetchScope &scope,
                                                        bool &ok) const
{
    const Akonadi::Item item = MessageHelper::createItemFromMessage(message, uid, size, attrs, flags, scope, ok);
    if (!ok) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to read imap message";
        return item;
    }
    Kolab::FolderType folderType = Kolab::MailType;
    if (mCollection.hasAttribute<Akonadi::CollectionAnnotationsAttribute>()) {
        const QByteArray folderTypeString =
            KolabHelpers::getFolderTypeAnnotation(mCollection.attribute<Akonadi::CollectionAnnotationsAttribute>()->annotations());
        folderType = KolabHelpers::folderTypeFromString(folderTypeString);
    }
    return KolabHelpers::translateFromImap(folderType, item, ok);
}
