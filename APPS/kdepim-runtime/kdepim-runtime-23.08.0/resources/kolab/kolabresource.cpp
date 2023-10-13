/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabresource.h"

#include "kolabresource_debug.h"
#include "kolabresource_trace.h"
#include "sessionpool.h"
#include "sessionuiproxy.h"
#include "settingspasswordrequester.h"
#include "setupserver.h"

#include <Akonadi/BlockAlarmsAttribute>

#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionColorAttribute>

#include <Akonadi/CollectionAnnotationsAttribute>
#include <changecollectiontask.h>
#include <resourcestateinterface.h>
#include <retrieveitemstask.h>

#include <KLocalizedString>
#include <KWindowSystem>
#include <QIcon>

#include "kolabaddtagtask.h"
#include "kolabchangeitemsrelationstask.h"
#include "kolabchangeitemstagstask.h"
#include "kolabchangetagtask.h"
#include "kolabhelpers.h"
#include "kolabremovetagtask.h"
#include "kolabresourcestate.h"
#include "kolabretrievecollectionstask.h"
#include "kolabretrievetagstask.h"
#include "kolabsettings.h"

KolabResource::KolabResource(const QString &id)
    : ImapResourceBase(id)
{
    m_pool->setPasswordRequester(new SettingsPasswordRequester(this, m_pool));
    m_pool->setSessionUiProxy(SessionUiProxy::Ptr(new SessionUiProxy));
    m_pool->setClientId(clientId());

    Akonadi::AttributeFactory::registerAttribute<Akonadi::CollectionColorAttribute>();
    // Ensure we have up-to date metadata before attempting to sync folder
    setScheduleAttributeSyncBeforeItemSync(true);
    setKeepLocalCollectionChanges(QSet<QByteArray>() << "ENTITYDISPLAY" << Akonadi::BlockAlarmsAttribute().type());

    settings(); // make sure the D-Bus settings interface is up
}

KolabResource::~KolabResource() = default;

Settings *KolabResource::settings() const
{
    if (!m_settings) {
        m_settings = new KolabSettings;
    }

    return m_settings;
}

void KolabResource::delayedInit()
{
    ImapResourceBase::delayedInit();
    settings()->setRetrieveMetadataOnFolderListing(false);
    Q_ASSERT(!settings()->retrieveMetadataOnFolderListing());
}

QString KolabResource::defaultName() const
{
    return i18n("Kolab Resource");
}

QByteArray KolabResource::clientId() const
{
    return QByteArrayLiteral("Kontact Kolab Resource 5/KOLAB");
}

QDialog *KolabResource::createConfigureDialog(WId windowId)
{
    auto dlg = new SetupServer(this, windowId);
    dlg->setAttribute(Qt::WA_NativeWindow, true);
    KWindowSystem::setMainWindow(dlg->windowHandle(), windowId);
    dlg->setWindowTitle(i18nc("@title:window", "Kolab Account Settings"));
    dlg->setWindowIcon(QIcon::fromTheme(QStringLiteral("kolab")));
    connect(dlg, &QDialog::finished, this, &KolabResource::onConfigurationDone);
    return dlg;
}

void KolabResource::onConfigurationDone(int result)
{
    auto dlg = qobject_cast<SetupServer *>(sender());
    if (result) {
        if (dlg->shouldClearCache()) {
            clearCache();
        }
        settings()->save();
    }
    dlg->deleteLater();
}

ResourceStateInterface::Ptr KolabResource::createResourceState(const TaskArguments &args)
{
    return ResourceStateInterface::Ptr(new KolabResourceState(this, args));
}

void KolabResource::retrieveCollections()
{
    qCDebug(KOLABRESOURCE_TRACE);
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Retrieving folders"));

    startTask(new KolabRetrieveCollectionsTask(createResourceState(TaskArguments()), this));
    synchronizeTags();
    synchronizeRelations();
}

void KolabResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    qCDebug(KOLABRESOURCE_TRACE) << item.id() << collection.id();
    bool ok = true;
    const Akonadi::Item imapItem = KolabHelpers::translateToImap(item, ok);
    if (!ok) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to convert item";
        cancelTask();
        return;
    }
    ImapResourceBase::itemAdded(imapItem, collection);
}

void KolabResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    qCDebug(KOLABRESOURCE_TRACE) << item.id() << parts;
    bool ok = true;
    const Akonadi::Item imapItem = KolabHelpers::translateToImap(item, ok);
    if (!ok) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to convert item";
        cancelTask();
        return;
    }
    ImapResourceBase::itemChanged(imapItem, parts);
}

void KolabResource::itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &source, const Akonadi::Collection &destination)
{
    qCDebug(KOLABRESOURCE_TRACE) << items.size() << source.id() << destination.id();
    bool ok = true;
    const Akonadi::Item::List imapItems = KolabHelpers::translateToImap(items, ok);
    if (!ok) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to convert item";
        cancelTask();
        return;
    }
    ImapResourceBase::itemsMoved(imapItems, source, destination);
}

