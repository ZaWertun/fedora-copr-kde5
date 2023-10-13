/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QObject>

namespace Akonadi
{
namespace FileStore
{
class Job;

/**
 */
class AbstractJobSession : public QObject
{
    Q_OBJECT

public:
    explicit AbstractJobSession(QObject *parent = nullptr);

    ~AbstractJobSession() override;

    virtual void addJob(Job *job) = 0;

    virtual void cancelAllJobs() = 0;

    void notifyCollectionsReceived(Job *job, const Collection::List &collections);

    void notifyCollectionCreated(Job *job, const Collection &collection);

    void notifyCollectionDeleted(Job *job, const Collection &collection);

    void notifyCollectionModified(Job *job, const Collection &collection);

    void notifyCollectionMoved(Job *job, const Collection &collection);

    void notifyItemsReceived(Job *job, const Item::List &items);

    void notifyItemCreated(Job *job, const Item &item);

    void notifyItemModified(Job *job, const Item &item);

    void notifyItemMoved(Job *job, const Item &item);

    void notifyCollectionsChanged(Job *job, const Collection::List &collections);

    void notifyItemsChanged(Job *job, const Item::List &items);

    void setError(Job *job, int errorCode, const QString &errorText);

    void emitResult(Job *job);

Q_SIGNALS:
    void jobsReady(const QList<FileStore::Job *> &jobs);

protected:
    virtual void removeJob(Job *job) = 0;
};
}
}
