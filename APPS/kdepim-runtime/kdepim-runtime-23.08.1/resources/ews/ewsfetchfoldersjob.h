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
class EwsFetchFoldersJobPrivate;

class EwsFetchFoldersJob : public EwsJob
{
    Q_OBJECT
public:
    EwsFetchFoldersJob(EwsClient &client, const Akonadi::Collection &rootCollection, QObject *parent);
    ~EwsFetchFoldersJob() override;

    Akonadi::Collection::List folders() const
    {
        return mFolders;
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
    Akonadi::Collection::List mFolders;

    QString mSyncState;

    std::unique_ptr<EwsFetchFoldersJobPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(EwsFetchFoldersJob)
};
