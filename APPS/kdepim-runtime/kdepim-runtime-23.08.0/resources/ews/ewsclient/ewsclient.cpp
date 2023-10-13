/*
    SPDX-FileCopyrightText: 2015-2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsclient.h"

#include "auth/ewsabstractauth.h"
#include "ewsclient_debug.h"

QHash<QString, QString> EwsClient::folderHash;

EwsClient::EwsClient(QObject *parent)
    : QObject(parent)
    , mEnableNTLMv2(true)
{
}

EwsClient::~EwsClient()
{
}

void EwsClient::setServerVersion(const EwsServerVersion &version)
{
    if (mServerVersion.isValid() && mServerVersion != version) {
        qCWarning(EWSCLI_LOG) << "Warning - server version changed." << mServerVersion << version;
    }
    mServerVersion = version;
}

QUrl EwsClient::url() const
{
    return mUrl;
}

void EwsClient::setAuth(EwsAbstractAuth *auth)
{
    mAuth = auth;
}

EwsAbstractAuth *EwsClient::auth() const
{
    return mAuth.data();
}
