/*
    SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Job>
#include <QQueue>
#include <QStringList>

/**
 * @short Job that updates the gid of all items in the store.
 *
 * Requires a serializer plugin supporting the gidextractor interface for the mimetype of the objects to migrate.
 *
 * @author Christian Mollekopf <mollekopf@kolabsys.com>
 * @since 4.12
 */
class GidMigrationJob : public Akonadi::Job
{
    Q_OBJECT
public:
    /**
     * @param mimeTypeFilter The list of mimetypes of objects to be migrated.
     * @param parent The parent object.
     */
    explicit GidMigrationJob(const QStringList &mimeTypeFilter, QObject *parent = nullptr);

    /**
     * Destroys the item fetch job.
     */
    ~GidMigrationJob() override;

    void doStart() override;

private:
    void collectionsReceived(const Akonadi::Collection::List &);
    void collectionsFetched(KJob *);
    void itemsUpdated(KJob *);
    void processCollection();
    const QStringList mMimeTypeFilter;
    Akonadi::Collection::List mCollections;
};

/**
 * @internal
 */
class UpdateJob : public Akonadi::Job
{
    Q_OBJECT
public:
    explicit UpdateJob(const Akonadi::Collection &col, QObject *parent = nullptr);
    ~UpdateJob() override;

    void doStart() override;
    void slotResult(KJob *job) override;

private:
    void itemsReceived(const Akonadi::Item::List &items);
    bool processNext();

    const Akonadi::Collection mCollection;
    QQueue<Akonadi::Item> mItemQueue;
    bool mModJobRunning;
};
