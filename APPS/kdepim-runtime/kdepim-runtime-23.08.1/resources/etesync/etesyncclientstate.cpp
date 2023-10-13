/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "etesyncclientstate.h"

#include "etesync_debug.h"

using namespace KWallet;

static const QString etebaseWalletFolder = QStringLiteral("Akonadi Etebase");

EteSyncClientState::EteSyncClientState(const QString &agentId, WId winId)
    : mAgentId(agentId)
    , mWinId(winId)
{
}

void EteSyncClientState::init()
{
    // Load settings
    Settings::self()->load();
    mServerUrl = Settings::self()->baseUrl();
    mUsername = Settings::self()->username();

    if (mServerUrl.isEmpty() || mUsername.isEmpty()) {
        Q_EMIT clientInitialised(false);
        return;
    }

    // Initialize client object
    mClient = etebase_client_new(QStringLiteral("Akonadi EteSync Resource"), mServerUrl);
    if (!mClient) {
        qCDebug(ETESYNC_LOG) << "Could not initialise Etebase client";
        qCDebug(ETESYNC_LOG) << "Etebase error" << etebase_error_get_message();
        Q_EMIT clientInitialised(false);
        return;
    }

    // Initialize etebase file cache
    mEtebaseCache = etebase_fs_cache_new(Settings::self()->basePath(), mUsername + QStringLiteral("_") + mAgentId);

    // Load Etebase account from cache
    loadAccount();

    Q_EMIT clientInitialised(true);
}

bool EteSyncClientState::openWalletFolder()
{
    mWallet = Wallet::openWallet(Wallet::NetworkWallet(), mWinId, Wallet::Synchronous);
    if (mWallet) {
        qCDebug(ETESYNC_LOG) << "Wallet opened";
    } else {
        qCWarning(ETESYNC_LOG) << "Failed to open wallet!";
        return false;
    }
    if (!mWallet->hasFolder(etebaseWalletFolder) && !mWallet->createFolder(etebaseWalletFolder)) {
        qCWarning(ETESYNC_LOG) << "Failed to create wallet folder" << etebaseWalletFolder;
        return false;
    }

    if (!mWallet->setFolder(etebaseWalletFolder)) {
        qWarning() << "Failed to open wallet folder" << etebaseWalletFolder;
        return false;
    }
    qCDebug(ETESYNC_LOG) << "Wallet opened" << etebaseWalletFolder;
    return true;
}

bool EteSyncClientState::login(const QString &serverUrl, const QString &username, const QString &password)
{
    mServerUrl = serverUrl;
    mUsername = username;

    mClient = etebase_client_new(QStringLiteral("Akonadi EteSync Resource"), mServerUrl);
    if (!mClient) {
        qCDebug(ETESYNC_LOG) << "Could not initialise Etebase client";
        qCDebug(ETESYNC_LOG) << "Etebase error" << etebase_error_get_message();
        return false;
    }
    mAccount = etebase_account_login(mClient.get(), mUsername, password);
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Could not fetch Etebase account";
        qCDebug(ETESYNC_LOG) << "Etebase error" << etebase_error_get_message();
        return false;
    }
    mEtebaseCache = etebase_fs_cache_new(Settings::self()->basePath(), mUsername + QStringLiteral("_") + mAgentId);
    return true;
}

void EteSyncClientState::logout()
{
    if (etebase_account_logout(mAccount.get())) {
        qCDebug(ETESYNC_LOG) << "Could not logout";
    }
    deleteWalletEntry();
}

