/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboynotesresource.h"
#include "debug.h"
#include "settings.h"
#include "settingsadaptor.h"
#include "tomboycollectionsdownloadjob.h"
#include "tomboyitemdownloadjob.h"
#include "tomboyitemsdownloadjob.h"
#include "tomboyitemuploadjob.h"
#include "tomboyserverauthenticatejob.h"
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <KLocalizedString>
#include <QDBusConnection>
#include <QSslCipher>

using namespace Akonadi;

TomboyNotesResource::TomboyNotesResource(const QString &id)
    : ResourceBase(id)
{
    Settings::instance(KSharedConfig::openConfig());
    new SettingsAdaptor(Settings::self());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), Settings::self(), QDBusConnection::ExportAdaptors);

    // Akonadi:Item should always provide the payload
    changeRecorder()->itemFetchScope().fetchFullPayload(true);

    // Status message stuff
    mStatusMessageTimer = new QTimer(this);
    mStatusMessageTimer->setSingleShot(true);
    connect(mStatusMessageTimer, &QTimer::timeout, [=]() {
        Q_EMIT status(Akonadi::AgentBase::Idle, QString());
    });
    connect(this, &AgentBase::error, this, &TomboyNotesResource::showError);

    mUploadJobProcessRunning = false;

    mManager = new QNetworkAccessManager(this);
    mManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    mManager->setStrictTransportSecurityEnabled(true);
    mManager->enableStrictTransportSecurityStore(true);
    connect(mManager, &QNetworkAccessManager::sslErrors, this, &TomboyNotesResource::onSslError);

    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Resource started";
    connect(this, &TomboyNotesResource::reloadConfiguration, this, &TomboyNotesResource::slotReloadConfig);
}

TomboyNotesResource::~TomboyNotesResource() = default;

void TomboyNotesResource::retrieveCollections()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving collections started";

    if (configurationNotValid()) {
        cancelTask(i18n("Resource configuration is not valid"));
        return;
    }

    int refreshInterval = Settings::self()->refreshInterval();
    if (refreshInterval == 0) {
        refreshInterval = -1;
    }

    auto job = new TomboyCollectionsDownloadJob(Settings::collectionName(), mManager, refreshInterval, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::userURL());
    // connect to its result() signal
    connect(job, &KJob::result, this, &TomboyNotesResource::onCollectionsRetrieved);
    job->start();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving collections job started";
}

void TomboyNotesResource::retrieveItems(const Akonadi::Collection &collection)
{
    if (configurationNotValid()) {
        cancelTask(i18n("Resource configuration is not valid"));
        return;
    }

    // create the job
    auto job = new TomboyItemsDownloadJob(collection.id(), mManager, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::contentURL());
    // connect to its result() signal
    connect(job, &KJob::result, this, &TomboyNotesResource::onItemsRetrieved);
    job->start();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving items job started";
}

bool TomboyNotesResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    if (configurationNotValid()) {
        cancelTask(i18n("Resource configuration is not valid"));
        return false;
    }

    // this method is called when Akonadi wants more data for a given item.
    auto job = new TomboyItemDownloadJob(item, mManager, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::contentURL());
    // connect to its result() signal
    connect(job, &KJob::result, this, &TomboyNotesResource::onItemRetrieved);
    job->start();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving item data job started";

    return true;
}

void TomboyNotesResource::onAuthorizationFinished(KJob *kjob)
{
    // Saves the received client authentication data in the settings
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Authorization job finished";
    auto job = qobject_cast<TomboyServerAuthenticateJob *>(kjob);
    if (job->error() == TomboyJobError::NoError) {
        Settings::setRequestToken(job->getRequestToken());
        Settings::setRequestTokenSecret(job->getRequestTokenSecret());
        Settings::setContentURL(job->getContentUrl());
        Settings::setUserURL(job->getUserURL());
        Settings::self()->save();
        synchronizeCollectionTree();
        synchronize();
    } else {
        showError(job->errorText());
    }
}

void TomboyNotesResource::onCollectionsRetrieved(KJob *kjob)
{
    auto job = qobject_cast<TomboyCollectionsDownloadJob *>(kjob);
    if (job->error() != TomboyJobError::NoError) {
        cancelTask();
        showError(job->errorText());
        return;
    }

    collectionsRetrieved(job->collections());
}

