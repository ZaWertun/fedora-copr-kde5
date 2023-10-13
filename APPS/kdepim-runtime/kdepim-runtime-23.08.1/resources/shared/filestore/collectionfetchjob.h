/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <Akonadi/Collection>

#include <memory>

namespace Akonadi
{
class CollectionFetchScope;

namespace FileStore
{
class AbstractJobSession;
class CollectionFetchJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT CollectionFetchJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    enum Type {
        Base,
        FirstLevel,
        Recursive,
    };

    explicit CollectionFetchJob(const Collection &collection, Type type = FirstLevel, AbstractJobSession *session = nullptr);

    ~CollectionFetchJob() override;

    Q_REQUIRED_RESULT Type type() const;

    Q_REQUIRED_RESULT Collection collection() const;

    void setFetchScope(const CollectionFetchScope &fetchScope);

    CollectionFetchScope &fetchScope();

    Q_REQUIRED_RESULT Collection::List collections() const;

    bool accept(Visitor *visitor) override;

Q_SIGNALS:
    void collectionsReceived(const Akonadi::Collection::List &items);

private:
    void handleCollectionsReceived(const Akonadi::Collection::List &collections);

private:
    std::unique_ptr<CollectionFetchJobPrivate> const d;
};
}
}
