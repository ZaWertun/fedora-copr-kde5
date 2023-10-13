/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "messagehelper.h"
#include "resourcestateinterface.h"

class ImapResourceBase;

struct TaskArguments {
    TaskArguments() = default;

    TaskArguments(const Akonadi::Item &_item)
        : items(Akonadi::Item::List() << _item)
    {
    }

    TaskArguments(const Akonadi::Item &_item, const Akonadi::Collection &_collection)
        : collection(_collection)
        , items(Akonadi::Item::List() << _item)
    {
    }

    TaskArguments(const Akonadi::Item &_item, const QSet<QByteArray> &_parts)
        : items(Akonadi::Item::List() << _item)
        , parts(_parts)
    {
    }

    TaskArguments(const Akonadi::Item::List &_items)
        : items(_items)
    {
    }

    TaskArguments(const Akonadi::Item::List &_items, const QSet<QByteArray> &_addedFlags, const QSet<QByteArray> &_removedFlags)
        : items(_items)
        , addedFlags(_addedFlags)
        , removedFlags(_removedFlags)
    {
    }

    TaskArguments(const Akonadi::Item::List &_items, const Akonadi::Collection &_sourceCollection, const Akonadi::Collection &_targetCollection)
        : items(_items)
        , sourceCollection(_sourceCollection)
        , targetCollection(_targetCollection)
    {
    }

    TaskArguments(const Akonadi::Item::List &_items, const QSet<Akonadi::Tag> &_addedTags, const QSet<Akonadi::Tag> &_removedTags)
        : items(_items)
        , addedTags(_addedTags)
        , removedTags(_removedTags)
    {
    }

    TaskArguments(const Akonadi::Item::List &_items, const Akonadi::Relation::List &_addedRelations, const Akonadi::Relation::List &_removedRelations)
        : items(_items)
        , addedRelations(_addedRelations)
        , removedRelations(_removedRelations)
    {
    }

    TaskArguments(const Akonadi::Collection &_collection)
        : collection(_collection)
    {
    }

    TaskArguments(const Akonadi::Collection &_collection, const Akonadi::Collection &_parentCollection)
        : collection(_collection)
        , parentCollection(_parentCollection)
    {
    }

    TaskArguments(const Akonadi::Collection &_collection, const Akonadi::Collection &_sourceCollection, const Akonadi::Collection &_targetCollection)
        : collection(_collection)
        , sourceCollection(_sourceCollection)
        , targetCollection(_targetCollection)
    {
    }

    TaskArguments(const Akonadi::Collection &_collection, const QSet<QByteArray> &_parts)
        : collection(_collection)
        , parts(_parts)
    {
    }

    TaskArguments(const Akonadi::Tag &_tag)
        : tag(_tag)
    {
    }

    Akonadi::Collection collection;
    Akonadi::Item::List items;
    Akonadi::Collection parentCollection; // only used as parent of a collection
    Akonadi::Collection sourceCollection;
    Akonadi::Collection targetCollection;
    Akonadi::Tag tag;
    QSet<QByteArray> parts;
    QSet<QByteArray> addedFlags;
    QSet<QByteArray> removedFlags;
    QSet<Akonadi::Tag> addedTags;
    QSet<Akonadi::Tag> removedTags;
    Akonadi::Relation::List addedRelations;
    Akonadi::Relation::List removedRelations;
};

class ResourceState : public ResourceStateInterface
{
public:
    explicit ResourceState(ImapResourceBase *resource, const TaskArguments &arguments);

public:
    ~ResourceState() override;

    QString userName() const override;
    QString resourceName() const override;
    QString resourceIdentifier() const override;
    QStringList serverCapabilities() const override;
    QList<KIMAP::MailBoxDescriptor> serverNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> personalNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> userNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> sharedNamespaces() const override;

    bool isAutomaticExpungeEnabled() const override;
    bool isSubscriptionEnabled() const override;
    bool isDisconnectedModeEnabled() const override;
    int intervalCheckTime() const override;

    Akonadi::Collection collection() const override;
    Akonadi::Item item() const override;
    Akonadi::Item::List items() const override;

    Akonadi::Collection parentCollection() const override;

    Akonadi::Collection sourceCollection() const override;
    Akonadi::Collection targetCollection() const override;

    QSet<QByteArray> parts() const override;
    QSet<QByteArray> addedFlags() const override;
    QSet<QByteArray> removedFlags() const override;

    Akonadi::Tag tag() const override;
    QSet<Akonadi::Tag> addedTags() const override;
    QSet<Akonadi::Tag> removedTags() const override;

    Akonadi::Relation::List addedRelations() const override;
    Akonadi::Relation::List removedRelations() const override;

    QString rootRemoteId() const override;

    void setIdleCollection(const Akonadi::Collection &collection) override;
    void applyCollectionChanges(const Akonadi::Collection &collection) override;

    void collectionAttributesRetrieved(const Akonadi::Collection &collection) override;

    void itemRetrieved(const Akonadi::Item &item) override;

    void itemsRetrieved(const Akonadi::Item::List &items) override;
    void itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed) override;
    void itemsRetrievalDone() override;

    void setTotalItems(int) override;

    void itemChangeCommitted(const Akonadi::Item &item) override;
    void itemsChangesCommitted(const Akonadi::Item::List &items) override;

    void collectionsRetrieved(const Akonadi::Collection::List &collections) override;

    void tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &) override;
    void relationsRetrieved(const Akonadi::Relation::List &tags) override;

    void collectionChangeCommitted(const Akonadi::Collection &collection) override;

    void tagChangeCommitted(const Akonadi::Tag &tag) override;

    void changeProcessed() override;

    void searchFinished(const QVector<qint64> &result, bool isRid = true) override;

    void cancelTask(const QString &errorString) override;
    void deferTask() override;
    void restartItemRetrieval(Akonadi::Collection::Id col) override;
    void taskDone() override;

    void emitError(const QString &message) override;
    void emitWarning(const QString &message) override;

    void emitPercent(int percent) override;

    virtual void synchronizeCollection(Akonadi::Collection::Id);
    void synchronizeCollectionTree() override;
    void scheduleConnectionAttempt() override;

    QChar separatorCharacter() const override;
    void setSeparatorCharacter(QChar separator) override;

    void showInformationDialog(const QString &message, const QString &title, const QString &dontShowAgainName) override;

    int batchSize() const override;

    MessageHelper::Ptr messageHelper() const override;

    void setItemMergingMode(Akonadi::ItemSync::MergeMode mergeMode) override;

private:
    ImapResourceBase *const m_resource;
    const TaskArguments m_arguments;
};
