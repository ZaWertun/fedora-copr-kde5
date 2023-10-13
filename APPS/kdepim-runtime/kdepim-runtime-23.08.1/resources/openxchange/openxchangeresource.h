/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>

class OpenXchangeResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV2
{
    Q_OBJECT

public:
    explicit OpenXchangeResource(const QString &id);
    ~OpenXchangeResource() override;

    void cleanup() override;

public Q_SLOTS:
    void aboutToQuit() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

protected:
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;
    void itemMoved(const Akonadi::Item &item, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    // do not hide the other variant, use implementation from base class
    // which just forwards to the one above
    using Akonadi::AgentBase::ObserverV2::collectionChanged;
    void collectionRemoved(const Akonadi::Collection &collection) override;
    void collectionMoved(const Akonadi::Collection &collection,
                         const Akonadi::Collection &collectionSource,
                         const Akonadi::Collection &collectionDestination) override;

private:
    void onReloadConfiguration();
    void onUpdateUsersJobFinished(KJob *);
    void onFoldersRequestJobFinished(KJob *);
    void onFoldersRequestDeltaJobFinished(KJob *);
    void onFolderCreateJobFinished(KJob *);
    void onFolderModifyJobFinished(KJob *);
    void onFolderMoveJobFinished(KJob *);
    void onFolderDeleteJobFinished(KJob *);
    void onObjectsRequestJobFinished(KJob *);
    void onObjectsRequestDeltaJobFinished(KJob *);
    void onObjectRequestJobFinished(KJob *);
    void onObjectCreateJobFinished(KJob *);
    void onObjectModifyJobFinished(KJob *);
    void onObjectMoveJobFinished(KJob *);
    void onObjectDeleteJobFinished(KJob *);
    void onFetchResourceCollectionsFinished(KJob *);

private:
    void syncCollectionsRemoteIdCache();
    QMap<qlonglong, Akonadi::Collection> mCollectionsMap;

    Akonadi::Collection mResourceCollection;
    QMap<qlonglong, Akonadi::Collection> mStandardCollectionsMap;

    bool mUseIncrementalUpdates = false;
};