void TomboyNotesResource::onItemChangeCommitted(KJob *kjob)
{
    auto job = qobject_cast<TomboyItemUploadJob *>(kjob);
    mUploadJobProcessRunning = false;
    switch (job->error()) {
    case TomboyJobError::PermanentError:
        cancelTask();
        showError(job->errorText());
        return;
    case TomboyJobError::TemporaryError:
        retryAfterFailure(job->errorString());
        return;
    case TomboyJobError::NoError:
        changeCommitted(job->item());
        // The data should be actualized for the next UploadJob
        synchronize();
        return;
    }
}

void TomboyNotesResource::onItemRetrieved(KJob *kjob)
{
    auto job = qobject_cast<TomboyItemDownloadJob *>(kjob);

    if (job->error() != TomboyJobError::NoError) {
        cancelTask();
        showError(job->errorText());
        return;
    }

    itemRetrieved(job->item());
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving item data job with remoteId " << job->item().remoteId() << " finished";
}

void TomboyNotesResource::onItemsRetrieved(KJob *kjob)
{
    auto job = qobject_cast<TomboyItemsDownloadJob *>(kjob);
    if (job->error() != TomboyJobError::NoError) {
        cancelTask();
        showError(job->errorText());
        return;
    }

    itemsRetrieved(job->items());
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Retrieving items job finished";
}

void TomboyNotesResource::onSslError(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(errors)
    if (Settings::ignoreSslErrors()) {
        reply->ignoreSslErrors();
    }
}

void TomboyNotesResource::aboutToQuit()
{
    // TODO: any cleanup you need to do while there is still an active
    // event loop. The resource will terminate after this method returns
}

void TomboyNotesResource::slotReloadConfig()
{
    setAgentName(Settings::collectionName());

    if (configurationNotValid()) {
        auto job = new TomboyServerAuthenticateJob(mManager, this);
        job->setServerURL(Settings::serverURL(), QString());
        connect(job, &KJob::result, this, &TomboyNotesResource::onAuthorizationFinished);
        job->start();
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << "Authorization job started";
    } else {
        synchronize();
    }
}

void TomboyNotesResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)
    if (Settings::readOnly() || configurationNotValid()) {
        cancelTask(i18n("Resource is read-only"));
        return;
    }

    if (mUploadJobProcessRunning) {
        retryAfterFailure(QString());
        return;
    }

    auto job = new TomboyItemUploadJob(item, JobType::AddItem, mManager, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::contentURL());
    connect(job, &KJob::result, this, &TomboyNotesResource::onItemChangeCommitted);
    mUploadJobProcessRunning = true;
    job->start();
}

void TomboyNotesResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)
    if (Settings::readOnly() || configurationNotValid()) {
        cancelTask(i18n("Resource is read-only"));
        return;
    }

    if (mUploadJobProcessRunning) {
        retryAfterFailure(QString());
        return;
    }

    auto job = new TomboyItemUploadJob(item, JobType::ModifyItem, mManager, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::contentURL());
    connect(job, &KJob::result, this, &TomboyNotesResource::onItemChangeCommitted);
    mUploadJobProcessRunning = true;
    job->start();
}

void TomboyNotesResource::itemRemoved(const Akonadi::Item &item)
{
    if (Settings::readOnly() || configurationNotValid()) {
        cancelTask(i18n("Resource is read-only"));
        return;
    }

    if (mUploadJobProcessRunning) {
        retryAfterFailure(QString());
        return;
    }

    auto job = new TomboyItemUploadJob(item, JobType::DeleteItem, mManager, this);
    job->setAuthentication(Settings::requestToken(), Settings::requestTokenSecret());
    job->setServerURL(Settings::serverURL(), Settings::contentURL());
    connect(job, &KJob::result, this, &TomboyNotesResource::onItemChangeCommitted);
    mUploadJobProcessRunning = true;
    job->start();
}

bool TomboyNotesResource::configurationNotValid() const
{
    return Settings::requestToken().isEmpty() || Settings::contentURL().isEmpty();
}

void TomboyNotesResource::retryAfterFailure(const QString &errorMessage)
{
    Q_EMIT status(Broken, errorMessage);
    deferTask();
    setTemporaryOffline(Settings::self()->refreshInterval() <= 0 ? 300 : Settings::self()->refreshInterval() * 60);
}

void TomboyNotesResource::showError(const QString &errorText)
{
    Q_EMIT status(Akonadi::AgentBase::Idle, errorText);
    mStatusMessageTimer->start(1000 * 10);
}

AKONADI_RESOURCE_MAIN(TomboyNotesResource)

#include "moc_tomboynotesresource.cpp"
