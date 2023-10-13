/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "etesyncresource.h"

#include <KContacts/Addressee>
#include <kwindowsystem.h>

#include <Akonadi/AttributeFactory>
#include <Akonadi/CachePolicy>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionColorAttribute>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>
#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>
#include <KMessageBox>
#include <QDBusConnection>

#include "entriesfetchjob.h"
#include "etesync_debug.h"
#include "journalsfetchjob.h"
#include "settings.h"
#include "settingsadaptor.h"
#include "setupwizard.h"

using namespace EteSyncAPI;
using namespace Akonadi;

#define ROOT_COLLECTION_REMOTEID QStringLiteral("EteSyncRootCollection")
// Resource offline time for temporary errors (30 min)
#define SHORT_OFFLINE_TIME 30 * 60
// Resource offline time for major errors (4 hours)
#define LONG_OFFLINE_TIME 4 * 60 * 60

EteSyncResource::EteSyncResource(const QString &id)
    : ResourceBase(id)
{
    Settings::instance(KSharedConfig::openConfig());
    new SettingsAdaptor(Settings::self());

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), Settings::self(), QDBusConnection::ExportAdaptors);

    setName(i18n("EteSync Resource"));

    setNeedsNetwork(true);

    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::All);
    changeRecorder()->fetchCollection(true);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(CollectionFetchScope::All);

    // Make resource directory
    initialiseDirectory(baseDirectoryPath());

    mClientState = EteSyncClientState::Ptr(new EteSyncClientState(identifier(), winIdForDialogs()));
    connect(mClientState.get(), &EteSyncClientState::clientInitialised, this, &EteSyncResource::initialiseDone);
    mClientState->init();

    AttributeFactory::registerAttribute<CollectionColorAttribute>();

    connect(this, &Akonadi::AgentBase::reloadConfiguration, this, &EteSyncResource::onReloadConfiguration);

    qCDebug(ETESYNC_LOG) << "Resource started";
}

void EteSyncResource::cleanup()
{
    mClientState->logout();
    mClientState->deleteEtebaseUserCache();
    ResourceBase::cleanup();
}

void EteSyncResource::configure(WId windowId)
{
    SetupWizard wizard(mClientState.get());

    if (windowId) {
        wizard.setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(wizard.windowHandle(), windowId);
    }
    const int result = wizard.exec();
    if (result == QDialog::Accepted) {
        mClientState->saveSettings();

        // Save account cache
        mClientState->saveAccount();

        mCredentialsRequired = false;
        qCDebug(ETESYNC_LOG) << "Setting online";
        setOnline(true);
        synchronize();
        Q_EMIT configurationDialogAccepted();
    } else {
        qCDebug(ETESYNC_LOG) << "Setting offline";
        setOnline(false);
        Q_EMIT configurationDialogRejected();
    }
}

void EteSyncResource::retrieveCollections()
{
    qCDebug(ETESYNC_LOG) << "Retrieving collections";

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    mRootCollection = createRootCollection();

    auto job = new JournalsFetchJob(mClientState.get(), mRootCollection, this);
    connect(job, &JournalsFetchJob::finished, this, &EteSyncResource::slotCollectionsRetrieved);
    job->start();
}

Collection EteSyncResource::createRootCollection()
{
    Collection rootCollection = Collection();
    rootCollection.setContentMimeTypes({Collection::mimeType()});
    rootCollection.setName(mClientState->username());
    rootCollection.setRemoteId(ROOT_COLLECTION_REMOTEID);
    rootCollection.setParentCollection(Collection::root());
    rootCollection.setRights(Collection::CanCreateCollection);

    // Keep collection list stoken preserved
    rootCollection.setRemoteRevision(mRootCollection.remoteRevision());

    Akonadi::CachePolicy cachePolicy;
    cachePolicy.setInheritFromParent(false);
    cachePolicy.setSyncOnDemand(false);
    cachePolicy.setCacheTimeout(-1);
    cachePolicy.setIntervalCheckTime(5);
    rootCollection.setCachePolicy(cachePolicy);

    EntityDisplayAttribute *attr = rootCollection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(mClientState->username());
    attr->setIconName(QStringLiteral("akonadi-etesync"));

    return rootCollection;
}

