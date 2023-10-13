/*
   SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>
   SPDX-FileCopyrightText: 2015-2020 Daniel Vrátil <dvratil@kde.org>
   SPDX-FileCopyrightText: 2020 Igor Pobiko <igor.poboiko@gmail.com>

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "googleresource.h"
#include "googleresource_debug.h"
#include "googleresourcestate.h"
#include "googlesettings.h"
#include "googlesettingsdialog.h"
#include "settingsadaptor.h"

#include "personhandler.h"
#include "taskhandler.h"

#include "defaultreminderattribute.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/CachePolicy>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KCalendarCore/Event>

#include <KLocalizedString>
#include <KNotification>
#include <QDialog>
#include <QIcon>

#include <KGAPI/Account>
#include <KGAPI/AccountInfo>
#include <KGAPI/AccountInfoFetchJob>
#include <KGAPI/AuthJob>

#include <algorithm>
#include <memory>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif
using namespace QKeychain;

#define CALENDARS_PROPERTY "_KGAPI2CalendarPtr"
#define ROOT_COLLECTION_REMOTEID QStringLiteral("RootCollection")

Q_DECLARE_METATYPE(KGAPI2::Job *)

using namespace KGAPI2;
using namespace Akonadi;

namespace
{
bool accountIsValid(const KGAPI2::AccountPtr &account)
{
    return account && !account->accessToken().isEmpty() && !account->refreshToken().isEmpty() && !account->accountName().isEmpty()
        && !account->scopes().isEmpty();
}
} // namespace

GoogleResource::GoogleResource(const QString &id)
    : ResourceBase(id)
    , AgentBase::ObserverV3()
    , m_iface(new GoogleResourceState(this))
{
    AttributeFactory::registerAttribute<DefaultReminderAttribute>();

    connect(this, &GoogleResource::reloadConfiguration, this, &GoogleResource::reloadConfig);

    setNeedsNetwork(true);

    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setFetchVirtualReferences(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::All);
    changeRecorder()->fetchCollection(true);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(CollectionFetchScope::All);

    Q_EMIT status(NotConfigured, i18n("Fetching password..."));

    m_settings = new GoogleSettings();
    m_settings->setWindowId(winIdForDialogs());
    connect(m_settings, &GoogleSettings::accountReady, this, [this](bool ready) {
        if (accountId() > 0) {
            return;
        }
        if (!ready) {
            Q_EMIT status(Broken, i18n("Can't fetch password"));
            return;
        }

        const auto account = m_settings->accountPtr();
        if (account.isNull()) {
            Q_EMIT status(NotConfigured);
            return;
        }

        if (!accountIsValid(account)) {
            requestAuthenticationFromUser(account);
        } else {
            emitReadyStatus();
            synchronize();
        }
    });
    m_settings->init();

    updateResourceName();

    m_freeBusyHandler = std::make_unique<FreeBusyHandler>(m_iface, m_settings);
    m_handlers.clear();
    m_handlers.push_back(GenericHandler::Ptr(new CalendarHandler(m_iface, m_settings)));
    m_handlers.push_back(GenericHandler::Ptr(new PersonHandler(m_iface, m_settings)));
    m_handlers.push_back(GenericHandler::Ptr(new TaskHandler(m_iface, m_settings)));

    new SettingsAdaptor(m_settings);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), m_settings, QDBusConnection::ExportAdaptors);
}

GoogleResource::~GoogleResource()
{
    delete m_iface;
}

void GoogleResource::cleanup()
{
    m_settings->cleanup();
    ResourceBase::cleanup();
}

void GoogleResource::emitReadyStatus()
{
    Q_EMIT status(Idle, i18nc("@info:status", "Ready"));
}

void GoogleResource::configure(WId windowId)
{
    if (!m_settings->isReady() || m_isConfiguring) {
        Q_EMIT configurationDialogAccepted();
        return;
    }

    m_isConfiguring = true;

    QScopedPointer<GoogleSettingsDialog> settingsDialog(new GoogleSettingsDialog(this, m_settings, windowId));
    settingsDialog->setWindowIcon(QIcon::fromTheme(QStringLiteral("im-google")));
    if (settingsDialog->exec() == QDialog::Accepted) {
        updateResourceName();

        Q_EMIT configurationDialogAccepted();

        if (m_settings->accountPtr().isNull()) {
            Q_EMIT status(NotConfigured, i18n("Configured account does not exist"));
            m_isConfiguring = false;
            return;
        }

        emitReadyStatus();
        synchronize();
    } else {
        updateResourceName();

        Q_EMIT configurationDialogRejected();
    }

    m_isConfiguring = false;
}

QList<QUrl> GoogleResource::scopes() const
{
    // TODO: determine it based on what user wants?
    const QList<QUrl> scopes = {
        Account::accountInfoScopeUrl(),
        Account::calendarScopeUrl(),
        Account::peopleScopeUrl(),
        Account::tasksScopeUrl(),
    };
    return scopes;
}

void GoogleResource::updateResourceName()
{
    const QString accountName = m_settings->account();
    setName(i18nc("%1 is account name (user@gmail.com)", "Google Groupware (%1)", accountName.isEmpty() ? i18n("not configured") : accountName));
}

void GoogleResource::reloadConfig()
{
    const AccountPtr account = m_settings->accountPtr();
    if (account.isNull() || account->accountName().isEmpty()) {
        Q_EMIT status(NotConfigured, i18n("Configured account does not exist"));
    } else {
        emitReadyStatus();
    }
}

bool GoogleResource::handleError(KGAPI2::Job *job, bool _cancelTask)
{
    if ((job->error() == KGAPI2::NoError) || (job->error() == KGAPI2::OK)) {
        return true;
    }
    qCWarning(GOOGLE_LOG) << "Got error:" << job << job->errorString();
    AccountPtr account = job->account();
    if (job->error() == KGAPI2::Unauthorized) {
        const QList<QUrl> resourceScopes = scopes();
        bool scopesChanged = false;
        for (const QUrl &scope : resourceScopes) {
            if (!account->scopes().contains(scope)) {
                account->addScope(scope);
                scopesChanged = true;
            }
        }

        if (scopesChanged || !accountIsValid(account)) {
            requestAuthenticationFromUser(account, QVariant::fromValue(job));
        } else {
            runAuthJob(account, QVariant::fromValue(job));
        }
        return false;
    }

    if (_cancelTask) {
        cancelTask(job->errorString());
    }
    return false;
}

void GoogleResource::runAuthJob(const KGAPI2::AccountPtr &account, const QVariant &args)
{
    auto authJob = new AuthJob(account, m_settings->clientId(), m_settings->clientSecret(), this);
    authJob->setProperty(JOB_PROPERTY, args);
    connect(authJob, &AuthJob::finished, this, &GoogleResource::slotAuthJobFinished);
}

void GoogleResource::requestAuthenticationFromUser(const KGAPI2::AccountPtr &account, const QVariant &args)
{
    Q_EMIT status(Broken, i18n("Account has been logged out."));

    const QString msg = account->accountName().isEmpty()
        ? i18n("Google Groupware has been logged out from your account. Please log in to enable Google Contacts and Calendar sync again.")
        : i18n("Google Groupware has been logged out from account %1. Please log in to enable Google Contacts and Calendar sync again.",
               account->accountName());

    auto ntf = KNotification::event(QStringLiteral("authNeeded"),
                                    i18nc("@title", "%1 needs your attention.", agentName()),
                                    msg,
                                    QStringLiteral("im-google"),
                                    /*widget=*/nullptr,
                                    KNotification::Persistent | KNotification::SkipGrouping);
    ntf->setActions({i18nc("@action", "Log in")});
    ntf->setComponentName(QStringLiteral("akonadi_google_resource"));
    connect(ntf, &KNotification::action1Activated, this, [this, ntf, account, args]() {
        runAuthJob(account, args);
        ntf->close();
    });
    connect(ntf, &KNotification::ignored, ntf, &KNotification::close);
    ntf->sendEvent();
    qCDebug(GOOGLE_LOG) << "Prompting notification" << ntf->id() << " to ask user to reauthenticate";
}

