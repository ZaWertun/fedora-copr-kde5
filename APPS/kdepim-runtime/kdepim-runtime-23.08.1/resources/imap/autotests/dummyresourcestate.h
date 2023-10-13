/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QPair>
#include <QVariant>

#include "resourcestateinterface.h"

using TagListAndMembers = QPair<Akonadi::Tag::List, QHash<QString, Akonadi::Item::List>>;

class DummyResourceState : public ResourceStateInterface
{
public:
    using Ptr = QSharedPointer<DummyResourceState>;

    explicit DummyResourceState();
    ~DummyResourceState() override;

    void setUserName(const QString &name);
    QString userName() const override;

    void setResourceName(const QString &name);
    QString resourceName() const override;

    void setResourceIdentifier(const QString &identifier);
    QString resourceIdentifier() const override;

    void setServerCapabilities(const QStringList &capabilities);
    QStringList serverCapabilities() const override;

    void setServerNamespaces(const QList<KIMAP::MailBoxDescriptor> &namespaces);
    QList<KIMAP::MailBoxDescriptor> serverNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> personalNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> userNamespaces() const override;
    QList<KIMAP::MailBoxDescriptor> sharedNamespaces() const override;

    void setAutomaticExpungeEnagled(bool enabled);
    bool isAutomaticExpungeEnabled() const override;

    void setSubscriptionEnabled(bool enabled);
    bool isSubscriptionEnabled() const override;
    void setDisconnectedModeEnabled(bool enabled);
    bool isDisconnectedModeEnabled() const override;
    void setIntervalCheckTime(int interval);
    int intervalCheckTime() const override;

    void setCollection(const Akonadi::Collection &collection);
    Akonadi::Collection collection() const override;
    void setItem(const Akonadi::Item &item);
    Akonadi::Item item() const override;
    Akonadi::Item::List items() const override;

    void setParentCollection(const Akonadi::Collection &collection);
    Akonadi::Collection parentCollection() const override;

    void setSourceCollection(const Akonadi::Collection &collection);
    Akonadi::Collection sourceCollection() const override;
    void setTargetCollection(const Akonadi::Collection &collection);
    Akonadi::Collection targetCollection() const override;

    void setParts(const QSet<QByteArray> &parts);
    QSet<QByteArray> parts() const override;

    void setTag(const Akonadi::Tag &tag);
    Akonadi::Tag tag() const override;
    void setAddedTags(const QSet<Akonadi::Tag> &addedTags);
    QSet<Akonadi::Tag> addedTags() const override;
    void setRemovedTags(const QSet<Akonadi::Tag> &removedTags);
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

    QSet<QByteArray> addedFlags() const override;
    QSet<QByteArray> removedFlags() const override;

    void itemChangeCommitted(const Akonadi::Item &item) override;
    void itemsChangesCommitted(const Akonadi::Item::List &items) override;

    void collectionsRetrieved(const Akonadi::Collection::List &collections) override;

    void collectionChangeCommitted(const Akonadi::Collection &collection) override;

    void tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &) override;
    void relationsRetrieved(const Akonadi::Relation::List &tags) override;
    void tagChangeCommitted(const Akonadi::Tag &tag) override;

    void searchFinished(const QVector<qint64> &result, bool isRid = true) override;

    void changeProcessed() override;

    void cancelTask(const QString &errorString) override;
    void deferTask() override;
    void restartItemRetrieval(Akonadi::Collection::Id col) override;
    void taskDone() override;

    void emitError(const QString &message) override;
    void emitWarning(const QString &message) override;
    void emitPercent(int percent) override;

    void synchronizeCollectionTree() override;
    void scheduleConnectionAttempt() override;

    QChar separatorCharacter() const override;
    void setSeparatorCharacter(QChar separator) override;

    void showInformationDialog(const QString &message, const QString &title, const QString &dontShowAgainName) override;

    int batchSize() const override;
    void setItemMergingMode(Akonadi::ItemSync::MergeMode mergeMode) override;

    MessageHelper::Ptr messageHelper() const override;

    QList<QPair<QByteArray, QVariant>> calls() const;

private:
    void recordCall(const QByteArray &callName, const QVariant &parameter = QVariant());

    QString m_userName;
    QString m_resourceName;
    QString m_resourceIdentifier;
    QStringList m_capabilities;
    QList<KIMAP::MailBoxDescriptor> m_namespaces;

    bool m_automaticExpunge;
    bool m_subscriptionEnabled;
    bool m_disconnectedMode;
    int m_intervalCheckTime;
    QChar m_separator;

    Akonadi::ItemSync::MergeMode m_mergeMode;

    Akonadi::Collection m_collection;
    Akonadi::Item::List m_items;

    Akonadi::Collection m_parentCollection;

    Akonadi::Collection m_sourceCollection;
    Akonadi::Collection m_targetCollection;

    QSet<QByteArray> m_parts;

    Akonadi::Tag m_tag;
    QSet<Akonadi::Tag> m_addedTags;
    QSet<Akonadi::Tag> m_removedTags;

    QList<QPair<QByteArray, QVariant>> m_calls;
};
