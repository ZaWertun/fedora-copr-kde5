/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapresourcebase.h"

#include <QHostInfo>
#include <QSettings>

#include "imapresource_debug.h"
#include <KLocalizedString>
#include <QIcon>

#include <kwindowsystem.h>

#include <Akonadi/MessageParts>

#include <Akonadi/AgentManager>
#include <Akonadi/AttributeFactory>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionAnnotationsAttribute>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>
#include <Akonadi/SpecialCollections>

#include <QStandardPaths>

#include "collectionflagsattribute.h"
#include "highestmodseqattribute.h"
#include "imapaclattribute.h"
#include "imapquotaattribute.h"
#include "noselectattribute.h"
#include "uidnextattribute.h"
#include "uidvalidityattribute.h"

#include "imapaccount.h"
#include "imapidlemanager.h"
#include "settings.h"
#include "subscriptiondialog.h"

#include "addcollectiontask.h"
#include "additemtask.h"
#include "changecollectiontask.h"
#include "changeitemsflagstask.h"
#include "changeitemtask.h"
#include "expungecollectiontask.h"
#include "movecollectiontask.h"
#include "moveitemstask.h"
#include "removecollectionrecursivetask.h"
#include "retrievecollectionmetadatatask.h"
#include "retrievecollectionstask.h"
#include "retrieveitemstask.h"
#include "retrieveitemtask.h"
#include "searchtask.h"

#include "imapflags.h"
#include "sessionpool.h"

#include "resourceadaptor.h"

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
#endif

Q_DECLARE_METATYPE(QList<qint64>)
Q_DECLARE_METATYPE(QWeakPointer<QObject>)

using namespace Akonadi;

ImapResourceBase::ImapResourceBase(const QString &id)
    : ResourceBase(id)
    , m_pool(new SessionPool(2, this))
    , m_settings(nullptr)
{
    QTimer::singleShot(0, this, &ImapResourceBase::updateResourceName);

    connect(m_pool, &SessionPool::connectDone, this, &ImapResourceBase::onConnectDone);
    connect(m_pool, &SessionPool::connectionLost, this, &ImapResourceBase::onConnectionLost);

    Akonadi::AttributeFactory::registerAttribute<UidValidityAttribute>();
    Akonadi::AttributeFactory::registerAttribute<UidNextAttribute>();
    Akonadi::AttributeFactory::registerAttribute<NoSelectAttribute>();
    Akonadi::AttributeFactory::registerAttribute<HighestModSeqAttribute>();

    Akonadi::AttributeFactory::registerAttribute<CollectionFlagsAttribute>();

    Akonadi::AttributeFactory::registerAttribute<ImapAclAttribute>();
    Akonadi::AttributeFactory::registerAttribute<ImapQuotaAttribute>();

    // For QMetaObject::invokeMethod()
    qRegisterMetaType<QList<qint64>>();

    changeRecorder()->fetchCollection(true);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(CollectionFetchScope::All);
    changeRecorder()->collectionFetchScope().setIncludeStatistics(true);
    changeRecorder()->collectionFetchScope().fetchAttribute<CollectionAnnotationsAttribute>();
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::All);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    //(Andras) disable now, as tokoe reported problems with it and the mail filter: changeRecorder()->fetchChangedOnly( true );

    setHierarchicalRemoteIdentifiersEnabled(true);
    setItemTransactionMode(ItemSync::MultipleTransactions); // we can recover from incomplete syncs, so we can use a faster mode
    setDisableAutomaticItemDeliveryDone(true);
    setItemSyncBatchSize(100);

    connect(this, &AgentBase::reloadConfiguration, this, &ImapResourceBase::reconnect);

    m_statusMessageTimer = new QTimer(this);
    m_statusMessageTimer->setSingleShot(true);
    connect(m_statusMessageTimer, &QTimer::timeout, this, &ImapResourceBase::clearStatusMessage);
    connect(this, &AgentBase::error, this, &ImapResourceBase::showError);

    new ImapResourceBaseAdaptor(this);

    QMetaObject::invokeMethod(this, &ImapResourceBase::delayedInit, Qt::QueuedConnection);
}

void ImapResourceBase::delayedInit()
{
    setNeedsNetwork(needsNetwork());

    // Migration issue: trash folder had ID in config, but didn't have SpecialCollections attribute, fix that.
    if (!settings()->trashCollectionMigrated()) {
        const Akonadi::Collection::Id trashCollection = settings()->trashCollection();
        if (trashCollection != -1) {
            Collection attributeCollection(trashCollection);
            SpecialCollections::setSpecialCollectionType("trash", attributeCollection);
        }
        settings()->setTrashCollectionMigrated(true);
    }
}

