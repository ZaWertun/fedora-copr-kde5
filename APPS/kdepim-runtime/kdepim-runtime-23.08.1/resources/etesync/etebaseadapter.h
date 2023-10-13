/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QString>
#include <etebase.h>
#include <memory>
#include <vector>

#define charArrFromQString(str) ((str).isNull() ? nullptr : qUtf8Printable((str)))

#define ETEBASE_COLLECTION_TYPE_CALENDAR QStringLiteral("etebase.vevent")
#define ETEBASE_COLLECTION_TYPE_ADDRESS_BOOK QStringLiteral("etebase.vcard")
#define ETEBASE_COLLECTION_TYPE_TASKS QStringLiteral("etebase.vtodo")

#define ETESYNC_DEFAULT_COLLECTION_COLOR QStringLiteral("#8BC34A")

static const char *ETESYNC_COLLECTION_TYPES[] = {"etebase.vevent", "etebase.vcard", "etebase.vtodo"};
static const int ETESYNC_COLLECTION_TYPES_SIZE = ETEBASE_UTILS_C_ARRAY_LEN(ETESYNC_COLLECTION_TYPES);

struct EtebaseDeleter {
    void operator()(EtebaseClient *ptr)
    {
        etebase_client_destroy(ptr);
    }

    void operator()(EtebaseAccount *ptr)
    {
        etebase_account_destroy(ptr);
    }

    void operator()(EtebaseFetchOptions *ptr)
    {
        etebase_fetch_options_destroy(ptr);
    }

    void operator()(EtebaseCollectionListResponse *ptr)
    {
        etebase_collection_list_response_destroy(ptr);
    }

    void operator()(EtebaseCollectionManager *ptr)
    {
        etebase_collection_manager_destroy(ptr);
    }

    void operator()(EtebaseCollection *ptr)
    {
        etebase_collection_destroy(ptr);
    }

    void operator()(EtebaseItemManager *ptr)
    {
        etebase_item_manager_destroy(ptr);
    }

    void operator()(EtebaseItemListResponse *ptr)
    {
        etebase_item_list_response_destroy(ptr);
    }

    void operator()(EtebaseItemMetadata *ptr)
    {
        etebase_item_metadata_destroy(ptr);
    }

    void operator()(EtebaseItem *ptr)
    {
        etebase_item_destroy(ptr);
    }

    void operator()(EtebaseFileSystemCache *ptr)
    {
        etebase_fs_cache_destroy(ptr);
    }
};

struct VoidDeleter {
    void operator()(void *ptr)
    {
        std::free(ptr);
    }
};

using EtebaseClientPtr = std::unique_ptr<EtebaseClient, EtebaseDeleter>;
using EtebaseAccountPtr = std::unique_ptr<EtebaseAccount, EtebaseDeleter>;
using EtebaseFetchOptionsPtr = std::unique_ptr<EtebaseFetchOptions, EtebaseDeleter>;
using EtebaseCollectionListResponsePtr = std::unique_ptr<EtebaseCollectionListResponse, EtebaseDeleter>;
using EtebaseCollectionManagerPtr = std::unique_ptr<EtebaseCollectionManager, EtebaseDeleter>;
using EtebaseCollectionPtr = std::unique_ptr<EtebaseCollection, EtebaseDeleter>;
using EtebaseItemManagerPtr = std::unique_ptr<EtebaseItemManager, EtebaseDeleter>;
using EtebaseItemListResponsePtr = std::unique_ptr<EtebaseItemListResponse, EtebaseDeleter>;
using EtebaseItemMetadataPtr = std::unique_ptr<EtebaseItemMetadata, EtebaseDeleter>;
using EtebaseItemPtr = std::unique_ptr<EtebaseItem, EtebaseDeleter>;
using EtebaseFileSystemCachePtr = std::unique_ptr<EtebaseFileSystemCache, EtebaseDeleter>;
using EtebaseCachePtr = std::unique_ptr<void, VoidDeleter>;
using CharPtr = std::unique_ptr<char, VoidDeleter>;

QString QStringFromCharPtr(const CharPtr &str);

EtebaseClientPtr etebase_client_new(const QString &client_name, const QString &server_url);

EtebaseAccountPtr etebase_account_login(const EtebaseClient *client, const QString &username, const QString &password);

void etebase_fetch_options_set_stoken(EtebaseFetchOptions *fetch_options, const QString &stoken);

void etebase_item_metadata_set_color(EtebaseItemMetadata *meta_data, const QString &color);

void etebase_item_metadata_set_name(EtebaseItemMetadata *meta_data, const QString &name);

EtebaseCollectionPtr etebase_collection_manager_create(const EtebaseCollectionManager *col_mgr,
                                                       const QString &collection_type,
                                                       const EtebaseItemMetadata *meta,
                                                       const void *content,
                                                       uintptr_t content_size);

EtebaseFileSystemCachePtr etebase_fs_cache_new(const QString &path, const QString &username);

int32_t etebase_fs_cache_item_set(const EtebaseFileSystemCache *fs_cache, const EtebaseItemManager *item_mgr, const QString &col_uid, const EtebaseItem *item);

EtebaseCollectionPtr etebase_fs_cache_collection_get(const EtebaseFileSystemCache *fs_cache, const EtebaseCollectionManager *col_mgr, const QString &col_uid);

EtebaseItemPtr
etebase_fs_cache_item_get(const EtebaseFileSystemCache *fs_cache, const EtebaseItemManager *item_mgr, const QString &col_uid, const QString &item_uid);

int32_t etebase_fs_cache_collection_unset(const EtebaseFileSystemCache *fs_cache, const EtebaseCollectionManager *col_mgr, const QString &col_uid);

int32_t
etebase_fs_cache_item_unset(const EtebaseFileSystemCache *fs_cache, const EtebaseItemManager *item_mgr, const QString &col_uid, const QString &item_uid);
