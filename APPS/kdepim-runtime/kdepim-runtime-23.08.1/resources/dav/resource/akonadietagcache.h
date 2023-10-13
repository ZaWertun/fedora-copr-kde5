/*
    SPDX-FileCopyrightText: 2010 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDAV/EtagCache>

namespace Akonadi
{
class Collection;
}

class KJob;

/**
 * @short A helper class to cache etags.
 *
 * The KDAV::EtagCache caches the remote ids and etags of all items
 * in a given collection. This cache is needed to find
 * out which items have been changed in the backend and have to
 * be refetched on the next call of ResourceBase::retrieveItems()
 */
class AkonadiEtagCache : public KDAV::EtagCache
{
    Q_OBJECT

public:
    /**
     * Creates a new etag cache and populates it with the ETags
     * of items found in @p collection.
     */
    explicit AkonadiEtagCache(const Akonadi::Collection &collection, QObject *parent = nullptr);

private Q_SLOTS:
    void onItemFetchJobFinished(KJob *job);
};
