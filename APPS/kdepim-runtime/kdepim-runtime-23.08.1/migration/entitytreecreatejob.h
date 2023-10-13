/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Job>
#include <Akonadi/TransactionSequence>

class EntityTreeCreateJob : public Akonadi::TransactionSequence
{
    Q_OBJECT
public:
    explicit EntityTreeCreateJob(const QList<Akonadi::Collection::List> &collections, const Akonadi::Item::List &items, QObject *parent = nullptr);

    void doStart() override;

private:
    void collectionCreateJobDone(KJob *);
    void createNextLevelOfCollections();
    void createReadyItems();

private:
    QList<Akonadi::Collection::List> m_collections;
    Akonadi::Item::List m_items;
    int m_pendingJobs = 0;
};
