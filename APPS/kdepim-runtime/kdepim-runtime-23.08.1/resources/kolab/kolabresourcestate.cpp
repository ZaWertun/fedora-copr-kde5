/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabresourcestate.h"
#include "kolabhelpers.h"
#include "kolabmessagehelper.h"

#include <imapresource.h>

#include <Akonadi/CachePolicy>
#include <Akonadi/CollectionAnnotationsAttribute>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/MessageParts>
#include <noselectattribute.h>

KolabResourceState::KolabResourceState(ImapResourceBase *resource, const TaskArguments &arguments)
    : ResourceState(resource, arguments)
{
}

static Akonadi::Collection processAnnotations(const Akonadi::Collection &collection)
{
    if (collection.attribute<Akonadi::CollectionAnnotationsAttribute>()) {
        Akonadi::Collection col = collection;
        const QMap<QByteArray, QByteArray> rawAnnotations = col.attribute<Akonadi::CollectionAnnotationsAttribute>()->annotations();
        const QByteArray type = KolabHelpers::getFolderTypeAnnotation(rawAnnotations);
        const Kolab::FolderType folderType = KolabHelpers::folderTypeFromString(type);
        col.setContentMimeTypes(KolabHelpers::getContentMimeTypes(folderType));

        const QString icon = KolabHelpers::getIcon(folderType);
        if (!icon.isEmpty()) {
            // qCDebug(KOLABRESOURCE_LOG) << " setting icon " << icon;
            auto attr = col.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
            attr->setIconName(icon);
        }
        if (folderType != Kolab::MailType) {
            // Groupware data always requires the full message, because it cannot translate without the body
            Akonadi::CachePolicy cachePolicy = col.cachePolicy();
            QStringList localParts = cachePolicy.localParts();
            if (!localParts.contains(QLatin1String(Akonadi::MessagePart::Body))) {
                localParts << QLatin1String(Akonadi::MessagePart::Body);
                cachePolicy.setLocalParts(localParts);
                cachePolicy.setCacheTimeout(-1);
                cachePolicy.setInheritFromParent(false);
                cachePolicy.setSyncOnDemand(true);
                col.setCachePolicy(cachePolicy);
            }
        }
        if (folderType == Kolab::ConfigurationType) {
            // we want to hide this folder from indexing and display, but still have the data available locally.
            col.setEnabled(false);
            col.setShouldList(Akonadi::Collection::ListSync, true);
        }
        if (!KolabHelpers::isHandledType(folderType)) {
            // If we don't handle the folder, make sure we don't download the messages
            col.attribute<NoSelectAttribute>(Akonadi::Collection::AddIfMissing);
        }
        return col;
    }
    return collection;
}

void KolabResourceState::collectionAttributesRetrieved(const Akonadi::Collection &collection)
{
    if (!collection.isValid() && collection.remoteId().isEmpty()) {
        ResourceState::collectionAttributesRetrieved(collection);
        return;
    }
    const Akonadi::Collection col = processAnnotations(collection);
    ResourceState::collectionAttributesRetrieved(col);
}

void KolabResourceState::collectionsRetrieved(const Akonadi::Collection::List &collections)
{
    Akonadi::Collection::List modifiedCollections;
    modifiedCollections.reserve(collections.count());
    for (const Akonadi::Collection &col : collections) {
        modifiedCollections << processAnnotations(col);
    }
    ResourceState::collectionsRetrieved(modifiedCollections);
}

MessageHelper::Ptr KolabResourceState::messageHelper() const
{
    return MessageHelper::Ptr(new KolabMessageHelper(collection()));
}
