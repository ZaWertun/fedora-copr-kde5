/*
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "googleresourcestateinterface.h"

class GoogleResource;

class GoogleResourceState : public GoogleResourceStateInterface
{
public:
    explicit GoogleResourceState(GoogleResource *resource);
    ~GoogleResourceState() override = default;

    // Items handling
    void itemRetrieved(const Akonadi::Item &item) override;
    void itemsRetrieved(const Akonadi::Item::List &items) override;
    void itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed) override;
    void itemsRetrievalDone() override;
    void setTotalItems(int) override;
    void itemChangeCommitted(const Akonadi::Item &item) override;
    void itemsChangesCommitted(const Akonadi::Item::List &items) override;
    Akonadi::Item::List currentItems() override;

    // Collections handling
    void collectionsRetrieved(const Akonadi::Collection::List &collections) override;
    void collectionAttributesRetrieved(const Akonadi::Collection &collection) override;
    void collectionChangeCommitted(const Akonadi::Collection &collection) override;
    void collectionsRetrievedFromHandler(const Akonadi::Collection::List &collections) override;
    Akonadi::Collection currentCollection() override;

    // Tags handling
    void tagsRetrieved(const Akonadi::Tag::List &tags, const QHash<QString, Akonadi::Item::List> &) override;
    void tagChangeCommitted(const Akonadi::Tag &tag) override;

    // Relations handling
    void relationsRetrieved(const Akonadi::Relation::List &tags) override;

    // FreeBusy handling
    void freeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText) override;
    void handlesFreeBusy(const QString &email, bool handles) override;

    // Result reporting
    void changeProcessed() override;
    void cancelTask(const QString &errorString) override;
    void deferTask() override;
    void taskDone() override;

    void emitStatus(int status, const QString &message) override;
    void emitError(const QString &message) override;
    void emitWarning(const QString &message) override;
    void emitPercent(int percent) override;

    // Other
    void scheduleCustomTask(QObject *receiver, const char *method, const QVariant &argument) override;

    // Google-specific stuff
    bool canPerformTask() override;
    bool handleError(KGAPI2::Job *job, bool _cancelTask) override;

private:
    GoogleResource *const m_resource;
};
