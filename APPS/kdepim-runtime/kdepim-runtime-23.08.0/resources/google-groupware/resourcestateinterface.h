/*
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/ItemSync>

/**
 * This is a generic interface for ResourceBase class
 */
class ResourceStateInterface
{
public:
    using Ptr = QSharedPointer<ResourceStateInterface>;

    virtual ~ResourceStateInterface() = default;

    // Items handling
    virtual void itemRetrieved(const Akonadi::Item &item) = 0;
    virtual void itemsRetrieved(const Akonadi::Item::List &items) = 0;
    virtual void itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed) = 0;
    virtual void itemsRetrievalDone() = 0;
    virtual void setTotalItems(int) = 0;
    virtual void itemChangeCommitted(const Akonadi::Item &item) = 0;
    virtual void itemsChangesCommitted(const Akonadi::Item::List &items) = 0;
    virtual Akonadi::Item::List currentItems() = 0;

    // Collections handling
    virtual void collectionsRetrieved(const Akonadi::Collection::List &collections) = 0;
    virtual void collectionAttributesRetrieved(const Akonadi::Collection &collection) = 0;
    virtual void collectionChangeCommitted(const Akonadi::Collection &collection) = 0;
    virtual Akonadi::Collection currentCollection() = 0;

    // Tags handling
    virtual void tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &) = 0;
    virtual void tagChangeCommitted(const Akonadi::Tag &tag) = 0;

    // Relations handling
    virtual void relationsRetrieved(const Akonadi::Relation::List &tags) = 0;

    // Result reporting
    virtual void changeProcessed() = 0;
    virtual void cancelTask(const QString &errorString) = 0;
    virtual void deferTask() = 0;
    virtual void taskDone() = 0;

    virtual void emitStatus(int status, const QString &message) = 0;
    virtual void emitError(const QString &message) = 0;
    virtual void emitWarning(const QString &message) = 0;
    virtual void emitPercent(int percent) = 0;
};
