/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dummyresourcestate.h"
Q_DECLARE_METATYPE(QList<qint64>)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE(QVector<qint64>)
#endif
Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(TagListAndMembers)

DummyResourceState::DummyResourceState()
    : m_automaticExpunge(true)
    , m_subscriptionEnabled(true)
    , m_disconnectedMode(true)
    , m_intervalCheckTime(-1)
    , m_mergeMode(Akonadi::ItemSync::RIDMerge)
{
    qRegisterMetaType<QList<qint64>>();
    qRegisterMetaType<QVector<qint64>>();
    qRegisterMetaType<TagListAndMembers>();
}

DummyResourceState::~DummyResourceState() = default;

void DummyResourceState::setUserName(const QString &name)
{
    m_userName = name;
}

QString DummyResourceState::userName() const
{
    return m_userName;
}

void DummyResourceState::setResourceName(const QString &name)
{
    m_resourceName = name;
}

QString DummyResourceState::resourceName() const
{
    return m_resourceName;
}

void DummyResourceState::setResourceIdentifier(const QString &identifier)
{
    m_resourceIdentifier = identifier;
}

QString DummyResourceState::resourceIdentifier() const
{
    return m_resourceIdentifier;
}

void DummyResourceState::setServerCapabilities(const QStringList &capabilities)
{
    m_capabilities = capabilities;
}

QStringList DummyResourceState::serverCapabilities() const
{
    return m_capabilities;
}

void DummyResourceState::setServerNamespaces(const QList<KIMAP::MailBoxDescriptor> &namespaces)
{
    m_namespaces = namespaces;
}

QList<KIMAP::MailBoxDescriptor> DummyResourceState::serverNamespaces() const
{
    return m_namespaces;
}

QList<KIMAP::MailBoxDescriptor> DummyResourceState::personalNamespaces() const
{
    return {};
}

QList<KIMAP::MailBoxDescriptor> DummyResourceState::userNamespaces() const
{
    return {};
}

QList<KIMAP::MailBoxDescriptor> DummyResourceState::sharedNamespaces() const
{
    return {};
}

void DummyResourceState::setAutomaticExpungeEnagled(bool enabled)
{
    m_automaticExpunge = enabled;
}

bool DummyResourceState::isAutomaticExpungeEnabled() const
{
    return m_automaticExpunge;
}

void DummyResourceState::setSubscriptionEnabled(bool enabled)
{
    m_subscriptionEnabled = enabled;
}

bool DummyResourceState::isSubscriptionEnabled() const
{
    return m_subscriptionEnabled;
}

void DummyResourceState::setDisconnectedModeEnabled(bool enabled)
{
    m_disconnectedMode = enabled;
}

bool DummyResourceState::isDisconnectedModeEnabled() const
{
    return m_disconnectedMode;
}

void DummyResourceState::setIntervalCheckTime(int interval)
{
    m_intervalCheckTime = interval;
}

int DummyResourceState::intervalCheckTime() const
{
    return m_intervalCheckTime;
}

void DummyResourceState::setCollection(const Akonadi::Collection &collection)
{
    m_collection = collection;
}

Akonadi::Collection DummyResourceState::collection() const
{
    return m_collection;
}

void DummyResourceState::setItem(const Akonadi::Item &item)
{
    m_items.clear();
    m_items << item;
}

Akonadi::Item DummyResourceState::item() const
{
    return m_items.first();
}

Akonadi::Item::List DummyResourceState::items() const
{
    return m_items;
}

void DummyResourceState::setParentCollection(const Akonadi::Collection &collection)
{
    m_parentCollection = collection;
}

Akonadi::Collection DummyResourceState::parentCollection() const
{
    return m_parentCollection;
}

void DummyResourceState::setSourceCollection(const Akonadi::Collection &collection)
{
    m_sourceCollection = collection;
}

Akonadi::Collection DummyResourceState::sourceCollection() const
{
    return m_sourceCollection;
}

void DummyResourceState::setTargetCollection(const Akonadi::Collection &collection)
{
    m_targetCollection = collection;
}

Akonadi::Collection DummyResourceState::targetCollection() const
{
    return m_targetCollection;
}

void DummyResourceState::setParts(const QSet<QByteArray> &parts)
{
    m_parts = parts;
}

QSet<QByteArray> DummyResourceState::parts() const
{
    return m_parts;
}

void DummyResourceState::setTag(const Akonadi::Tag &tag)
{
    m_tag = tag;
}

Akonadi::Tag DummyResourceState::tag() const
{
    return m_tag;
}

void DummyResourceState::setAddedTags(const QSet<Akonadi::Tag> &addedTags)
{
    m_addedTags = addedTags;
}

QSet<Akonadi::Tag> DummyResourceState::addedTags() const
{
    return m_addedTags;
}

void DummyResourceState::setRemovedTags(const QSet<Akonadi::Tag> &removedTags)
{
    m_removedTags = removedTags;
}

QSet<Akonadi::Tag> DummyResourceState::removedTags() const
{
    return m_removedTags;
}

Akonadi::Relation::List DummyResourceState::addedRelations() const
{
    return {};
}

