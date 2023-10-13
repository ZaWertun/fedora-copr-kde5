/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <memory>

namespace Akonadi
{
namespace FileStore
{
class StoreCompactJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT StoreCompactJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit StoreCompactJob(AbstractJobSession *session = nullptr);

    ~StoreCompactJob() override;

    bool accept(Visitor *visitor) override;

    Q_REQUIRED_RESULT Item::List changedItems() const;

    Q_REQUIRED_RESULT Collection::List changedCollections() const;

Q_SIGNALS:
    void collectionsChanged(const Akonadi::Collection::List &collections);
    void itemsChanged(const Akonadi::Item::List &items);

private:
    void handleCollectionsChanged(const Collection::List &collections);
    void handleItemsChanged(const Item::List &items);

private:
    std::unique_ptr<StoreCompactJobPrivate> const d;
};
}
}
