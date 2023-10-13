/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "session_p.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionmovejob.h"
#include "itemcreatejob.h"
#include "itemfetchjob.h"
#include "itemmodifyjob.h"
#include "itemmovejob.h"
#include "storecompactjob.h"

using namespace Akonadi;

FileStore::AbstractJobSession::AbstractJobSession(QObject *parent)
    : QObject(parent)
{
}

FileStore::AbstractJobSession::~AbstractJobSession() = default;

void FileStore::AbstractJobSession::notifyCollectionsReceived(FileStore::Job *job, const Collection::List &collections)
{
    auto fetchJob = qobject_cast<FileStore::CollectionFetchJob *>(job);
    if (fetchJob != nullptr) {
        fetchJob->handleCollectionsReceived(collections);
    }
}

void FileStore::AbstractJobSession::notifyCollectionCreated(FileStore::Job *job, const Collection &collection)
{
    auto createJob = qobject_cast<FileStore::CollectionCreateJob *>(job);
    if (createJob != nullptr) {
        createJob->handleCollectionCreated(collection);
    }
}

void FileStore::AbstractJobSession::notifyCollectionDeleted(FileStore::Job *job, const Collection &collection)
{
    auto deleteJob = qobject_cast<FileStore::CollectionDeleteJob *>(job);
    if (deleteJob != nullptr) {
        deleteJob->handleCollectionDeleted(collection);
    }
}

void FileStore::AbstractJobSession::notifyCollectionModified(FileStore::Job *job, const Collection &collection)
{
    auto modifyJob = qobject_cast<FileStore::CollectionModifyJob *>(job);
    if (modifyJob != nullptr) {
        modifyJob->handleCollectionModified(collection);
    }
}

void FileStore::AbstractJobSession::notifyCollectionMoved(FileStore::Job *job, const Collection &collection)
{
    auto moveJob = qobject_cast<FileStore::CollectionMoveJob *>(job);
    if (moveJob != nullptr) {
        moveJob->handleCollectionMoved(collection);
    }
}

void FileStore::AbstractJobSession::notifyItemsReceived(FileStore::Job *job, const Item::List &items)
{
    auto fetchJob = qobject_cast<FileStore::ItemFetchJob *>(job);
    if (fetchJob != nullptr) {
        fetchJob->handleItemsReceived(items);
    }
}

void FileStore::AbstractJobSession::notifyItemCreated(FileStore::Job *job, const Item &item)
{
    auto createJob = qobject_cast<FileStore::ItemCreateJob *>(job);
    if (createJob != nullptr) {
        createJob->handleItemCreated(item);
    }
}

void FileStore::AbstractJobSession::notifyItemModified(FileStore::Job *job, const Item &item)
{
    auto modifyJob = qobject_cast<FileStore::ItemModifyJob *>(job);
    if (modifyJob != nullptr) {
        modifyJob->handleItemModified(item);
    }
}

void FileStore::AbstractJobSession::notifyItemMoved(FileStore::Job *job, const Item &item)
{
    auto moveJob = qobject_cast<FileStore::ItemMoveJob *>(job);
    if (moveJob != nullptr) {
        moveJob->handleItemMoved(item);
    }
}

void FileStore::AbstractJobSession::notifyCollectionsChanged(FileStore::Job *job, const Collection::List &collections)
{
    auto compactJob = qobject_cast<FileStore::StoreCompactJob *>(job);
    if (compactJob != nullptr) {
        compactJob->handleCollectionsChanged(collections);
    }
}

void FileStore::AbstractJobSession::notifyItemsChanged(FileStore::Job *job, const Item::List &items)
{
    auto compactJob = qobject_cast<FileStore::StoreCompactJob *>(job);
    if (compactJob != nullptr) {
        compactJob->handleItemsChanged(items);
    }
}

void FileStore::AbstractJobSession::setError(FileStore::Job *job, int errorCode, const QString &errorText)
{
    job->setError(errorCode);
    job->setErrorText(errorText);
}

void FileStore::AbstractJobSession::emitResult(FileStore::Job *job)
{
    removeJob(job);

    job->emitResult();
}