Akonadi::Relation::List DummyResourceState::removedRelations() const
{
    return {};
}

QString DummyResourceState::rootRemoteId() const
{
    return QStringLiteral("root-id");
}

void DummyResourceState::setIdleCollection(const Akonadi::Collection &collection)
{
    recordCall("setIdleCollection", QVariant::fromValue(collection));
}

void DummyResourceState::applyCollectionChanges(const Akonadi::Collection &collection)
{
    recordCall("applyCollectionChanges", QVariant::fromValue(collection));
}

void DummyResourceState::collectionAttributesRetrieved(const Akonadi::Collection &collection)
{
    recordCall("collectionAttributesRetrieved", QVariant::fromValue(collection));
}

void DummyResourceState::itemRetrieved(const Akonadi::Item &item)
{
    recordCall("itemRetrieved", QVariant::fromValue(item));
}

void DummyResourceState::itemsRetrieved(const Akonadi::Item::List &items)
{
    recordCall("itemsRetrieved", QVariant::fromValue(items));
}

void DummyResourceState::itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed)
{
    Q_UNUSED(removed)

    recordCall("itemsRetrievedIncremental", QVariant::fromValue(changed));
}

void DummyResourceState::itemsRetrievalDone()
{
    recordCall("itemsRetrievalDone");
}

void DummyResourceState::setTotalItems(int)
{
}

QSet<QByteArray> DummyResourceState::addedFlags() const
{
    return {};
}

QSet<QByteArray> DummyResourceState::removedFlags() const
{
    return {};
}

void DummyResourceState::itemChangeCommitted(const Akonadi::Item &item)
{
    recordCall("itemChangeCommitted", QVariant::fromValue(item));
}

void DummyResourceState::itemsChangesCommitted(const Akonadi::Item::List &items)
{
    recordCall("itemsChangesCommitted", QVariant::fromValue(items));
}

void DummyResourceState::collectionsRetrieved(const Akonadi::Collection::List &collections)
{
    recordCall("collectionsRetrieved", QVariant::fromValue(collections));
}

void DummyResourceState::collectionChangeCommitted(const Akonadi::Collection &collection)
{
    recordCall("collectionChangeCommitted", QVariant::fromValue(collection));
}

void DummyResourceState::tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &items)
{
    recordCall("tagsRetrieved", QVariant::fromValue(qMakePair(tags, items)));
}

void DummyResourceState::relationsRetrieved(const Akonadi::Relation::List &relations)
{
    recordCall("relationsRetrieved", QVariant::fromValue(relations));
}

void DummyResourceState::tagChangeCommitted(const Akonadi::Tag &tag)
{
    recordCall("tagChangeCommitted", QVariant::fromValue(tag));
}

void DummyResourceState::changeProcessed()
{
    recordCall("changeProcessed");
}

void DummyResourceState::searchFinished(const QVector<qint64> &result, bool isRid)
{
    Q_UNUSED(isRid)
    recordCall("searchFinished", QVariant::fromValue(result));
}

void DummyResourceState::cancelTask(const QString &errorString)
{
    recordCall("cancelTask", QVariant::fromValue(errorString));
}

void DummyResourceState::deferTask()
{
    recordCall("deferTask");
}

void DummyResourceState::restartItemRetrieval(Akonadi::Collection::Id col)
{
    recordCall("restartItemRetrieval", QVariant::fromValue(col));
}

void DummyResourceState::taskDone()
{
    recordCall("taskDone");
}

void DummyResourceState::emitError(const QString &message)
{
    recordCall("emitError", QVariant::fromValue(message));
}

void DummyResourceState::emitWarning(const QString &message)
{
    recordCall("emitWarning", QVariant::fromValue(message));
}

void DummyResourceState::emitPercent(int percent)
{
    Q_UNUSED(percent)
    // FIXME: Many tests need to be updated for this to be uncommented out.
    // recordCall( "emitPercent", QVariant::fromValue(percent) );
}

void DummyResourceState::synchronizeCollectionTree()
{
    recordCall("synchronizeCollectionTree");
}

void DummyResourceState::scheduleConnectionAttempt()
{
    recordCall("scheduleConnectionAttempt");
}

void DummyResourceState::showInformationDialog(const QString &message, const QString &, const QString &)
{
    recordCall("showInformationDialog", QVariant::fromValue(message));
}

QList<QPair<QByteArray, QVariant>> DummyResourceState::calls() const
{
    return m_calls;
}

QChar DummyResourceState::separatorCharacter() const
{
    return m_separator;
}

void DummyResourceState::setSeparatorCharacter(QChar separator)
{
    m_separator = separator;
}

void DummyResourceState::recordCall(const QByteArray &callName, const QVariant &parameter)
{
    m_calls << QPair<QByteArray, QVariant>(callName, parameter);
}

int DummyResourceState::batchSize() const
{
    return 10;
}

void DummyResourceState::setItemMergingMode(Akonadi::ItemSync::MergeMode mergeMode)
{
    m_mergeMode = mergeMode;
}

MessageHelper::Ptr DummyResourceState::messageHelper() const
{
    return MessageHelper::Ptr(new MessageHelper());
}
