/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "loginjob.h"

#include <QtConcurrent>

#include "etesync_debug.h"

#define ETESYNC_DEFAULT_CALENDAR_NAME QStringLiteral("My Calendar")
#define ETESYNC_DEFAULT_ADDRESS_BOOK_NAME QStringLiteral("My Contacts")
#define ETESYNC_DEFAULT_TASKS_NAME QStringLiteral("My Tasks")

using namespace EteSyncAPI;

LoginJob::LoginJob(EteSyncClientState *clientState, const QString &serverUrl, const QString &username, const QString &password, QObject *parent)
    : KJob(parent)
    , mClientState(clientState)
    , mServerUrl(serverUrl)
    , mUsername(username)
    , mPassword(password)
{
}

void LoginJob::start()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this] {
        qCDebug(ETESYNC_LOG) << "emitResult from LoginJob";
        emitResult();
    });
    QFuture<void> loginFuture = QtConcurrent::run(this, &LoginJob::login);
    watcher->setFuture(loginFuture);
}

void LoginJob::login()
{
    qCDebug(ETESYNC_LOG) << "Logging in" << mServerUrl << mUsername << mPassword;
    mLoginResult = mClientState->login(mServerUrl, mUsername, mPassword);
    qCDebug(ETESYNC_LOG) << "Login result" << mLoginResult;
    if (!mLoginResult) {
        qCDebug(ETESYNC_LOG) << "Returning error from LoginJob";
        setError(etebase_error_get_code());
        const char *err = etebase_error_get_message();
        setErrorText(QString::fromUtf8(err));
        return;
    }
    EteSyncClientState::AccountStatus accountStatusResult = mClientState->accountStatus();
    qCDebug(ETESYNC_LOG) << "Account status result" << accountStatusResult;
    if (accountStatusResult == EteSyncClientState::ERROR) {
        qCDebug(ETESYNC_LOG) << "Returning error from LoginJob";
        setError(etebase_error_get_code());
        const char *err = etebase_error_get_message();
        setErrorText(QString::fromUtf8(err));
        return;
    }

    // Create default collections if this is a new user and has no EteSync collections
    if (accountStatusResult == EteSyncClientState::NEW_ACCOUNT) {
        qCDebug(ETESYNC_LOG) << "New account: creating default collections";
        createDefaultCollection(ETEBASE_COLLECTION_TYPE_CALENDAR, ETESYNC_DEFAULT_CALENDAR_NAME);
        createDefaultCollection(ETEBASE_COLLECTION_TYPE_ADDRESS_BOOK, ETESYNC_DEFAULT_ADDRESS_BOOK_NAME);
        createDefaultCollection(ETEBASE_COLLECTION_TYPE_TASKS, ETESYNC_DEFAULT_TASKS_NAME);
    }
    mAccountStatusResult = true;
}

void LoginJob::createDefaultCollection(const QString &collectionType, const QString &collectionName)
{
    qCDebug(ETESYNC_LOG) << "Creating default collection" << collectionName;

    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(mClientState->account()));

    // Create metadata
    int64_t modificationTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
    EtebaseItemMetadataPtr collectionMetaData(etebase_item_metadata_new());
    etebase_item_metadata_set_name(collectionMetaData.get(), collectionName);
    etebase_item_metadata_set_mtime(collectionMetaData.get(), &modificationTimeSinceEpoch);

    qCDebug(ETESYNC_LOG) << "Created metadata";

    // Create EteSync collection
    EtebaseCollectionPtr etesyncCollection(etebase_collection_manager_create(collectionManager.get(), collectionType, collectionMetaData.get(), nullptr, 0));
    if (!etesyncCollection) {
        qCDebug(ETESYNC_LOG) << "Could not create new etesyncCollection";
        qCDebug(ETESYNC_LOG) << "Etebase error;" << etebase_error_get_message();
        return;
    }

    qCDebug(ETESYNC_LOG) << "Created EteSync collection";

    // Upload to server
    if (etebase_collection_manager_upload(collectionManager.get(), etesyncCollection.get(), NULL)) {
        qCDebug(ETESYNC_LOG) << "Error uploading collection addition";
        qCDebug(ETESYNC_LOG) << "Etebase error:" << etebase_error_get_message();
        return;
    } else {
        qCDebug(ETESYNC_LOG) << "Uploaded collection addition to server";
    }
}
