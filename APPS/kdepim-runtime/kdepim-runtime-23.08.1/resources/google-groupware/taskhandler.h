/*
    SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "generichandler.h"

class TaskHandler : public GenericHandler
{
public:
    using GenericHandler::GenericHandler;

    QString mimeType() override;
    bool canPerformTask(const Akonadi::Item &item) override;
    bool canPerformTask(const Akonadi::Item::List &items) override;

    void retrieveCollections(const Akonadi::Collection &rootCollection) override;
    void retrieveItems(const Akonadi::Collection &collection) override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) override;
    void itemsRemoved(const Akonadi::Item::List &items) override;
    void itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    void collectionRemoved(const Akonadi::Collection &collection) override;

private:
    void slotItemsRetrieved(KGAPI2::Job *job);
    void setupCollection(Akonadi::Collection &collection, const KGAPI2::TaskListPtr &taskList);
    void doRemoveTasks(const Akonadi::Item::List &items);
};
