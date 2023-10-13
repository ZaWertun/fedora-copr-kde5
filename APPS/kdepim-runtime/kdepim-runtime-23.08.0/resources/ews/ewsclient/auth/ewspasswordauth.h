/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "ewsabstractauth.h"

class EwsPasswordAuth : public EwsAbstractAuth
{
    Q_OBJECT
public:
    explicit EwsPasswordAuth(const QString &username, QObject *parent = nullptr);
    ~EwsPasswordAuth() override = default;

    void init() override;
    bool getAuthData(QString &username, QString &password, QStringList &customHeaders) override;
    void notifyRequestAuthFailed() override;
    bool authenticate(bool interactive) override;
    const QString &reauthPrompt() const override;
    const QString &authFailedPrompt() const override;

    void walletPasswordRequestFinished(const QString &password) override;
    void walletMapRequestFinished(const QMap<QString, QString> &map) override;

    void setUsername(const QString &username);

protected:
    QString mUsername;
    QString mPassword;
};
