/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "settingsbase.h"

#include <KIMAP/LoginJob>

#include <MailTransport/Transport>

class ImapAccount;
class KJob;

class Settings : public SettingsBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.Imap.Wallet")
public:
    static KIMAP::LoginJob::AuthenticationMode mapTransportAuthToKimap(MailTransport::Transport::EnumAuthenticationType authType);

    explicit Settings(WId = 0);
    void setWinId(WId);

    virtual void requestPassword();

    virtual void loadAccount(ImapAccount *account) const;

    Q_REQUIRED_RESULT QString rootRemoteId() const;
    virtual void renameRootCollection(const QString &newName);

    virtual void clearCachedPassword();
    virtual void cleanup();

    virtual QString password(bool *userRejected = nullptr) const;
    virtual QString sieveCustomPassword(bool *userRejected = nullptr) const;

Q_SIGNALS:
    void passwordRequestCompleted(const QString &password, bool userRejected);

public Q_SLOTS:
    Q_SCRIPTABLE virtual void setPassword(const QString &password);
    Q_SCRIPTABLE virtual void setSieveCustomPassword(const QString &password);

protected Q_SLOTS:
    virtual void onWalletOpened(bool success);

    void onRootCollectionFetched(KJob *job);

protected:
    WId m_winId;
    mutable QString m_password;
    mutable QString m_customSievePassword;
};
