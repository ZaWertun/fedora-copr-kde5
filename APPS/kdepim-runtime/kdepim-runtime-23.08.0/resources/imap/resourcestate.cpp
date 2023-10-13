/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcestate.h"

#include "imapaccount.h"
#include "imapresource.h"
#include "sessionpool.h"
#include "settings.h"

#include "imapresource_debug.h"
#include <Akonadi/AgentSearchInterface>
#include <Akonadi/CollectionModifyJob>
#include <KLocalizedString>
#include <KMessageBox>

ResourceState::ResourceState(ImapResourceBase *resource, const TaskArguments &args)
    : m_resource(resource)
    , m_arguments(args)
{
}

ResourceState::~ResourceState() = default;

QString ResourceState::userName() const
{
    return m_resource->m_pool->account()->userName();
}

QString ResourceState::resourceName() const
{
    return m_resource->name();
}

QString ResourceState::resourceIdentifier() const
{
    return m_resource->identifier();
}

QStringList ResourceState::serverCapabilities() const
{
    return m_resource->m_pool->serverCapabilities();
}

QList<KIMAP::MailBoxDescriptor> ResourceState::serverNamespaces() const
{
    return m_resource->m_pool->serverNamespaces();
}

QList<KIMAP::MailBoxDescriptor> ResourceState::personalNamespaces() const
{
    return m_resource->m_pool->serverNamespaces(SessionPool::Personal);
}

QList<KIMAP::MailBoxDescriptor> ResourceState::userNamespaces() const
{
    return m_resource->m_pool->serverNamespaces(SessionPool::User);
}

QList<KIMAP::MailBoxDescriptor> ResourceState::sharedNamespaces() const
{
    return m_resource->m_pool->serverNamespaces(SessionPool::Shared);
}

bool ResourceState::isAutomaticExpungeEnabled() const
{
    return m_resource->settings()->automaticExpungeEnabled();
}

bool ResourceState::isSubscriptionEnabled() const
{
    return m_resource->settings()->subscriptionEnabled();
}

bool ResourceState::isDisconnectedModeEnabled() const
{
    return m_resource->settings()->disconnectedModeEnabled();
}

int ResourceState::intervalCheckTime() const
{
    if (m_resource->settings()->intervalCheckEnabled()) {
        return m_resource->settings()->intervalCheckTime();
    } else {
        return -1; // -1 for never
    }
}

Akonadi::Collection ResourceState::collection() const
{
    return m_arguments.collection;
}

Akonadi::Item ResourceState::item() const
{
    if (m_arguments.items.count() > 1) {
        qCWarning(IMAPRESOURCE_LOG) << "Called item() while state holds multiple items!";
    }

    return m_arguments.items.first();
}

Akonadi::Item::List ResourceState::items() const
{
    return m_arguments.items;
}

Akonadi::Collection ResourceState::parentCollection() const
{
    return m_arguments.parentCollection;
}

Akonadi::Collection ResourceState::sourceCollection() const
{
    return m_arguments.sourceCollection;
}

Akonadi::Collection ResourceState::targetCollection() const
{
    return m_arguments.targetCollection;
}

QSet<QByteArray> ResourceState::parts() const
{
    return m_arguments.parts;
}

QSet<QByteArray> ResourceState::addedFlags() const
{
    return m_arguments.addedFlags;
}

QSet<QByteArray> ResourceState::removedFlags() const
{
    return m_arguments.removedFlags;
}

Akonadi::Tag ResourceState::tag() const
{
    return m_arguments.tag;
}

QSet<Akonadi::Tag> ResourceState::addedTags() const
{
    return m_arguments.addedTags;
}

QSet<Akonadi::Tag> ResourceState::removedTags() const
{
    return m_arguments.removedTags;
}

Akonadi::Relation::List ResourceState::addedRelations() const
{
    return m_arguments.addedRelations;
}

Akonadi::Relation::List ResourceState::removedRelations() const
{
    return m_arguments.removedRelations;
}

QString ResourceState::rootRemoteId() const
{
    return m_resource->settings()->rootRemoteId();
}

void ResourceState::setIdleCollection(const Akonadi::Collection &collection)
{
    QStringList ridPath;

    Akonadi::Collection curCol = collection;
    while (curCol != Akonadi::Collection::root() && !curCol.remoteId().isEmpty()) {
        ridPath.append(curCol.remoteId());
        curCol = curCol.parentCollection();
    }

    m_resource->settings()->setIdleRidPath(ridPath);
    m_resource->settings()->save();
}

