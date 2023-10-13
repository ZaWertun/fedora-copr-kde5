/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Akonadi/ResourceBase>
#include <KLocalizedString>

#include "etebaseadapter.h"
#include "etesyncclientstate.h"

class EteSyncResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV2
{
    Q_OBJECT

public:
    explicit EteSyncResource(const QString &id);
    ~EteSyncResource() override = default;

    void cleanup() override;
Q_SIGNALS:

    void clientInitialised(bool successful);

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &col) override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;

    void configure(WId windowId) override;

protected:
    void aboutToQuit() override;

    void initialise();

    Q_REQUIRED_RESULT Akonadi::Collection createRootCollection();

    void initialiseDirectory(const QString &path) const;
    Q_REQUIRED_RESULT QString baseDirectoryPath() const;

    bool handleError();
    bool handleError(const int errorCode, const QString &errorMessage);
    bool credentialsRequired();

private Q_SLOTS:
    void onReloadConfiguration();
    void initialiseDone(bool successful);
    void slotItemsRetrieved(KJob *job);
    void slotCollectionsRetrieved(KJob *job);
    void slotTokenRefreshed(bool successful);
    void showErrorDialog(const QString &errorText, const QString &errorDetails = QString(), const QString &title = i18n("Something went wrong"));

private:
    EteSyncClientState::Ptr mClientState;
    QDateTime mJournalsCacheUpdateTime;
    bool mCredentialsRequired = false;
    Akonadi::Collection mRootCollection;
};