EteSyncClientState::AccountStatus EteSyncClientState::accountStatus()
{
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Could not fetch collection list with limit 1";
        qCDebug(ETESYNC_LOG) << "Etebase account is null";
        return ERROR;
    }
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    EtebaseFetchOptionsPtr fetchOptions(etebase_fetch_options_new());
    etebase_fetch_options_set_limit(fetchOptions.get(), 1);
    etebase_fetch_options_set_prefetch(fetchOptions.get(), ETEBASE_PREFETCH_OPTION_MEDIUM);

    EtebaseCollectionListResponsePtr collectionList(
        etebase_collection_manager_list_multi(collectionManager.get(), ETESYNC_COLLECTION_TYPES, ETESYNC_COLLECTION_TYPES_SIZE, fetchOptions.get()));
    if (!collectionList) {
        qCDebug(ETESYNC_LOG) << "Could not fetch collection list with limit 1";
        qCDebug(ETESYNC_LOG) << "Etebase error" << etebase_error_get_message();
        return ERROR;
    }

    uintptr_t dataLength = etebase_collection_list_response_get_data_length(collectionList.get());
    if (dataLength == 0) {
        return NEW_ACCOUNT;
    }
    return OK;
}

void EteSyncClientState::refreshToken()
{
    qCDebug(ETESYNC_LOG) << "Refreshing token";
    if (etebase_account_fetch_token(mAccount.get())) {
        tokenRefreshed(false);
        return;
    }
    tokenRefreshed(true);
    return;
}

void EteSyncClientState::saveSettings()
{
    Settings::self()->setBaseUrl(mServerUrl);
    Settings::self()->setUsername(mUsername);

    Settings::self()->save();
}

void EteSyncClientState::saveAccount()
{
    if (!mWallet) {
        qCDebug(ETESYNC_LOG) << "Save account - wallet not opened";
        if (!openWalletFolder()) {
            return;
        }
    }

    QByteArray encryptionKey(32, '\0');
    etebase_utils_randombytes(encryptionKey.data(), encryptionKey.size());
    if (mWallet->writeEntry(mUsername, encryptionKey)) {
        qCDebug(ETESYNC_LOG) << "Could not store encryption key for account" << mUsername << "in KWallet";
        return;
    }

    qCDebug(ETESYNC_LOG) << "Wrote encryption key to wallet";

    if (etebase_fs_cache_save_account(mEtebaseCache.get(), mAccount.get(), encryptionKey.constData(), encryptionKey.size())) {
        qCDebug(ETESYNC_LOG) << "Could not save account to cache";
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_code() << etebase_error_get_message();
    }
}

void EteSyncClientState::loadAccount()
{
    if (!mWallet) {
        qCDebug(ETESYNC_LOG) << "Get account - wallet not opened";
        if (!openWalletFolder()) {
            return;
        }
    }

    if (!mWallet->entryList().contains(mUsername)) {
        qCDebug(ETESYNC_LOG) << "Encryption key for account" << mUsername << "not found in KWallet";
        return;
    }

    QByteArray encryptionKey;
    if (mWallet->readEntry(mUsername, encryptionKey)) {
        qCDebug(ETESYNC_LOG) << "Could not read encryption key for account" << mUsername << "from KWallet";
        return;
    }

    qCDebug(ETESYNC_LOG) << "Read encryption key from wallet";

    mAccount = EtebaseAccountPtr(etebase_fs_cache_load_account(mEtebaseCache.get(), mClient.get(), encryptionKey.constData(), encryptionKey.size()));

    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Could not get etebase account from caache";
    }
}

void EteSyncClientState::deleteWalletEntry()
{
    qCDebug(ETESYNC_LOG) << "Deleting wallet entry";

    if (!mWallet) {
        qCDebug(ETESYNC_LOG) << "Delete wallet entry - wallet not opened";
        if (!openWalletFolder()) {
            return;
        }
    }

    if (mWallet->removeEntry(mUsername)) {
        qCDebug(ETESYNC_LOG) << "Unable to delete wallet entry";
    }

    qCDebug(ETESYNC_LOG) << "Deleted wallet entry";
}

void EteSyncClientState::saveEtebaseCollectionCache(const EtebaseCollection *etesyncCollection) const
{
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Unable to save collection cache - account is null";
        return;
    }

    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Unable to save collection cache - etesyncCollection is null";
        return;
    }

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    if (etebase_fs_cache_collection_set(mEtebaseCache.get(), collectionManager.get(), etesyncCollection)) {
        qCDebug(ETESYNC_LOG) << "Could not save etebase collection cache for collection" << etebase_collection_get_uid(etesyncCollection);
        return;
    }

    qCDebug(ETESYNC_LOG) << "Saved cache for collection" << etebase_collection_get_uid(etesyncCollection);
}

