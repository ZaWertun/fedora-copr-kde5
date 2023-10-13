/*
    SPDX-FileCopyrightText: 2010 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadietagcache.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

using namespace KDAV;

AkonadiEtagCache::AkonadiEtagCache(const Akonadi::Collection &collection, QObject *parent)
    : KDAV::EtagCache(parent)
{
    auto job = new Akonadi::ItemFetchJob(collection);
    job->fetchScope().fetchFullPayload(false); // We only need the remote id and the revision
    connect(job, &Akonadi::ItemFetchJob::result, this, &AkonadiEtagCache::onItemFetchJobFinished);
    job->start();
}

void AkonadiEtagCache::onItemFetchJobFinished(KJob *job)
{
    if (job->error()) {
        return;
    }

    const Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    const Akonadi::Item::List items = fetchJob->items();

    for (const Akonadi::Item &item : items) {
        if (!contains(item.remoteId())) {
            setEtagInternal(item.remoteId(), item.remoteRevision());
        }
    }
}
