/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kolabrelationresourcetask.h"

class KolabChangeItemsRelationsTask : public KolabRelationResourceTask
{
    Q_OBJECT
public:
    explicit KolabChangeItemsRelationsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);

protected:
    void startRelationTask(KIMAP::Session *session) override;

private Q_SLOTS:
    void onRelationFetchDone(KJob *job);

    void addRelation(const Akonadi::Relation &relation);
    void onItemsFetched(KJob *job);
    void removeRelation(const Akonadi::Relation &relation);
    void onSelectDone(KJob *job);
    void triggerStoreJob();
    void onChangeCommitted(KJob *job);

private:
    void processNextRelation();

    KIMAP::Session *mSession = nullptr;
    Akonadi::Relation::List mAddedRelations;
    Akonadi::Relation::List mRemovedRelations;
    Akonadi::Relation mCurrentRelation;
    bool mAdding = false;
};