ImapResourceBase::~ImapResourceBase()
{
    // Destroy everything that could cause callbacks immediately, otherwise the callbacks can result in a crash.

    delete m_idle;
    m_idle = nullptr;

    for (ResourceTask *task : std::as_const(m_taskList)) {
        delete task;
    }
    m_taskList.clear();

    delete m_pool;
    delete m_settings;
}

void ImapResourceBase::aboutToQuit()
{
    // TODO the resource would ideally have to signal when it's done with logging out etc, before the destructor gets called
    if (m_idle) {
        m_idle->stop();
    }

    for (ResourceTask *task : std::as_const(m_taskList)) {
        task->kill();
    }

    m_pool->disconnect();
}

void ImapResourceBase::updateResourceName()
{
    if (name() == identifier()) {
        const QString agentType = AgentManager::self()->instance(identifier()).type().identifier();
        const QString agentsrcFile =
            QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QLatin1String("akonadi/agentsrc");

        const QSettings agentsrc(agentsrcFile, QSettings::IniFormat);
        const int instanceCounter = agentsrc.value(QStringLiteral("InstanceCounters/%1/InstanceCounter").arg(agentType), -1).toInt();

        if (instanceCounter > 0) {
            setName(QStringLiteral("%1 %2").arg(defaultName()).arg(instanceCounter));
        } else {
            setName(defaultName());
        }
    }
}

// -----------------------------------------------------------------------------

void ImapResourceBase::configure(WId windowId)
{
    if (createConfigureDialog(windowId)->exec() == QDialog::Accepted) {
        Q_EMIT configurationDialogAccepted();
        reconnect();
    } else {
        Q_EMIT configurationDialogRejected();
    }
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::startConnect(const QVariant &)
{
    if (settings()->imapServer().isEmpty()) {
        setOnline(false);
        Q_EMIT status(NotConfigured, i18n("No server configured yet."));
        taskDone();
        return;
    }

    m_pool->disconnect(); // reset all state, delete any old account
    auto account = new ImapAccount;
    settings()->loadAccount(account);

    const bool result = m_pool->connect(account);
    Q_ASSERT(result);
    Q_UNUSED(result)
}

int ImapResourceBase::configureSubscription(qlonglong windowId)
{
    if (mSubscriptions) {
        return 0;
    }

    if (!m_pool->account()) {
        return -2;
    }
    const QString password = settings()->password();
    if (password.isEmpty()) {
        return -1;
    }

    mSubscriptions.reset(new SubscriptionDialog(nullptr, SubscriptionDialog::AllowToEnableSubscription));
    if (windowId) {
        mSubscriptions->setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(mSubscriptions->windowHandle(), windowId);
    }
    mSubscriptions->setWindowTitle(i18nc("@title:window", "Serverside Subscription"));
    mSubscriptions->setWindowIcon(QIcon::fromTheme(QStringLiteral("network-server")));
    mSubscriptions->connectAccount(*m_pool->account(), password);
    mSubscriptions->setSubscriptionEnabled(settings()->subscriptionEnabled());
    connect(mSubscriptions.get(), &SubscriptionDialog::accepted, this, [this]() {
        settings()->setSubscriptionEnabled(mSubscriptions->subscriptionEnabled());
        settings()->save();
        Q_EMIT configurationDialogAccepted();
        reconnect();
    });
    connect(mSubscriptions.get(), &SubscriptionDialog::finished, this, [this]() {
        mSubscriptions.reset();
    });
    mSubscriptions->show();

    return 0;
}

void ImapResourceBase::onConnectDone(int errorCode, const QString &errorString)
{
    switch (errorCode) {
    case SessionPool::NoError:
        setOnline(true);
        taskDone();
        Q_EMIT status(Idle, i18n("Connection established."));

        synchronizeCollectionTree();
        break;

    case SessionPool::PasswordRequestError:
    case SessionPool::EncryptionError:
    case SessionPool::LoginFailError:
    case SessionPool::CapabilitiesTestError:
    case SessionPool::IncompatibleServerError:
        setOnline(false);
        Q_EMIT status(Broken, errorString);
        cancelTask(errorString);
        return;

    case SessionPool::CouldNotConnectError:
    case SessionPool::CancelledError: // e.g. we got disconnected during login
        Q_EMIT status(Idle, i18n("Server is not available."));
        deferTask();
        setTemporaryOffline((m_pool->account() && m_pool->account()->timeout() > 0) ? m_pool->account()->timeout() : 300);
        return;

    case SessionPool::ReconnectNeededError:
        reconnect();
        return;

    case SessionPool::NoAvailableSessionError:
        qFatal("Shouldn't happen");
        return;
    }
}

void ImapResourceBase::onConnectionLost(KIMAP::Session * /*session*/)
{
    if (!m_pool->isConnected() && isOnline()) {
        reconnect();
    }
}

ResourceStateInterface::Ptr ImapResourceBase::createResourceState(const TaskArguments &args)
{
    return ResourceStateInterface::Ptr(new ResourceState(this, args));
}

Settings *ImapResourceBase::settings() const
{
    if (m_settings == nullptr) {
        m_settings = new Settings;
    }

    return m_settings;
}

// ----------------------------------------------------------------------------------

bool ImapResourceBase::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    // The collection name is empty here...
    // Q_EMIT status( AgentBase::Running, i18nc( "@info:status", "Retrieving item in '%1'", item.parentCollection().name() ) );

    auto task = new RetrieveItemTask(createResourceState(TaskArguments(item, parts)), this);
    task->start(m_pool);
    queueTask(task);
    return true;
}

