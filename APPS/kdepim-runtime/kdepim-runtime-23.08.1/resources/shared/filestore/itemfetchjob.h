/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <Akonadi/Item>

#include <memory>

namespace Akonadi
{
class Collection;
class ItemFetchScope;

namespace FileStore
{
class AbstractJobSession;
class ItemFetchJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT ItemFetchJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit ItemFetchJob(const Collection &collection, AbstractJobSession *session = nullptr);

    explicit ItemFetchJob(const Item &item, AbstractJobSession *session = nullptr);

    explicit ItemFetchJob(const Item::List &items, AbstractJobSession *session = nullptr);

    ~ItemFetchJob() override;

    Q_REQUIRED_RESULT Collection collection() const;

    Q_REQUIRED_RESULT Item item() const;

    Item::List requestedItems() const;

    void setFetchScope(const ItemFetchScope &fetchScope);

    ItemFetchScope &fetchScope();

    Q_REQUIRED_RESULT Item::List items() const;

    bool accept(Visitor *visitor) override;

Q_SIGNALS:
    void itemsReceived(const Akonadi::Item::List &items);

private:
    void handleItemsReceived(const Akonadi::Item::List &items);

private:
    std::unique_ptr<ItemFetchJobPrivate> const d;
};
}
}
