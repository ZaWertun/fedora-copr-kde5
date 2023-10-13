/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsid.h"
#include "ewsjob.h"

class EwsClient;
class EwsSettings;

class EwsSubscribedFoldersJob : public EwsJob
{
    Q_OBJECT
public:
    EwsSubscribedFoldersJob(EwsClient &client, EwsSettings *settings, QObject *parent);
    ~EwsSubscribedFoldersJob() override;

    void start() override;

    EwsId::List folders() const
    {
        return mFolders;
    }

    static const EwsId::List &defaultSubscriptionFolders();
private Q_SLOTS:
    void verifySubFoldersRequestFinished(KJob *job);

private:
    EwsId::List mFolders;
    EwsClient &mClient;
    EwsSettings *mSettings = nullptr;
};