void ImapResourceBase::itemAdded(const Item &item, const Collection &collection)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Adding item in '%1'", collection.name()));

    startTask(new AddItemTask(createResourceState(TaskArguments(item, collection)), this));
}

void ImapResourceBase::itemChanged(const Item &item, const QSet<QByteArray> &parts)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Updating item in '%1'", item.parentCollection().name()));

    startTask(new ChangeItemTask(createResourceState(TaskArguments(item, parts)), this));
}

void ImapResourceBase::itemsFlagsChanged(const Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Updating items"));

    startTask(new ChangeItemsFlagsTask(createResourceState(TaskArguments(items, addedFlags, removedFlags)), this));
}

void ImapResourceBase::itemsRemoved(const Akonadi::Item::List &items)
{
    const QString mailBox = ResourceStateInterface::mailBoxForCollection(items.first().parentCollection(), false);
    if (mailBox.isEmpty()) {
        // this item will be removed soon by its parent collection
        changeProcessed();
        return;
    }

    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Removing items"));

    startTask(new ChangeItemsFlagsTask(createResourceState(TaskArguments(items, QSet<QByteArray>() << ImapFlags::Deleted, QSet<QByteArray>())), this));
}

void ImapResourceBase::itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &source, const Akonadi::Collection &destination)
{
    if (items.first().parentCollection() != destination) { // should have been set by the server
        qCWarning(IMAPRESOURCE_LOG) << "Collections don't match: destination=" << destination.id() << "; items parent=" << items.first().parentCollection().id()
                                    << "; source collection=" << source.id();
        // Q_ASSERT( false );
        // TODO: Find out why this happens
        cancelTask();
        return;
    }

    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Moving items from '%1' to '%2'", source.name(), destination.name()));

    startTask(new MoveItemsTask(createResourceState(TaskArguments(items, source, destination)), this));
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::retrieveCollections()
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Retrieving folders"));

    startTask(new RetrieveCollectionsTask(createResourceState(TaskArguments()), this));
}

void ImapResourceBase::retrieveCollectionAttributes(const Akonadi::Collection &col)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Retrieving extra folder information for '%1'", col.name()));
    startTask(new RetrieveCollectionMetadataTask(createResourceState(TaskArguments(col)), this));
}

void ImapResourceBase::retrieveItems(const Collection &col)
{
    synchronizeCollectionAttributes(col.id());

    setItemStreamingEnabled(true);

    auto task = new RetrieveItemsTask(createResourceState(TaskArguments(col)), this);
    connect(task, SIGNAL(status(int, QString)), SIGNAL(status(int, QString)));
    connect(this, &ResourceBase::retrieveNextItemSyncBatch, task, &RetrieveItemsTask::onReadyForNextBatch);
    startTask(task);
}

void ImapResourceBase::collectionAdded(const Collection &collection, const Collection &parent)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Creating folder '%1'", collection.name()));
    startTask(new AddCollectionTask(createResourceState(TaskArguments(collection, parent)), this));
}