bool GoogleResource::canPerformTask()
{
    if (!m_settings->accountPtr() && accountId() == 0) {
        cancelTask(i18nc("@info:status", "Resource is not configured"));
        Q_EMIT status(NotConfigured, i18nc("@info:status", "Resource is not configured"));
        return false;
    }

    return true;
}

void GoogleResource::slotAuthJobFinished(KGAPI2::Job *job)
{
    if (job->error() == KGAPI2::BadRequest) {
        auto account = KGAPI2::AccountPtr::create();
        account->setScopes(scopes());
        requestAuthenticationFromUser(account, job->property(JOB_PROPERTY));
        return;
    } else if (job->error() != KGAPI2::NoError) {
        cancelTask(i18n("Failed to refresh tokens"));
        return;
    }

    Q_EMIT status(Running);

    auto authJob = qobject_cast<AuthJob *>(job);
    AccountPtr account = authJob->account();
    auto writeJob = m_settings->storeAccount(account);
    connect(writeJob, &WritePasswordJob::finished, this, [job, account, writeJob]() {
        if (writeJob->error()) {
            qCWarning(GOOGLE_LOG) << "Failed to store account's password in secret storage" << writeJob->errorString();
        }

        auto otherJob = job->property(JOB_PROPERTY).value<KGAPI2::Job *>();
        if (otherJob) {
            otherJob->setAccount(account);
            otherJob->restart();
        }
    });
}