void EteSyncResource::slotCollectionsRetrieved(KJob *job)
{
    if (job->error()) {
        qCWarning(ETESYNC_LOG) << "Error in fetching journals";
        qCWarning(ETESYNC_LOG) << "EteSync error" << job->error() << job->errorText();
        handleError(job->error(), job->errorText());
        return;
    }

    qCDebug(ETESYNC_LOG) << "slotCollectionsRetrieved()";

    const QString sToken = qobject_cast<JournalsFetchJob *>(job)->syncToken();
    mRootCollection.setRemoteRevision(sToken);

    Collection::List collections = {mRootCollection};
    collections.append(qobject_cast<JournalsFetchJob *>(job)->collections());
    const Collection::List removedCollections = qobject_cast<JournalsFetchJob *>(job)->removedCollections();

    collectionsRetrievedIncremental(collections, removedCollections);

    mJournalsCacheUpdateTime = QDateTime::currentDateTime();

    qCDebug(ETESYNC_LOG) << "Collections retrieval done";
}

bool EteSyncResource::handleError(const int errorCode, const QString &errorMessage)
{
    qCDebug(ETESYNC_LOG) << "handleError" << errorCode << errorMessage;
    switch (errorCode) {
    case ETEBASE_ERROR_CODE_UNAUTHORIZED:
        qCDebug(ETESYNC_LOG) << "Invalid token";
        deferTask();
        connect(mClientState.get(), &EteSyncClientState::tokenRefreshed, this, &EteSyncResource::slotTokenRefreshed);
        scheduleCustomTask(mClientState.get(), "refreshToken", QVariant(), ResourceBase::Prepend);
        return true;
    case ETEBASE_ERROR_CODE_TEMPORARY_SERVER_ERROR:
        qCDebug(ETESYNC_LOG) << "Temporary server error";
        qCDebug(ETESYNC_LOG) << "Setting resource offline for" << SHORT_OFFLINE_TIME << "seconds";
        setTemporaryOffline(SHORT_OFFLINE_TIME);
        cancelTask(i18n("Temporary server error"));
        return true;
    case ETEBASE_ERROR_CODE_PERMISSION_DENIED:
        qCDebug(ETESYNC_LOG) << "Permission denied";
        showErrorDialog(i18n("You do not have permission to perform this action."), i18n(charArrFromQString(errorMessage)));
        qCDebug(ETESYNC_LOG) << "Setting resource offline for" << LONG_OFFLINE_TIME << "seconds";
        setTemporaryOffline(LONG_OFFLINE_TIME);
        cancelTask(i18n("Permission denied"));
        return true;
    case ETEBASE_ERROR_CODE_SERVER_ERROR:
        qCDebug(ETESYNC_LOG) << "Server error";
        showErrorDialog(i18n("A server error occurred."), i18n(charArrFromQString(errorMessage)));
        qCDebug(ETESYNC_LOG) << "Setting resource offline for" << LONG_OFFLINE_TIME << "seconds";
        setTemporaryOffline(LONG_OFFLINE_TIME);
        cancelTask(i18n("Server error"));
        return true;
    default:
        qCDebug(ETESYNC_LOG) << "Cancelling task";
        cancelTask();
        return true;
    }
    return false;
}

bool EteSyncResource::handleError()
{
    return handleError(etebase_error_get_code(), QString::fromUtf8(etebase_error_get_message()));
}

bool EteSyncResource::credentialsRequired()
{
    if (mCredentialsRequired) {
        qCDebug(ETESYNC_LOG) << "Credentials required";
        showErrorDialog(i18n("Your EteSync credentials were changed. Please click OK to re-enter your credentials."),
                        i18n(etebase_error_get_message()),
                        i18n("Credentials Changed"));
        configure(winIdForDialogs());
    }
    return mCredentialsRequired;
}

