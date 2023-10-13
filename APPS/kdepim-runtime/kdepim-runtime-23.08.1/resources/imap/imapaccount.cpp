/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapaccount.h"

ImapAccount::ImapAccount()
    : m_encryption(KIMAP::LoginJob::Unencrypted)
    , m_authentication(KIMAP::LoginJob::ClearText)
{
}

ImapAccount::~ImapAccount() = default;

void ImapAccount::setServer(const QString &server)
{
    m_server = server;
}

QString ImapAccount::server() const
{
    return m_server;
}

void ImapAccount::setPort(quint16 port)
{
    m_port = port;
}

quint16 ImapAccount::port() const
{
    return m_port;
}

void ImapAccount::setUserName(const QString &userName)
{
    m_userName = userName;
}

QString ImapAccount::userName() const
{
    return m_userName;
}

void ImapAccount::setEncryptionMode(KIMAP::LoginJob::EncryptionMode mode)
{
    m_encryption = mode;
}

KIMAP::LoginJob::EncryptionMode ImapAccount::encryptionMode() const
{
    return m_encryption;
}

void ImapAccount::setAuthenticationMode(KIMAP::LoginJob::AuthenticationMode mode)
{
    m_authentication = mode;
}

KIMAP::LoginJob::AuthenticationMode ImapAccount::authenticationMode() const
{
    return m_authentication;
}

void ImapAccount::setSubscriptionEnabled(bool enabled)
{
    m_subscriptionEnabled = enabled;
}

bool ImapAccount::isSubscriptionEnabled() const
{
    return m_subscriptionEnabled;
}

void ImapAccount::setTimeout(int timeout)
{
    m_timeout = timeout;
}

int ImapAccount::timeout() const
{
    return m_timeout;
}

void ImapAccount::setUseNetworkProxy(bool useProxy)
{
    m_useProxy = useProxy;
}

bool ImapAccount::useNetworkProxy() const
{
    return m_useProxy;
}