int GoogleResource::accountId() const
{
    return 0;
}

GenericHandler *GoogleResource::fetchHandlerByMimetype(const QString &mimeType)
{
    auto it = std::find_if(m_handlers.cbegin(), m_handlers.cend(), [&mimeType](const GenericHandler::Ptr &handler) {
        return handler->mimeType() == mimeType;
    });

    if (it != m_handlers.cend()) {
        return it->get();
    } else {
        return nullptr;
    }
}

GenericHandler *GoogleResource::fetchHandlerForCollection(const Akonadi::Collection &collection)
{
    auto it = std::find_if(m_handlers.cbegin(), m_handlers.cend(), [&collection](const GenericHandler::Ptr &handler) {
        return collection.contentMimeTypes().contains(handler->mimeType());
    });
    if (it != m_handlers.cend()) {
        return it->get();
    } else {
        return nullptr;
    }
}

/*
 * FreeBusy handling
 */

QDateTime GoogleResource::lastCacheUpdate() const
{
    if (m_freeBusyHandler) {
        return m_freeBusyHandler->lastCacheUpdate();
    }
    return {};
}

void GoogleResource::canHandleFreeBusy(const QString &email) const
{
    if (m_freeBusyHandler) {
        m_freeBusyHandler->canHandleFreeBusy(email);
    } else {
        handlesFreeBusy(email, false);
    }
}

void GoogleResource::retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end)
{
    if (m_freeBusyHandler) {
        m_freeBusyHandler->retrieveFreeBusy(email, start, end);
    } else {
        freeBusyRetrieved(email, QString(), false, QString());
    }
}

/*
 * Collection handling
 */
void GoogleResource::retrieveCollections()
{
    if (!canPerformTask()) {
        return;
    }
    qCDebug(GOOGLE_LOG) << "Retrieve Collections";

    setCollectionStreamingEnabled(true);
    CachePolicy cachePolicy;
    if (m_settings->enableIntervalCheck()) {
        cachePolicy.setInheritFromParent(false);
        cachePolicy.setIntervalCheckTime(m_settings->intervalCheckTime());
    }

    // Setting up root collection
    m_rootCollection = Collection();
    m_rootCollection.setContentMimeTypes({Collection::mimeType(), Collection::virtualMimeType()});
    m_rootCollection.setRemoteId(ROOT_COLLECTION_REMOTEID);
    m_rootCollection.setName(m_settings->accountPtr()->accountName());
    m_rootCollection.setParentCollection(Collection::root());
    m_rootCollection.setRights(Collection::CanCreateCollection);
    m_rootCollection.setCachePolicy(cachePolicy);

    auto attr = m_rootCollection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(m_settings->accountPtr()->accountName());
    attr->setIconName(QStringLiteral("im-google"));

    collectionsRetrieved({m_rootCollection});

    m_jobs = m_handlers.size();
    for (const auto &handler : m_handlers) {
        handler->retrieveCollections(m_rootCollection);
    }
}

