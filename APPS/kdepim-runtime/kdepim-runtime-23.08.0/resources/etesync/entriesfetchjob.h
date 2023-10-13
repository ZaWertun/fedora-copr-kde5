/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KJob>

#include "etebaseadapter.h"
#include "etesyncclientstate.h"

namespace EteSyncAPI
{
class EntriesFetchJob : public KJob
{
    Q_OBJECT

public:
    explicit EntriesFetchJob(const EteSyncClientState *mClientState,
                             const Akonadi::Collection &collection,
                             EtebaseCollectionPtr etesyncCollection,
                             QObject *parent = nullptr);

    void start() override;

    Q_REQUIRED_RESULT Akonadi::Item::List items() const
    {
        return mItems;
    }

    Q_REQUIRED_RESULT Akonadi::Item::List removedItems() const
    {
        return mRemovedItems;
    }

    Q_REQUIRED_RESULT Akonadi::Collection collection() const
    {
        return mCollection;
    }

protected:
    void fetchEntries();
    void setupItem(const EtebaseItem *etesyncItem, const QString &type);

private:
    const EteSyncClientState *const mClientState;
    Akonadi::Collection mCollection;
    const EtebaseCollectionPtr mEtesyncCollection;
    Akonadi::Item::List mItems;
    Akonadi::Item::List mRemovedItems;
};
} // namespace EteSyncAPI
