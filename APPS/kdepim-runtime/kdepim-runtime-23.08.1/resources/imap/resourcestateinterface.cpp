/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcestateinterface.h"
#include "imapresource_debug.h"

ResourceStateInterface::~ResourceStateInterface() = default;

QString ResourceStateInterface::mailBoxForCollection(const Akonadi::Collection &collection, bool showWarnings)
{
    if (collection.remoteId().isEmpty()) { // This should never happen, investigate why a collection without remoteId made it this far
        if (showWarnings) {
            qCWarning(IMAPRESOURCE_LOG) << "Got incomplete ancestor chain due to empty remoteId:" << collection;
        }
        return {};
    }

    if (collection.parentCollection() == Akonadi::Collection::root()) {
        /*if ( showWarnings  && collection.remoteId() != rootRemoteId())
          qCWarning(IMAPRESOURCE_LOG) << "RID mismatch, is " << collection.remoteId() << " expected " << rootRemoteId();
        */
        return QLatin1String(""); // see below, this intentionally not just QString()!
    }
    const QString parentMailbox = mailBoxForCollection(collection.parentCollection());
    if (parentMailbox.isNull()) { // invalid, != isEmpty() here!
        return {};
    }

    const QString mailbox = parentMailbox + collection.remoteId();
    if (parentMailbox.isEmpty()) {
        return mailbox.mid(1); // strip of the separator on top-level mailboxes
    }
    return mailbox;
}