void ImapResourceBase::collectionChanged(const Collection &collection, const QSet<QByteArray> &parts)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Updating folder '%1'", collection.name()));
    startTask(new ChangeCollectionTask(createResourceState(TaskArguments(collection, parts)), this));
}

void ImapResourceBase::collectionRemoved(const Collection &collection)
{
    // TODO Move this to the task
    const QString mailBox = ResourceStateInterface::mailBoxForCollection(collection, false);
    if (mailBox.isEmpty()) {
        // this collection will be removed soon by its parent collection
        changeProcessed();
        return;
    }
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Removing folder '%1'", collection.name()));

    startTask(new RemoveCollectionRecursiveTask(createResourceState(TaskArguments(collection)), this));
}

void ImapResourceBase::collectionMoved(const Akonadi::Collection &collection, const Akonadi::Collection &source, const Akonadi::Collection &destination)
{
    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Moving folder '%1' from '%2' to '%3'", collection.name(), source.name(), destination.name()));
    startTask(new MoveCollectionTask(createResourceState(TaskArguments(collection, source, destination)), this));
}

void ImapResourceBase::addSearch(const QString &query, const QString &queryLanguage, const Collection &resultCollection)
{
    Q_UNUSED(query)
    Q_UNUSED(queryLanguage)
    Q_UNUSED(resultCollection)
}

void ImapResourceBase::removeSearch(const Collection &resultCollection)
{
    Q_UNUSED(resultCollection)
}

void ImapResourceBase::search(const QString &query, const Collection &collection)
{
    QVariantMap arg;
    arg[QStringLiteral("query")] = query;
    arg[QStringLiteral("collection")] = QVariant::fromValue(collection);
    scheduleCustomTask(this, "doSearch", arg);
}

