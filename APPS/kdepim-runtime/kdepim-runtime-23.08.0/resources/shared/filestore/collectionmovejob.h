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
class CollectionMoveJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT CollectionMoveJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit CollectionMoveJob(const Collection &collection, const Collection &targetParent, AbstractJobSession *session = nullptr);

    ~CollectionMoveJob() override;

    Q_REQUIRED_RESULT Collection collection() const;

    Q_REQUIRED_RESULT Collection targetParent() const;

    bool accept(Visitor *visitor) override;

private:
    void handleCollectionMoved(const Collection &collection);

private:
    std::unique_ptr<CollectionMoveJobPrivate> const d;
};
}
}
