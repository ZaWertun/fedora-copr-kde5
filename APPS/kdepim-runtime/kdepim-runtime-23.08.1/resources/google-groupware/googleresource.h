/*
   SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <Akonadi/AgentBase>
#include <Akonadi/FreeBusyProviderBase>
#include <Akonadi/ResourceBase>

#include <qwindowdefs.h>

#include "calendarhandler.h"
#include "generichandler.h"

#define JOB_PROPERTY "_KGAPI2Job"

namespace KGAPI2
{
class Job;
}

class GoogleSettings;
class GoogleResourceState;

class GoogleResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV3, public Akonadi::FreeBusyProviderBase
{
    Q_OBJECT

public:
    explicit GoogleResource(const QString &id);
    ~GoogleResource() override;

    QList<QUrl> scopes() const;

    void cleanup() override;
public Q_SLOTS:
    void configure(WId windowId) override;
    void reloadConfig();

protected:
    void updateResourceName();
    // Freebusy
    QDateTime lastCacheUpdate() const override;
    void canHandleFreeBusy(const QString &email) const override;
    void retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end) override;

    bool canPerformTask();
    /**
     * KAccounts support abstraction.
     *
     * Returns 0 when compiled without KAccounts or not configured for KAccounts
     */
    int accountId() const;

    void emitReadyStatus();
    void collectionsRetrievedFromHandler(const Akonadi::Collection::List &collections);

    void requestAuthenticationFromUser(const KGAPI2::AccountPtr &account, const QVariant &args = {});
    void runAuthJob(const KGAPI2::AccountPtr &account, const QVariant &args);

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;
    void itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination) override;
    void itemsLinked(const Akonadi::Item::List &items, const Akonadi::Collection &collection) override;
    void itemsUnlinked(const Akonadi::Item::List &items, const Akonadi::Collection &collection) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;

    bool handleError(KGAPI2::Job *job, bool cancelTask = true);

    void slotAuthJobFinished(KGAPI2::Job *job);

private:
    bool m_isConfiguring = false;
    GoogleSettings *m_settings = nullptr;
    Akonadi::Collection m_rootCollection;

    GoogleResourceState *const m_iface;

    std::vector<GenericHandler::Ptr> m_handlers;
    FreeBusyHandler::Ptr m_freeBusyHandler;
    int m_jobs;

    friend class GoogleSettingsDialog;
    friend class GoogleResourceState;

    GenericHandler *fetchHandlerByMimetype(const QString &mimeType);
    GenericHandler *fetchHandlerForCollection(const Akonadi::Collection &collection);
};
