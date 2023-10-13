/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "entriesfetchjob.h"

#include <Akonadi/CollectionModifyJob>
#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>
#include <KContacts/Addressee>
#include <QtConcurrent>

#include "etesync_debug.h"
#include "settings.h"

#define ITEMS_FETCH_BATCH_SIZE 50
#define ITEM_SIZE_INITIAL_TRY 2000

using namespace Akonadi;
using namespace EteSyncAPI;

EntriesFetchJob::EntriesFetchJob(const EteSyncClientState *clientState,
                                 const Akonadi::Collection &collection,
                                 EtebaseCollectionPtr etesyncCollection,
                                 QObject *parent)
    : KJob(parent)
    , mClientState(clientState)
    , mCollection(collection)
    , mEtesyncCollection(std::move(etesyncCollection))
{
}

void EntriesFetchJob::start()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this, [this] {
        qCDebug(ETESYNC_LOG) << "emitResult from EntriesFetchJob";
        emitResult();
    });
    QFuture<void> fetchEntriesFuture = QtConcurrent::run(this, &EntriesFetchJob::fetchEntries);
    watcher->setFuture(fetchEntriesFuture);
}

void EntriesFetchJob::fetchEntries()
{
    if (!mClientState) {
        setError(UserDefinedError);
        setErrorText(QStringLiteral("EntriesFetchJob: EteSync client state is null"));
        return;
    }

    EtebaseAccount *account = mClientState->account();

    if (!account) {
        setError(UserDefinedError);
        setErrorText(QStringLiteral("EntriesFetchJob: Etebase account is null"));
        return;
    }

    if (!mEtesyncCollection) {
        setError(UserDefinedError);
        setErrorText(QStringLiteral("EntriesFetchJob: Etebase collection is null"));
        return;
    }

    EtebaseItemMetadataPtr metaData(etebase_collection_get_meta(mEtesyncCollection.get()));
    const QString type = QString::fromUtf8(etebase_collection_get_collection_type(mEtesyncCollection.get()));
    qCDebug(ETESYNC_LOG) << "Type:" << type;

    QString sToken = mCollection.remoteRevision();
    bool done = false;
    EtebaseCollectionManagerPtr collectionManager(etebase_account_get_collection_manager(account));
    EtebaseItemManagerPtr itemManager(etebase_collection_manager_get_item_manager(collectionManager.get(), mEtesyncCollection.get()));

    while (!done) {
        EtebaseFetchOptionsPtr fetchOptions(etebase_fetch_options_new());
        etebase_fetch_options_set_stoken(fetchOptions.get(), sToken);
        etebase_fetch_options_set_limit(fetchOptions.get(), ITEMS_FETCH_BATCH_SIZE);

        EtebaseItemListResponsePtr itemList(etebase_item_manager_list(itemManager.get(), fetchOptions.get()));
        if (!itemList) {
            setError(int(etebase_error_get_code()));
            const char *err = etebase_error_get_message();
            setErrorText(QString::fromUtf8(err));
            return;
        }

        sToken = QString::fromUtf8(etebase_item_list_response_get_stoken(itemList.get()));

        done = etebase_item_list_response_is_done(itemList.get());

        uintptr_t dataLength = etebase_item_list_response_get_data_length(itemList.get());

        qCDebug(ETESYNC_LOG) << "Retrieved item list length" << dataLength;

        std::vector<const EtebaseItem *> etesyncItems(dataLength, nullptr);
        if (etebase_item_list_response_get_data(itemList.get(), etesyncItems.data())) {
            setError(int(etebase_error_get_code()));
            const char *err = etebase_error_get_message();
            setErrorText(QString::fromUtf8(err));
        }

        for (uintptr_t i = 0; i < dataLength; i++) {
            mClientState->saveEtebaseItemCache(etesyncItems[i], mEtesyncCollection.get());
            setupItem(etesyncItems[i], type);
        }
    }

    mCollection.setRemoteRevision(sToken);
}

void EntriesFetchJob::setupItem(const EtebaseItem *etesyncItem, const QString &type)
{
    if (!etesyncItem) {
        qCDebug(ETESYNC_LOG) << "Unable to setup item - etesyncItem is null";
        return;
    }

    qCDebug(ETESYNC_LOG) << "Setting up item" << etebase_item_get_uid(etesyncItem);

    Item item;

    if (type == ETEBASE_COLLECTION_TYPE_ADDRESS_BOOK) {
        item.setMimeType(KContacts::Addressee::mimeType());
    } else if (type == ETEBASE_COLLECTION_TYPE_CALENDAR) {
        item.setMimeType(KCalendarCore::Event::eventMimeType());
    } else if (type == ETEBASE_COLLECTION_TYPE_TASKS) {
        item.setMimeType(KCalendarCore::Todo::todoMimeType());
    } else {
        qCWarning(ETESYNC_LOG) << "Unknown item type. Cannot set item mime type.";
        return;
    }

    const QString itemUid = QString::fromUtf8(etebase_item_get_uid(etesyncItem));
    item.setParentCollection(mCollection);
    item.setRemoteId(itemUid);

    QByteArray content(ITEM_SIZE_INITIAL_TRY, '\0');
    auto const len = etebase_item_get_content(etesyncItem, content.data(), ITEM_SIZE_INITIAL_TRY);
    if (len > ITEM_SIZE_INITIAL_TRY) {
        content.resize(len);
        etebase_item_get_content(etesyncItem, content.data(), len);
    }
    item.setPayloadFromData(content);

    if (etebase_item_is_deleted(etesyncItem)) {
        mRemovedItems.push_back(item);
        return;
    }

    mItems.push_back(item);
}
