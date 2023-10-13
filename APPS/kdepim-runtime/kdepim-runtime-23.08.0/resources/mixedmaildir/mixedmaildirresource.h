/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>

#include <QStringList>

class CompactChangeHelper;
class MixedMaildirStore;

class MixedMaildirResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV2
{
    Q_OBJECT

public:
    explicit MixedMaildirResource(const QString &id);
    ~MixedMaildirResource() override;

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &col) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;

protected:
    void aboutToQuit() override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemMoved(const Akonadi::Item &item, const Akonadi::Collection &source, const Akonadi::Collection &dest) override;
    void itemRemoved(const Akonadi::Item &item) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes) override;
    void collectionMoved(const Akonadi::Collection &collection, const Akonadi::Collection &source, const Akonadi::Collection &dest) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;

private:
    bool ensureDirExists();
    bool ensureSaneConfiguration();

    void checkForInvalidatedIndexCollections(KJob *job);

private Q_SLOTS:
    void reapplyConfiguration();

    void retrieveCollectionsResult(KJob *job);
    void retrieveItemsResult(KJob *job);
    void retrieveItemResult(KJob *job);

    void itemAddedResult(KJob *job);
    void itemChangedResult(KJob *job);
    void itemMovedResult(KJob *job);
    void itemRemovedResult(KJob *job);

    void itemsDeleted(KJob *job);

    void collectionAddedResult(KJob *job);
    void collectionChangedResult(KJob *job);
    void collectionMovedResult(KJob *job);
    void collectionRemovedResult(KJob *job);

    void compactStore(const QVariant &arg);
    void compactStoreResult(KJob *job);

    void restoreTags(const QVariant &arg);
    void processNextTagContext();
    void tagFetchJobResult(KJob *job);

private:
    MixedMaildirStore *mStore = nullptr;

    struct TagContext {
        Akonadi::Item mItem;
        QStringList mTagList;
    };

    using TagContextList = QVector<TagContext>;
    QHash<Akonadi::Collection::Id, TagContextList> mTagContextByColId;
    TagContextList mPendingTagContexts;

    QSet<Akonadi::Collection::Id> mSynchronizedCollections;
    QSet<Akonadi::Collection::Id> mPendingSynchronizeCollections;

    CompactChangeHelper *mCompactHelper = nullptr;
};