static Akonadi::Collection updateAnnotations(const Akonadi::Collection &collection)
{
    qCDebug(KOLABRESOURCE_TRACE) << collection.id();
    // Set the annotations on new folders
    const QByteArray kolabType = KolabHelpers::kolabTypeForMimeType(collection.contentMimeTypes());
    Akonadi::Collection col = collection;
    auto attr = col.attribute<Akonadi::CollectionAnnotationsAttribute>(Akonadi::Collection::AddIfMissing);
    QMap<QByteArray, QByteArray> annotations = attr->annotations();

    bool changed = false;
    auto colorAttribute = col.attribute<Akonadi::CollectionColorAttribute>();
    if (colorAttribute) {
        const QColor color = colorAttribute->color();
        if (color.isValid()) {
            KolabHelpers::setFolderColor(annotations, color);
            changed = true;
        }
    }

    if (!kolabType.isEmpty()) {
        KolabHelpers::setFolderTypeAnnotation(annotations, kolabType);
        changed = true;
    }

    if (changed) {
        attr->setAnnotations(annotations);
        return col;
    }
    return collection;
}

void KolabResource::collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent)
{
    qCDebug(KOLABRESOURCE_TRACE) << collection.id() << parent.id();
    // Set the annotations on new folders
    const Akonadi::Collection col = updateAnnotations(collection);
    // TODO we need to save the collections as well if the annotations have changed
    // or we simply don't have the annotations locally, which perhaps is also not required?
    ImapResourceBase::collectionAdded(col, parent);
}

void KolabResource::collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &parts)
{
    qCDebug(KOLABRESOURCE_TRACE) << collection.id() << parts;
    QSet<QByteArray> p = parts;
    // Update annotations if necessary
    // FIXME col ?????
    const Akonadi::Collection col = updateAnnotations(collection);
    if (parts.contains(Akonadi::CollectionColorAttribute().type())) {
        p << Akonadi::CollectionAnnotationsAttribute().type();
    }

    // TODO we need to save the collections as well if the annotations have changed
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Updating folder '%1'", collection.name()));
    auto task = new ChangeCollectionTask(createResourceState(TaskArguments(collection, p)), this);
    task->syncEnabledState(true);
    startTask(task);
}

void KolabResource::tagAdded(const Akonadi::Tag &tag)
{
    qCDebug(KOLABRESOURCE_TRACE) << tag.id();
    auto task = new KolabAddTagTask(createResourceState(TaskArguments(tag)), this);
    startTask(task);
}

void KolabResource::tagChanged(const Akonadi::Tag &tag)
{
    qCDebug(KOLABRESOURCE_TRACE) << tag.id();
    auto task = new KolabChangeTagTask(createResourceState(TaskArguments(tag)), QSharedPointer<TagConverter>(new TagConverter), this);
    startTask(task);
}

void KolabResource::tagRemoved(const Akonadi::Tag &tag)
{
    qCDebug(KOLABRESOURCE_TRACE) << tag.id();
    auto task = new KolabRemoveTagTask(createResourceState(TaskArguments(tag)), this);
    startTask(task);
}

void KolabResource::itemsTagsChanged(const Akonadi::Item::List &items, const QSet<Akonadi::Tag> &addedTags, const QSet<Akonadi::Tag> &removedTags)
{
    qCDebug(KOLABRESOURCE_TRACE) << items.size() << addedTags.size() << removedTags.size();
    auto task =
        new KolabChangeItemsTagsTask(createResourceState(TaskArguments(items, addedTags, removedTags)), QSharedPointer<TagConverter>(new TagConverter), this);
    startTask(task);
}

void KolabResource::retrieveTags()
{
    qCDebug(KOLABRESOURCE_TRACE);
    auto task = new KolabRetrieveTagTask(createResourceState(TaskArguments()), KolabRetrieveTagTask::RetrieveTags, this);
    startTask(task);
}

void KolabResource::retrieveRelations()
{
    qCDebug(KOLABRESOURCE_TRACE);
    auto task = new KolabRetrieveTagTask(createResourceState(TaskArguments()), KolabRetrieveTagTask::RetrieveRelations, this);
    startTask(task);
}

void KolabResource::itemsRelationsChanged(const Akonadi::Item::List &items,
                                          const Akonadi::Relation::List &addedRelations,
                                          const Akonadi::Relation::List &removedRelations)
{
    qCDebug(KOLABRESOURCE_TRACE) << items.size() << addedRelations.size() << removedRelations.size();
    auto task = new KolabChangeItemsRelationsTask(createResourceState(TaskArguments(items, addedRelations, removedRelations)));
    startTask(task);
}

AKONADI_RESOURCE_MAIN(KolabResource)