void EteSyncClientState::saveEtebaseItemCache(const EtebaseItem *etesyncItem, const EtebaseCollection *parentCollection) const
{
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Unable to save collection cache - account is null";
        return;
    }

    if (!etesyncItem) {
        qCDebug(ETESYNC_LOG) << "Unable to save item cache - etesyncItem is null";
        return;
    }

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), parentCollection));

    QString collectionUid = QString::fromUtf8(etebase_collection_get_uid(parentCollection));

    if (etebase_fs_cache_item_set(mEtebaseCache.get(), itemManager.get(), collectionUid, etesyncItem)) {
        qCDebug(ETESYNC_LOG) << "Could not save etebase item cache for item" << etebase_item_get_uid(etesyncItem);
        return;
    }

    qCDebug(ETESYNC_LOG) << "Saved cache for item" << etebase_item_get_uid(etesyncItem);
}

EtebaseCollectionPtr EteSyncClientState::getEtebaseCollectionFromCache(const QString &collectionUid) const
{
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Unable to get collection cache - account is null";
        return nullptr;
    }

    if (collectionUid.isEmpty()) {
        qCDebug(ETESYNC_LOG) << "Unable to get collection cache - uid is empty";
        return nullptr;
    }

    qCDebug(ETESYNC_LOG) << "Getting cache for collection" << collectionUid;

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));

    return etebase_fs_cache_collection_get(mEtebaseCache.get(), collectionManager.get(), collectionUid);
}

EtebaseItemPtr EteSyncClientState::getEtebaseItemFromCache(const QString &itemUid, const EtebaseCollection *parentCollection) const
{
    if (!mAccount) {
        qCDebug(ETESYNC_LOG) << "Unable to get item cache - account is null";
        return nullptr;
    }

    if (!parentCollection) {
        qCDebug(ETESYNC_LOG) << "Unable to get item cache - parentCollection is null";
        return nullptr;
    }

    if (itemUid.isEmpty()) {
        qCDebug(ETESYNC_LOG) << "Unable to get item cache - uid is empty";
        return nullptr;
    }

    qCDebug(ETESYNC_LOG) << "Getting cache for item" << itemUid;

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), parentCollection));

    QString collectionUid = QString::fromUtf8(etebase_collection_get_uid(parentCollection));

    return etebase_fs_cache_item_get(mEtebaseCache.get(), itemManager.get(), collectionUid, itemUid);
}

void EteSyncClientState::deleteEtebaseCollectionCache(const QString &collectionUid)
{
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    if (etebase_fs_cache_collection_unset(mEtebaseCache.get(), collectionManager.get(), collectionUid)) {
        qCDebug(ETESYNC_LOG) << "Could not delete cache for collection" << collectionUid;
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_code() << etebase_error_get_message();
        return;
    }
    qCDebug(ETESYNC_LOG) << "Deleted cache for collection" << collectionUid;
}

void EteSyncClientState::deleteEtebaseItemCache(const QString &itemUid, const EtebaseCollection *parentCollection)
{
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mAccount.get()));
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), parentCollection));

    const QString collectionUid = QString::fromUtf8(etebase_collection_get_uid(parentCollection));
    if (etebase_fs_cache_item_unset(mEtebaseCache.get(), itemManager.get(), collectionUid, itemUid)) {
        qCDebug(ETESYNC_LOG) << "Could not delete cache for item" << itemUid;
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_code() << etebase_error_get_message();
        return;
    }
    qCDebug(ETESYNC_LOG) << "Deleted cache for item" << itemUid;
}

void EteSyncClientState::deleteEtebaseUserCache()
{
    if (etebase_fs_cache_clear_user(mEtebaseCache.get())) {
        qCDebug(ETESYNC_LOG) << "Could not remove cache for user";
    }
}
