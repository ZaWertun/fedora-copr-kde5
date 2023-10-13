/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>

#include "ewsfolder.h"
#include "ewsjob.h"

#include <memory>

class EwsClient;
class EwsFetchFoldersIncrJobPrivate;

class EwsFetchFoldersIncrJob : public EwsJob
{
    Q_OBJECT
public:
    EwsFetchFoldersIncrJob(EwsClient &client, const QString &syncState, const Akonadi::Collection &rootCollection, QObject *parent);
    ~EwsFetchFoldersIncrJob() override;

    Akonadi::Collection::List changedFolders() const
    {
        return mChangedFolders;
    }

    Akonadi::Collection::List deletedFolders() const
    {
        return mDeletedFolders;
    }

    const QString &syncState() const
    {
        return mSyncState;
    }

    void start() override;
Q_SIGNALS:
    void reportStatus(int status, const QString &message = QString());
    void reportPercent(int progress);

private:
    Akonadi::Collection::List mChangedFolders;
    Akonadi::Collection::List mDeletedFolders;

    QString mSyncState;

    std::unique_ptr<EwsFetchFoldersIncrJobPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(EwsFetchFoldersIncrJob)
};