void ResourceState::applyCollectionChanges(const Akonadi::Collection &collection)
{
    m_resource->modifyCollection(collection);
}

void ResourceState::collectionAttributesRetrieved(const Akonadi::Collection &collection)
{
    m_resource->collectionAttributesRetrieved(collection);
}

void ResourceState::itemRetrieved(const Akonadi::Item &item)
{
    m_resource->itemRetrieved(item);
}

void ResourceState::itemsRetrieved(const Akonadi::Item::List &items)
{
    m_resource->itemsRetrieved(items);
}

void ResourceState::itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed)
{
    m_resource->itemsRetrievedIncremental(changed, removed);
}

void ResourceState::itemsRetrievalDone()
{
    m_resource->itemsRetrievalDone();
    emitPercent(100);
}

void ResourceState::setTotalItems(int items)
{
    m_resource->setTotalItems(items);
}

void ResourceState::itemChangeCommitted(const Akonadi::Item &item)
{
    m_resource->changeCommitted(item);
}

void ResourceState::itemsChangesCommitted(const Akonadi::Item::List &items)
{
    m_resource->changesCommitted(items);
}

void ResourceState::collectionsRetrieved(const Akonadi::Collection::List &collections)
{
    m_resource->collectionsRetrieved(collections);
    m_resource->startIdleIfNeeded();
}

void ResourceState::collectionChangeCommitted(const Akonadi::Collection &collection)
{
    m_resource->changeCommitted(collection);
}

void ResourceState::tagChangeCommitted(const Akonadi::Tag &tag)
{
    m_resource->changeCommitted(tag);
}

void ResourceState::changeProcessed()
{
    m_resource->changeProcessed();
}

void ResourceState::searchFinished(const QVector<qint64> &result, bool isRid)
{
    m_resource->searchFinished(result, isRid ? Akonadi::AgentSearchInterface::Rid : Akonadi::AgentSearchInterface::Uid);
}

void ResourceState::cancelTask(const QString &errorString)
{
    m_resource->cancelTask(errorString);
}

void ResourceState::deferTask()
{
    m_resource->deferTask();
}

void ResourceState::restartItemRetrieval(Akonadi::Collection::Id col)
{
    // This ensures the collection fetch job is rerun (it isn't when using deferTask)
    // The task will be appended
    // TODO: deferTask should rerun the collectionfetchjob
    m_resource->synchronizeCollection(col);
    cancelTask(i18n("Restarting item retrieval."));
}

void ResourceState::taskDone()
{
    m_resource->taskDone();
}

void ResourceState::emitError(const QString &message)
{
    Q_EMIT m_resource->error(message);
}

void ResourceState::emitWarning(const QString &message)
{
    Q_EMIT m_resource->warning(message);
}

void ResourceState::emitPercent(int percent)
{
    Q_EMIT m_resource->percent(percent);
}

void ResourceState::synchronizeCollection(Akonadi::Collection::Id id)
{
    m_resource->synchronizeCollection(id);
}

void ResourceState::synchronizeCollectionTree()
{
    m_resource->synchronizeCollectionTree();
}

void ResourceState::scheduleConnectionAttempt()
{
    m_resource->scheduleConnectionAttempt();
}

QChar ResourceState::separatorCharacter() const
{
    return m_resource->separatorCharacter();
}

void ResourceState::setSeparatorCharacter(QChar separator)
{
    m_resource->setSeparatorCharacter(separator);
}

void ResourceState::showInformationDialog(const QString &message, const QString &title, const QString &dontShowAgainName)
{
    KMessageBox::information(nullptr, message, title, dontShowAgainName);
}

int ResourceState::batchSize() const
{
    return m_resource->itemSyncBatchSize();
}

MessageHelper::Ptr ResourceState::messageHelper() const
{
    return MessageHelper::Ptr(new MessageHelper());
}

void ResourceState::tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &tagMembers)
{
    m_resource->tagsRetrieved(tags, tagMembers);
}

void ResourceState::relationsRetrieved(const Akonadi::Relation::List &relations)
{
    m_resource->relationsRetrieved(relations);
}

void ResourceState::setItemMergingMode(Akonadi::ItemSync::MergeMode mode)
{
    m_resource->setItemMergingMode(mode);
}
