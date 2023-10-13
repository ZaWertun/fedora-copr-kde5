/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QObject;
#include "ewsabstractauth.h"

#include <memory>

class EwsOAuthPrivate;

class EwsOAuth : public EwsAbstractAuth
{
    Q_OBJECT
public:
    EwsOAuth(QObject *parent, const QString &email, const QString &appId, const QString &redirectUri);
    ~EwsOAuth() override;

    void init() override;
    bool getAuthData(QString &username, QString &password, QStringList &customHeaders) override;
    void notifyRequestAuthFailed() override;
    bool authenticate(bool interactive) override;
    const QString &reauthPrompt() const override;
    const QString &authFailedPrompt() const override;

    void walletPasswordRequestFinished(const QString &password) override;
    void walletMapRequestFinished(const QMap<QString, QString> &map) override;

private:
    std::unique_ptr<EwsOAuthPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(EwsOAuth)
};