void EteSyncResource::slotTokenRefreshed(bool successful)
{
    qCDebug(ETESYNC_LOG) << "slotTokenRefreshed" << successful;
    if (!successful) {
        if (etebase_error_get_code() == ETEBASE_ERROR_CODE_UNAUTHORIZED) {
            qCDebug(ETESYNC_LOG) << "Unauthorized for tokenRefresh";
            mCredentialsRequired = true;
        }
    }
    taskDone();
}

void EteSyncResource::showErrorDialog(const QString &errorText, const QString &errorDetails, const QString &title)
{
    QWidget *parent = QWidget::find(winIdForDialogs());
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    dialog->setAttribute(Qt::WA_NativeWindow, true);
    KWindowSystem::setMainWindow(dialog->windowHandle(), winIdForDialogs());
    KMessageBox::detailedError(dialog, errorText, errorDetails, title);
}

QString getEtebaseTypeForCollection(const Akonadi::Collection &collection)
{
    QStringList mimeTypes = collection.contentMimeTypes();
    if (mimeTypes.contains(KContacts::Addressee::mimeType())) {
        return ETEBASE_COLLECTION_TYPE_CALENDAR;
    } else if (mimeTypes.contains(KCalendarCore::Event::eventMimeType())) {
        return ETEBASE_COLLECTION_TYPE_CALENDAR;
    } else if (mimeTypes.contains(KCalendarCore::Todo::todoMimeType())) {
        return ETEBASE_COLLECTION_TYPE_TASKS;
    } else {
        qCDebug(ETESYNC_LOG) << "Unable to get Etebase collection type for collection" << collection.remoteId() << mimeTypes;
        return QString();
    }
}

void EteSyncResource::retrieveItems(const Akonadi::Collection &collection)
{
    qCDebug(ETESYNC_LOG) << "Retrieving items for collection" << collection.remoteId();

    if (!mClientState->account()) {
        qCDebug(ETESYNC_LOG) << "Cannot retrieve items - account is null";
        cancelTask(i18n("Cannot retrieve items - account is null"));
        return;
    }

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }

    const int timeSinceLastCacheUpdate = mJournalsCacheUpdateTime.secsTo(QDateTime::currentDateTime());
    if (timeSinceLastCacheUpdate <= 30) {
        qCDebug(ETESYNC_LOG) << "Retrieve items called immediately after collection fetch";

        const QString sToken = QString::fromUtf8(etebase_collection_get_stoken(etesyncCollection.get()));
        qCDebug(ETESYNC_LOG) << "Comparing" << sToken << "and" << collection.remoteRevision();
        if (sToken == collection.remoteRevision()) {
            qCDebug(ETESYNC_LOG) << "Already up-to-date: Fetched collection and cached collection have the same stoken";
            itemsRetrievalDone();
            return;
        }
    }

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    auto job = new EntriesFetchJob(mClientState.get(), collection, std::move(etesyncCollection), this);

    connect(job, &EntriesFetchJob::finished, this, &EteSyncResource::slotItemsRetrieved);

    job->start();
}

void EteSyncResource::slotItemsRetrieved(KJob *job)
{
    if (job->error()) {
        qCDebug(ETESYNC_LOG) << "Error in fetching entries";
        qCWarning(ETESYNC_LOG) << "EteSync error" << job->error() << job->errorText();
        handleError(job->error(), job->errorText());
    }

    Item::List items = qobject_cast<EntriesFetchJob *>(job)->items();
    Item::List removedItems = qobject_cast<EntriesFetchJob *>(job)->removedItems();

    qCDebug(ETESYNC_LOG) << "Updating collection sync token";
    Collection collection = qobject_cast<EntriesFetchJob *>(job)->collection();
    qCDebug(ETESYNC_LOG) << "Setting collection" << collection.remoteId() << "'s sync token to" << collection.remoteRevision();
    new CollectionModifyJob(collection, this);

    itemsRetrievedIncremental(items, removedItems);

    qCDebug(ETESYNC_LOG) << "Items retrieval done";
}

void EteSyncResource::aboutToQuit()
{
}

