/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KJob>

#include "etebaseadapter.h"
#include "etesyncclientstate.h"

namespace EteSyncAPI
{
class LoginJob : public KJob
{
    Q_OBJECT

public:
    explicit LoginJob(EteSyncClientState *clientState, const QString &serverUrl, const QString &username, const QString &password, QObject *parent = nullptr);

    void start() override;

    bool getLoginResult() const
    {
        return mLoginResult;
    }

    bool getAccountStatusResult() const
    {
        return mAccountStatusResult;
    }

private:
    void login();
    void createDefaultCollection(const QString &collectionType, const QString &collectionName);

    EteSyncClientState *mClientState = nullptr;
    QString mServerUrl;
    QString mUsername;
    QString mPassword;
    bool mLoginResult;
    bool mAccountStatusResult = false;
    bool mUserInfoResult;
};
} // namespace EteSyncAPI
