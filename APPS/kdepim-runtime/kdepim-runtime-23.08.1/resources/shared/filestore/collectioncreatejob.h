/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <memory>

namespace Akonadi
{
class Collection;

namespace FileStore
{
class AbstractJobSession;
class CollectionCreateJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT CollectionCreateJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit CollectionCreateJob(const Collection &collection, const Collection &targetParent, AbstractJobSession *session = nullptr);

    ~CollectionCreateJob() override;

    Collection collection() const;

    Collection targetParent() const;

    bool accept(Visitor *visitor) override;

private:
    void handleCollectionCreated(const Collection &collection);

private:
    std::unique_ptr<CollectionCreateJobPrivate> const d;
};
}
}
