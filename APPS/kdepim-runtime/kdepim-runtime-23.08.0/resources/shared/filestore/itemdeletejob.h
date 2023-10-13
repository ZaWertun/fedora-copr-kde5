/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <memory>

namespace Akonadi
{
class Item;

namespace FileStore
{
class AbstractJobSession;
class ItemDeleteJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT ItemDeleteJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit ItemDeleteJob(const Item &item, AbstractJobSession *session = nullptr);

    ~ItemDeleteJob() override;

    Q_REQUIRED_RESULT Item item() const;

    bool accept(Visitor *visitor) override;

private:
    void handleItemDeleted(const Akonadi::Item &item);

private:
    std::unique_ptr<ItemDeleteJobPrivate> const d;
};
}
}