void ImapResourceBase::doSearch(const QVariant &arg)
{
    const QVariantMap map = arg.toMap();
    const QString query = map[QStringLiteral("query")].toString();
    const auto collection = map[QStringLiteral("collection")].value<Collection>();

    Q_EMIT status(AgentBase::Running, i18nc("@info:status", "Searching..."));
    startTask(new SearchTask(createResourceState(TaskArguments(collection)), query, this));
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::scheduleConnectionAttempt()
{
    // block all other tasks, until we are connected
    scheduleCustomTask(this, "startConnect", QVariant(), ResourceBase::Prepend);
}

void ImapResourceBase::doSetOnline(bool online)
{
    qCDebug(IMAPRESOURCE_LOG) << "online=" << online;
    if (!online) {
        for (ResourceTask *task : std::as_const(m_taskList)) {
            task->kill();
            delete task;
        }
        m_taskList.clear();
        m_pool->cancelPasswordRequests();
        if (m_pool->isConnected()) {
            m_pool->disconnect();
        }
        if (m_idle) {
            m_idle->stop();
            delete m_idle;
            m_idle = nullptr;
        }
        settings()->clearCachedPassword();
    } else if (online && !m_pool->isConnected()) {
        scheduleConnectionAttempt();
    }
    ResourceBase::doSetOnline(online);
}

QChar ImapResourceBase::separatorCharacter() const
{
    return m_separatorCharacter;
}

void ImapResourceBase::setSeparatorCharacter(QChar separator)
{
    m_separatorCharacter = separator;
}

bool ImapResourceBase::needsNetwork() const
{
    const QString hostName = settings()->imapServer().section(QLatin1Char(':'), 0, 0);
    // ### is there a better way to do this?
    if (hostName == QLatin1String("127.0.0.1") || hostName == QLatin1String("localhost") || hostName == QHostInfo::localHostName()) {
        return false;
    }
    return true;
}

void ImapResourceBase::reconnect()
{
    setNeedsNetwork(needsNetwork());
    setOnline(false); // we are not connected initially
    setOnline(true);
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::startIdleIfNeeded()
{
    if (!m_idle) {
        startIdle();
    }
}

void ImapResourceBase::startIdle()
{
    delete m_idle;
    m_idle = nullptr;

    if (!m_pool->serverCapabilities().contains(QLatin1String("IDLE"))) {
        return;
    }

    // Without password we don't even have to try
    if (m_pool->account()->authenticationMode() != KIMAP::LoginJob::GSSAPI && settings()->password().isEmpty()) {
        return;
    }

    const QStringList ridPath = settings()->idleRidPath();
    if (ridPath.size() < 2) {
        return;
    }

    Collection c, p;
    p.setParentCollection(Collection::root());
    for (int i = ridPath.size() - 1; i > 0; --i) {
        p.setRemoteId(ridPath.at(i));
        c.setParentCollection(p);
        p = c;
    }
    c.setRemoteId(ridPath.first());

    Akonadi::CollectionFetchScope scope;
    scope.setResource(identifier());
    scope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

    auto fetch = new Akonadi::CollectionFetchJob(c, Akonadi::CollectionFetchJob::Base, this);
    fetch->setFetchScope(scope);

    connect(fetch, &KJob::result, this, &ImapResourceBase::onIdleCollectionFetchDone);
}

void ImapResourceBase::onIdleCollectionFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "CollectionFetch for idling failed."
                                    << "error=" << job->error() << ", errorString=" << job->errorString();
        return;
    }
    auto fetch = static_cast<Akonadi::CollectionFetchJob *>(job);
    // Can be empty if collection is not subscribed locally
    if (!fetch->collections().isEmpty()) {
        delete m_idle;
        m_idle = new ImapIdleManager(createResourceState(TaskArguments(fetch->collections().at(0))), m_pool, this);
    } else {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to retrieve IDLE collection: no such collection";
    }
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::requestManualExpunge(qint64 collectionId)
{
    if (!settings()->automaticExpungeEnabled()) {
        Collection collection(collectionId);

        Akonadi::CollectionFetchScope scope;
        scope.setResource(identifier());
        scope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        scope.setListFilter(CollectionFetchScope::NoFilter);

        auto fetch = new Akonadi::CollectionFetchJob(collection, Akonadi::CollectionFetchJob::Base, this);
        fetch->setFetchScope(scope);

        connect(fetch, &KJob::result, this, &ImapResourceBase::onExpungeCollectionFetchDone);
    }
}

void ImapResourceBase::onExpungeCollectionFetchDone(KJob *job)
{
    if (job->error() == 0) {
        auto fetch = static_cast<Akonadi::CollectionFetchJob *>(job);
        Akonadi::Collection collection = fetch->collections().at(0);

        scheduleCustomTask(this, "triggerCollectionExpunge", QVariant::fromValue(collection));
    } else {
        qCWarning(IMAPRESOURCE_LOG) << "CollectionFetch for expunge failed."
                                    << "error=" << job->error() << ", errorString=" << job->errorString();
    }
}

void ImapResourceBase::triggerCollectionExpunge(const QVariant &collectionVariant)
{
    const auto collection = collectionVariant.value<Collection>();

    auto task = new ExpungeCollectionTask(createResourceState(TaskArguments(collection)), this);
    task->start(m_pool);
    queueTask(task);
}

// ----------------------------------------------------------------------------------

void ImapResourceBase::abortActivity()
{
    if (!m_taskList.isEmpty()) {
        m_pool->disconnect(SessionPool::CloseSession);
        scheduleConnectionAttempt();
    }
}

void ImapResourceBase::queueTask(ResourceTask *task)
{
    connect(task, &QObject::destroyed, this, &ImapResourceBase::taskDestroyed);
    m_taskList << task;
}

void ImapResourceBase::startTask(ResourceTask *task)
{
    task->start(m_pool);
    queueTask(task);
}

void ImapResourceBase::taskDestroyed(QObject *task)
{
    m_taskList.removeAll(static_cast<ResourceTask *>(task));
}

QStringList ImapResourceBase::serverCapabilities() const
{
    return m_pool->serverCapabilities();
}

void ImapResourceBase::cleanup()
{
    settings()->cleanup();

    ResourceBase::cleanup();
}

QString ImapResourceBase::dumpResourceToString() const
{
    QString ret;
    for (ResourceTask *task : std::as_const(m_taskList)) {
        if (!ret.isEmpty()) {
            ret += QLatin1String(", ");
        }
        ret += QLatin1String(task->metaObject()->className());
    }
    return QLatin1String("IMAP tasks: ") + ret;
}

void ImapResourceBase::showError(const QString &message)
{
    Q_EMIT status(Akonadi::AgentBase::Idle, message);
    m_statusMessageTimer->start(1000 * 10);
}

void ImapResourceBase::clearStatusMessage()
{
    Q_EMIT status(Akonadi::AgentBase::Idle, QString());
}

void ImapResourceBase::modifyCollection(const Collection &col)
{
    auto modJob = new Akonadi::CollectionModifyJob(col, this);
    connect(modJob, &KJob::result, this, &ImapResourceBase::onCollectionModifyDone);
}

void ImapResourceBase::onCollectionModifyDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to modify collection: " << job->errorString();
    }
}
