/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "o1tomboy.h"

O1Tomboy::O1Tomboy(QObject *parent)
    : O1(parent)
{
}

void O1Tomboy::setBaseURL(const QString &value)
{
    setRequestTokenUrl(QUrl(value + QStringLiteral("/oauth/request_token")));
    setAuthorizeUrl(QUrl(value + QStringLiteral("/oauth/authorize")));
    setAccessTokenUrl(QUrl(value + QStringLiteral("/oauth/access_token")));
    setClientId(QStringLiteral("anyone"));
    setClientSecret(QStringLiteral("anyone"));
}

QString O1Tomboy::getRequestToken() const
{
    return requestToken_;
}

QString O1Tomboy::getRequestTokenSecret() const
{
    return requestTokenSecret_;
}

void O1Tomboy::restoreAuthData(const QString &token, const QString &secret)
{
    requestToken_ = token;
    requestTokenSecret_ = secret;
    setLinked(true);
}
