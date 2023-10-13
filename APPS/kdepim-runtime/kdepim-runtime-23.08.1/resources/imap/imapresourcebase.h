/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcestate.h"
#include "resourcestateinterface.h"
#include <Akonadi/AgentSearchInterface>
#include <Akonadi/ResourceBase>
class QDialog;

class QTimer;

class ResourceTask;
namespace KIMAP
{
class Session;
}

class ImapIdleManager;
class SessionPool;
class ResourceState;
class SubscriptionDialog;
class Settings;

class ImapResourceBase : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV4, public Akonadi::AgentSearchInterface
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.ImapResourceBase")
protected:
    using Akonadi::AgentBase::Observer::collectionChanged;

public:
    explicit ImapResourceBase(const QString &id);
    ~ImapResourceBase() override;

    virtual QDialog *createConfigureDialog(WId windowId) = 0;

    void cleanup() override;

    virtual Settings *settings() const;

public Q_SLOTS:
    void configure(WId windowId) override;

    // DBus methods
    Q_SCRIPTABLE void requestManualExpunge(qint64 collectionId);
    Q_SCRIPTABLE int configureSubscription(qlonglong windowId = 0);
    Q_SCRIPTABLE QStringList serverCapabilities() const;

    // pseudo-virtual called by ResourceBase
    QString dumpResourceToString() const override;

protected:
    using ResourceBase::retrieveItems; // suppress -Woverload-virtual

protected Q_SLOTS:
    void startIdleIfNeeded();
    void startIdle();

    void abortActivity() override;

    void retrieveCollections() override;
    void retrieveCollectionAttributes(const Akonadi::Collection &col) override;

    void retrieveItems(const Akonadi::Collection &col) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

protected:
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;
    void itemsMoved(const Akonadi::Item::List &item, const Akonadi::Collection &source, const Akonadi::Collection &destination) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &parts) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;
    void collectionMoved(const Akonadi::Collection &collection, const Akonadi::Collection &source, const Akonadi::Collection &destination) override;

    void addSearch(const QString &query, const QString &queryLanguage, const Akonadi::Collection &resultCollection) override;
    void removeSearch(const Akonadi::Collection &resultCollection) override;
    void search(const QString &query, const Akonadi::Collection &collection) override;

    void doSetOnline(bool online) override;

    Q_REQUIRED_RESULT QChar separatorCharacter() const;
    void setSeparatorCharacter(QChar separator);

    void aboutToQuit() override;

    virtual ResourceStateInterface::Ptr createResourceState(const TaskArguments &);
    virtual QString defaultName() const = 0;
    virtual QByteArray clientId() const = 0;

protected Q_SLOTS:
    void delayedInit();

private Q_SLOTS:
    void doSearch(const QVariant &arg);

    void reconnect();

    void scheduleConnectionAttempt();
    void startConnect(const QVariant &); // the parameter is necessary, since this method is used by the task scheduler
    void onConnectDone(int errorCode, const QString &errorMessage);
    void onConnectionLost(KIMAP::Session *session);

    void onIdleCollectionFetchDone(KJob *job);

    void onExpungeCollectionFetchDone(KJob *job);
    void triggerCollectionExpunge(const QVariant &collectionVariant);

    void taskDestroyed(QObject *task);

    void showError(const QString &message);
    void clearStatusMessage();

    void updateResourceName();

    void onCollectionModifyDone(KJob *job);

protected:
    // Starts and queues a task
    void startTask(ResourceTask *task);
    void queueTask(ResourceTask *task);
    SessionPool *m_pool = nullptr;
    mutable Settings *m_settings;

private:
    friend class ResourceState;

    bool needsNetwork() const;
    void modifyCollection(const Akonadi::Collection &);

    friend class ImapIdleManager;

    QList<ResourceTask *> m_taskList; // used to be able to kill tasks
    QScopedPointer<SubscriptionDialog, QScopedPointerDeleteLater> mSubscriptions;
    ImapIdleManager *m_idle = nullptr;
    QTimer *m_statusMessageTimer = nullptr;
    QChar m_separatorCharacter;
};