void EteSyncResource::onReloadConfiguration()
{
    qCDebug(ETESYNC_LOG) << "Resource config reload";
    synchronize();
}

void EteSyncResource::initialiseDone(bool successful)
{
    qCDebug(ETESYNC_LOG) << "Resource initialised";
    if (successful) {
        synchronize();
    }
}

QString EteSyncResource::baseDirectoryPath() const
{
    return Settings::self()->basePath();
}

void EteSyncResource::initialiseDirectory(const QString &path) const
{
    QDir dir(path);

    // if folder does not exists, create it
    QDir::root().mkpath(dir.absolutePath());

    // check whether warning file is in place...
    QFile file(dir.absolutePath() + QStringLiteral("/WARNING_README.txt"));
    if (!file.exists()) {
        // ... if not, create it
        file.open(QIODevice::WriteOnly);
        file.write(
            "Important warning!\n\n"
            "Do not create or copy files inside this folder manually, they are managed by the Akonadi framework!\n");
        file.close();
    }
}

void EteSyncResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    qCDebug(ETESYNC_LOG) << "Item added" << item.mimeType();
    qCDebug(ETESYNC_LOG) << "Journal UID" << collection.remoteId();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncCollection from cache" << collection.remoteId();
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }
    const QString type = QString::fromUtf8(etebase_collection_get_collection_type(etesyncCollection.get()));

    // Create metadata
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    EtebaseItemMetadataPtr itemMetaData(etebase_item_metadata_new());
    QString uid;
    if (type == ETEBASE_COLLECTION_TYPE_ADDRESS_BOOK) {
        uid = item.payload<KContacts::Addressee>().uid();
    } else {
        uid = item.payload<KCalendarCore::Incidence::Ptr>()->uid();
    }
    etebase_item_metadata_set_name(itemMetaData.get(), charArrFromQString(uid));
    etebase_item_metadata_set_mtime(itemMetaData.get(), &modificationTimeSinceEpoch);

    qCDebug(ETESYNC_LOG) << "Created metadata";

    // Get item manager
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), etesyncCollection.get()));

    // Create Etesync item
    QByteArray payloadData = item.payloadData();
    EtebaseItemPtr etesyncItem(etebase_item_manager_create(itemManager.get(), itemMetaData.get(), payloadData.constData(), payloadData.size()));
    if (!etesyncItem) {
        qCDebug(ETESYNC_LOG) << "Could not create new etesyncItem" << uid;
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        cancelTask(i18n("Could not create new etesyncItem '%1'", uid));
        return;
    }

    qCDebug(ETESYNC_LOG) << "Created EteSync item";

    // Upload to server
    const EtebaseItem *items[] = {etesyncItem.get()};

    if (etebase_item_manager_batch(itemManager.get(), items, ETEBASE_UTILS_C_ARRAY_LEN(items), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading item addition" << uid;
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded item addition to server";
    }

    // Save to cache
    mClientState->saveEtebaseItemCache(etesyncItem.get(), etesyncCollection.get());

    Item newItem(item);
    newItem.setRemoteId(QString::fromUtf8(etebase_item_get_uid(etesyncItem.get())));
    newItem.setPayloadFromData(payloadData);
    changeCommitted(newItem);
}

void EteSyncResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    qCDebug(ETESYNC_LOG) << "Item changed" << item.mimeType() << item.remoteId();
    qCDebug(ETESYNC_LOG) << "Journal UID" << item.parentCollection().remoteId();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    Collection collection = item.parentCollection();

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncCollection from cache" << collection.remoteId();
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), etesyncCollection.get()));
    EtebaseItemPtr etesyncItem = mClientState->getEtebaseItemFromCache(item.remoteId(), etesyncCollection.get());
    if (!etesyncItem) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncItem from cache" << item.remoteId();
        cancelTask(i18n("Could not get etesyncItem from cache '%1'", item.remoteId()));
        return;
    }

    // Update metadata (only mtime in this case)
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    EtebaseItemMetadataPtr itemMetadata(etebase_item_get_meta(etesyncItem.get()));
    etebase_item_metadata_set_mtime(itemMetadata.get(), &modificationTimeSinceEpoch);
    etebase_item_set_meta(etesyncItem.get(), itemMetadata.get());

    qCDebug(ETESYNC_LOG) << "Updated metadata mtime";

    // Update content
    QByteArray payloadData = item.payloadData();
    etebase_item_set_content(etesyncItem.get(), payloadData.constData(), payloadData.size());

    qCDebug(ETESYNC_LOG) << "Updated item content";

    // Upload to server
    const EtebaseItem *items[] = {etesyncItem.get()};

    if (etebase_item_manager_batch(itemManager.get(), items, ETEBASE_UTILS_C_ARRAY_LEN(items), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading item modifications" << item.remoteId();
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded item modifications to server";
    }

    // Update cache
    mClientState->saveEtebaseItemCache(etesyncItem.get(), etesyncCollection.get());

    changeProcessed();
}

void EteSyncResource::itemRemoved(const Akonadi::Item &item)
{
    qCDebug(ETESYNC_LOG) << "Item removed" << item.mimeType();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    Collection collection = item.parentCollection();

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncCollection from cache" << collection.remoteId();
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), etesyncCollection.get()));
    EtebaseItemPtr etesyncItem = mClientState->getEtebaseItemFromCache(item.remoteId(), etesyncCollection.get());
    if (!etesyncItem) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncItem from cache" << item.remoteId();
        cancelTask(i18n("Could not get etesyncItem from cache '%1'", item.remoteId()));
        return;
    }

    // Update metadata (only mtime in this case)
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    EtebaseItemMetadataPtr itemMetadata(etebase_item_get_meta(etesyncItem.get()));
    etebase_item_metadata_set_mtime(itemMetadata.get(), &modificationTimeSinceEpoch);
    etebase_item_set_meta(etesyncItem.get(), itemMetadata.get());

    qCDebug(ETESYNC_LOG) << "Updated metadata mtime";

    // Set item deleted
    etebase_item_delete(etesyncItem.get());

    qCDebug(ETESYNC_LOG) << "Set item deleted";

    // Upload to server
    const EtebaseItem *items[] = {etesyncItem.get()};

    if (etebase_item_manager_batch(itemManager.get(), items, ETEBASE_UTILS_C_ARRAY_LEN(items), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading item deletion" << item.remoteId();
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded item deletion to server";
    }

    // Delete cache
    mClientState->deleteEtebaseItemCache(item.remoteId(), etesyncCollection.get());

    changeProcessed();
}

void EteSyncResource::collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent)
{
    Q_UNUSED(parent)

    qCDebug(ETESYNC_LOG) << "Collection added" << collection.mimeType();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));

    // Create metadata
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    const QString type = getEtebaseTypeForCollection(collection);
    EtebaseItemMetadataPtr collectionMetaData(etebase_item_metadata_new());
    etebase_item_metadata_set_name(collectionMetaData.get(), collection.displayName());
    etebase_item_metadata_set_mtime(collectionMetaData.get(), &modificationTimeSinceEpoch);

    qCDebug(ETESYNC_LOG) << "Created metadata";

    // Create EteSync collection
    EtebaseCollectionPtr etesyncCollection(etebase_collection_manager_create(collectionManager.get(), type, collectionMetaData.get(), nullptr, 0));
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not create new etesyncCollection";
        qCDebug(ETESYNC_LOG) << "Etebase error;" << etebase_error_get_message();
        cancelTask(i18n("Could not create new etesyncCollection"));
        return;
    }

    qCDebug(ETESYNC_LOG) << "Created EteSync collection";

    // Upload to server
    if (etebase_collection_manager_upload(collectionManager.get(), etesyncCollection.get(), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading collection addition";
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded collection addition to server";
    }

    // Save to cache
    mClientState->saveEtebaseCollectionCache(etesyncCollection.get());

    // Setup icon, color and name of the new collection
    Collection newCollection(collection);
    newCollection.setRemoteId(QString::fromUtf8(etebase_collection_get_uid(etesyncCollection.get())));

    // Icon and name
    auto attr = newCollection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);

    if (type == ETEBASE_COLLECTION_TYPE_ADDRESS_BOOK) {
        attr->setDisplayName(collection.displayName());
        attr->setIconName(QStringLiteral("view-pim-contacts"));
    } else if (type == ETEBASE_COLLECTION_TYPE_CALENDAR) {
        attr->setDisplayName(collection.displayName());
        attr->setIconName(QStringLiteral("view-calendar"));
    } else if (type == ETEBASE_COLLECTION_TYPE_TASKS) {
        attr->setDisplayName(collection.displayName());
        attr->setIconName(QStringLiteral("view-pim-tasks"));
    } else {
        qCWarning(ETESYNC_LOG) << "Unknown journal type. Cannot set collection name and icon.";
    }

    // Color
    auto colorAttr = newCollection.attribute<Akonadi::CollectionColorAttribute>(Collection::AddIfMissing);
    colorAttr->setColor(ETESYNC_DEFAULT_COLLECTION_COLOR);

    changeCommitted(newCollection);
}

