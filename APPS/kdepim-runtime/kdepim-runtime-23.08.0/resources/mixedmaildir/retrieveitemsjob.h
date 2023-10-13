/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2011 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <Akonadi/Job>

#include <memory>

namespace Akonadi
{
class Collection;
}

class MixedMaildirStore;
class RetrieveItemsJobPrivate;

/**
 * Used to implement ResourceBase::retrieveItems() for MixedMail Resource.
 * This completely bypasses ItemSync in order to achieve maximum performance.
 */
class RetrieveItemsJob : public Akonadi::Job
{
    Q_OBJECT
public:
    RetrieveItemsJob(const Akonadi::Collection &collection, MixedMaildirStore *store, QObject *parent = nullptr);

    ~RetrieveItemsJob() override;

    Akonadi::Collection collection() const;

    Akonadi::Item::List availableItems() const;

    Akonadi::Item::List itemsMarkedAsDeleted() const;

protected:
    void doStart() override;

private:
    friend class RetrieveItemsJobPrivate;
    std::unique_ptr<RetrieveItemsJobPrivate> const d;

    Q_PRIVATE_SLOT(d, void processNewItem())
    Q_PRIVATE_SLOT(d, void processChangedItem())
};
