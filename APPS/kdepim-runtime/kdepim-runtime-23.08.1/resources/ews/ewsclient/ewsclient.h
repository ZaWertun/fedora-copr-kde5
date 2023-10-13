/*
    SPDX-FileCopyrightText: 2015-2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>

#include "ewsserverversion.h"

class EwsAbstractAuth;

class EwsClient : public QObject
{
    Q_OBJECT
public:
    explicit EwsClient(QObject *parent = nullptr);
    ~EwsClient() override;

    void setUrl(const QString &url)
    {
        mUrl.setUrl(url);
    }

    void setAuth(EwsAbstractAuth *auth);
    EwsAbstractAuth *auth() const;

    enum RequestedConfiguration {
        MailTips = 0,
        UnifiedMessagingConfiguration,
        ProtectionRules,
    };

    QUrl url() const;

    bool isConfigured() const
    {
        return !mUrl.isEmpty();
    }

    void setServerVersion(const EwsServerVersion &version);
    const EwsServerVersion &serverVersion() const
    {
        return mServerVersion;
    }

    void setUserAgent(const QString &userAgent)
    {
        mUserAgent = userAgent;
    }

    const QString &userAgent() const
    {
        return mUserAgent;
    }

    void setEnableNTLMv2(bool enable)
    {
        mEnableNTLMv2 = enable;
    }

    bool isNTLMv2Enabled() const
    {
        return mEnableNTLMv2;
    }

    static QHash<QString, QString> folderHash;
Q_SIGNALS:
    void oAuthTokensChanged(const QString &accessToken, const QString &refreshToken);
    void oAuthBrowserDisplayRequest();

private:
    QUrl mUrl;

    QPointer<EwsAbstractAuth> mAuth;

    QString mUserAgent;
    bool mEnableNTLMv2;

    EwsServerVersion mServerVersion;

    friend class EwsRequest;
};