void GoogleResource::collectionsRetrievedFromHandler(const Collection::List &collections)
{
    collectionsRetrieved(collections);
    m_jobs--;
    if (m_jobs == 0) {
        qCDebug(GOOGLE_LOG) << "All collections retrieved!";
        collectionsRetrievalDone();
        // taskDone(); // ???
        emitReadyStatus();
    }
}

void GoogleResource::retrieveItems(const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }

    auto handler = fetchHandlerForCollection(collection);
    if (handler) {
        handler->retrieveItems(collection);
    } else {
        qCWarning(GOOGLE_LOG) << "Unknown collection" << collection.name();
        itemsRetrieved({});
    }
}

bool GoogleResource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts);

    if (!canPerformTask()) {
        return false;
    }

    QSet<Akonadi::Collection> collections;

    for (const auto &item : items) {
        collections.insert(item.parentCollection());
    }

    bool ok = true;

    for (const auto &collection : std::as_const(collections)) {
        auto handler = fetchHandlerForCollection(collection);
        if (handler) {
            handler->retrieveItems(collection);
        } else {
            qCWarning(GOOGLE_LOG) << "Unknown collection" << collection.name();
            ok = false;
        }
    }

    return ok;
}

void GoogleResource::itemAdded(const Item &item, const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }

    auto handler = fetchHandlerByMimetype(item.mimeType());
    if (handler && handler->canPerformTask(item)) {
        handler->itemAdded(item, collection);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not add item" << item.mimeType();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::itemChanged(const Item &item, const QSet<QByteArray> &partIdentifiers)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerByMimetype(item.mimeType());
    if (handler && handler->canPerformTask(item)) {
        handler->itemChanged(item, partIdentifiers);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not change item" << item.mimeType();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::itemsRemoved(const Item::List &items)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerByMimetype(items.first().mimeType());
    if (handler) {
        handler->itemsRemoved(items);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not remove item" << items.first().mimeType();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::itemsMoved(const Item::List &items, const Collection &collectionSource, const Collection &collectionDestination)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerByMimetype(items.first().mimeType());
    if (handler && handler->canPerformTask(items)) {
        handler->itemsMoved(items, collectionSource, collectionDestination);
    } else if (!handler) {
        qCWarning(GOOGLE_LOG) << "Could not move item" << items.first().mimeType() << "from" << collectionSource.remoteId() << "to"
                              << collectionDestination.remoteId();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::itemsLinked(const Item::List &items, const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerByMimetype(items.first().mimeType());
    if (handler && handler->canPerformTask(items)) {
        handler->itemsLinked(items, collection);
    } else if (!handler) {
        qCWarning(GOOGLE_LOG) << "Could not link item" << items.first().mimeType() << "to" << collection.remoteId();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::itemsUnlinked(const Item::List &items, const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerByMimetype(items.first().mimeType());
    if (handler && handler->canPerformTask(items)) {
        handler->itemsUnlinked(items, collection);
    } else if (!handler) {
        qCWarning(GOOGLE_LOG) << "Could not unlink item mimetype" << items.first().mimeType() << "from" << collection.remoteId();
        cancelTask(i18n("Invalid payload type"));
    }
}

void GoogleResource::collectionAdded(const Collection &collection, const Collection &parent)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerForCollection(collection);
    if (handler) {
        handler->collectionAdded(collection, parent);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not add collection" << collection.displayName() << "mimetypes:" << collection.contentMimeTypes();
        cancelTask(i18n("Unknown collection mimetype"));
    }
}

void GoogleResource::collectionChanged(const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerForCollection(collection);
    if (handler) {
        handler->collectionChanged(collection);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not change collection" << collection.displayName() << "mimetypes:" << collection.contentMimeTypes();
        cancelTask(i18n("Unknown collection mimetype"));
    }
}

void GoogleResource::collectionRemoved(const Collection &collection)
{
    if (!canPerformTask()) {
        return;
    }
    auto handler = fetchHandlerForCollection(collection);
    if (handler) {
        handler->collectionRemoved(collection);
    } else {
        qCWarning(GOOGLE_LOG) << "Could not remove collection" << collection.displayName() << "mimetypes:" << collection.contentMimeTypes();
        cancelTask(i18n("Unknown collection mimetype"));
    }
}

AKONADI_RESOURCE_MAIN(GoogleResource)
