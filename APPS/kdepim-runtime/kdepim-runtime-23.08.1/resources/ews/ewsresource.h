/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQueue>
#include <QScopedPointer>

#include <Akonadi/ResourceBase>
#include <Akonadi/TransportResourceBase>

#include "ewsclient.h"
#include "ewsfetchitemsjob.h"
#include "ewsid.h"

#include <ewsconfig.h>

class FetchItemState;
class EwsAbstractAuth;
class EwsSubscriptionManager;
class EwsTagStore;
class EwsSettings;
class KNotification;

class EwsResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV4, public Akonadi::TransportResourceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.Ews.Resource")
public:
    static const QString akonadiEwsPropsetUuid;
    static const EwsPropertyField globalTagsProperty;
    static const EwsPropertyField globalTagsVersionProperty;
    static const EwsPropertyField tagsProperty;
    static const EwsPropertyField flagsProperty;

    explicit EwsResource(const QString &id);
    ~EwsResource() override;

    void itemsTagsChanged(const Akonadi::Item::List &items, const QSet<Akonadi::Tag> &addedTags, const QSet<Akonadi::Tag> &removedTags) override;
    void tagAdded(const Akonadi::Tag &tag) override;
    void tagChanged(const Akonadi::Tag &tag) override;
    void tagRemoved(const Akonadi::Tag &tag) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionMoved(const Akonadi::Collection &collection,
                         const Akonadi::Collection &collectionSource,
                         const Akonadi::Collection &collectionDestination) override;
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) override;
    void itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags) override;
    void itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &sourceCollection, const Akonadi::Collection &destinationCollection) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;

    void sendItem(const Akonadi::Item &item) override;

    const Akonadi::Collection &rootCollection() const
    {
        return mRootCollection;
    }

    EwsSettings *settings()
    {
        return mSettings.data();
    }

protected:
    void doSetOnline(bool online) override;
public Q_SLOTS:
    void configure(WId windowId) override;
    Q_SCRIPTABLE void clearCollectionSyncState(int collectionId);
    Q_SCRIPTABLE void clearFolderTreeSyncState();
    Q_SCRIPTABLE void setInitialReconnectTimeout(int timeout);
protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    void retrieveTags() override;
    QString dumpResourceToString() const override;
private Q_SLOTS:
    void fetchFoldersJobFinished(KJob *job);
    void fetchFoldersIncrJobFinished(KJob *job);
    void itemFetchJobFinished(KJob *job);
    void getItemsRequestFinished(KJob *job);
    void itemChangeRequestFinished(KJob *job);
    void itemModifyFlagsRequestFinished(KJob *job);
    void itemMoveRequestFinished(KJob *job);
    void itemDeleteRequestFinished(KJob *job);
    void itemCreateRequestFinished(KJob *job);
    void itemSendRequestFinished(KJob *job);
    void folderCreateRequestFinished(KJob *job);
    void folderMoveRequestFinished(KJob *job);
    void folderUpdateRequestFinished(KJob *job);
    void folderDeleteRequestFinished(KJob *job);
    void delayedInit();
    void foldersModifiedEvent(const EwsId::List &folders);
    void foldersModifiedCollectionSyncFinished(KJob *job);
    void folderTreeModifiedEvent();
    void fullSyncRequestedEvent();
    void rootFolderFetchFinished(KJob *job);
    void specialFoldersFetchFinished(KJob *job);
    void itemsTagChangeFinished(KJob *job);
    void globalTagChangeFinished(KJob *job);
    void globalTagsRetrievalFinished(KJob *job);
    void adjustInboxRemoteIdFetchFinished(KJob *job);
    void rootCollectionFetched(KJob *job);
    void connectionError();
    void reloadConfig();
    void authSucceeded();
    void authFailed(const QString &error);
    void requestAuthFailed();
    void emitReadyStatus();
    void adjustRootCollectionName(const QString &newName);
public Q_SLOTS:
    Q_SCRIPTABLE void sendMessage(const QString &id, const QByteArray &content);
Q_SIGNALS:
    Q_SCRIPTABLE void messageSent(const QString &id, const QString &error);
#if HAVE_SEPARATE_MTA_RESOURCE
private Q_SLOTS:
    void messageSendRequestFinished(KJob *job);
#endif

private:
    enum AuthStage { AuthIdle, AuthRefreshToken, AuthAccessToken, AuthFailure };

    enum QueuedFetchItemsJobType { RetrieveItems, SubscriptionSync };

    void finishItemsFetch(FetchItemState *state);
    void fetchSpecialFolders();
    void specialFoldersCollectionsRetrieved(const Akonadi::Collection::List &folders);

    void saveState();
    void resetUrl();

    void doRetrieveCollections();

    int reconnectTimeout();
    void setUpAuth();
    void reauthNotificationDismissed(bool accepted);
    void reauthenticate();

    static QString getCollectionSyncState(const Akonadi::Collection &col);
    static void saveCollectionSyncState(Akonadi::Collection &col, const QString &state);

    void queueFetchItemsJob(const Akonadi::Collection &col, QueuedFetchItemsJobType type, const std::function<void(EwsFetchItemsJob *)> &startFn);
    void startFetchItemsJob(const Akonadi::Collection &col, std::function<void(EwsFetchItemsJob *)> startFn);
    void dequeueFetchItemsJob();

    template<class Job>
    void connectStatusSignals(Job *job);

    EwsClient mEwsClient;
    Akonadi::Collection mRootCollection;
    QScopedPointer<EwsSubscriptionManager> mSubManager;
    QString mFolderSyncState;
    QHash<QString, EwsId::List> mItemsToCheck;
    QString mPassword;
    QScopedPointer<EwsAbstractAuth> mAuth;
    AuthStage mAuthStage;
    QPointer<KNotification> mReauthNotification;

    bool mTagsRetrieved = false;
    int mReconnectTimeout;
    int mInitialReconnectTimeout;
    EwsTagStore *mTagStore = nullptr;
    QScopedPointer<EwsSettings> mSettings;

    struct QueuedFetchItemsJob {
        Akonadi::Collection col;
        QueuedFetchItemsJobType type;
        std::function<void(EwsFetchItemsJob *)> startFn;
    };
    QQueue<QueuedFetchItemsJob> mFetchItemsJobQueue;
};