void EteSyncResource::collectionChanged(const Akonadi::Collection &collection)
{
    qCDebug(ETESYNC_LOG) << "Collection changed" << collection.mimeType();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    // Get EteSync collection from cache
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncCollection from cache" << collection.remoteId();
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }

    // Update metadata
    EtebaseItemMetadataPtr collectionMetaData(etebase_collection_get_meta(etesyncCollection.get()));

    // mtime
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    etebase_item_metadata_set_mtime(collectionMetaData.get(), &modificationTimeSinceEpoch);

    // Name
    etebase_item_metadata_set_name(collectionMetaData.get(), collection.displayName());

    // Color
    auto journalColor = ETESYNC_DEFAULT_COLLECTION_COLOR;
    if (collection.hasAttribute<CollectionColorAttribute>()) {
        const CollectionColorAttribute *colorAttr = collection.attribute<CollectionColorAttribute>();
        if (colorAttr) {
            journalColor = colorAttr->color().name();
        }
    }
    etebase_item_metadata_set_color(collectionMetaData.get(), journalColor);

    // Set metadata
    etebase_collection_set_meta(etesyncCollection.get(), collectionMetaData.get());

    // Upload to server
    if (etebase_collection_manager_upload(collectionManager.get(), etesyncCollection.get(), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading collection modifications" << collection.remoteId();
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded collection modifications to server";
    }

    // Update cache
    mClientState->saveEtebaseCollectionCache(etesyncCollection.get());

    Collection newCollection(collection);
    changeCommitted(newCollection);
}

void EteSyncResource::collectionRemoved(const Akonadi::Collection &collection)
{
    qCDebug(ETESYNC_LOG) << "Collection removed" << collection.mimeType();

    if (credentialsRequired()) {
        deferTask();
        return;
    }

    // Get EteSync collection from cache
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));
    EtebaseCollectionPtr etesyncCollection = mClientState->getEtebaseCollectionFromCache(collection.remoteId());
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not get etesyncCollection from cache" << collection.remoteId();
        cancelTask(i18n("Could not get etesyncCollection from cache '%1'", collection.remoteId()));
        return;
    }

    // Update metadata
    EtebaseItemMetadataPtr collectionMetaData(etebase_collection_get_meta(etesyncCollection.get()));

    // mtime
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    etebase_item_metadata_set_mtime(collectionMetaData.get(), &modificationTimeSinceEpoch);

    // Set collection deleted
    etebase_collection_delete(etesyncCollection.get());

    // Upload to server
    if (etebase_collection_manager_upload(collectionManager.get(), etesyncCollection.get(), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading collection deletion" << collection.remoteId();
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        handleError();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded collection deletion to server";
    }

    // Delete cache
    mClientState->deleteEtebaseCollectionCache(collection.remoteId());

    changeProcessed();
}

AKONADI_RESOURCE_MAIN(EteSyncResource)
