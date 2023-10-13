/*
   SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
   SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "settingsbase.h"

#include <QPointer>
#include <qwindowdefs.h>

#include <KGAPI/Types>

namespace QKeychain
{
class ReadPasswordJob;
class WritePasswordJob;
}

/**
 * @brief Settings object
 *
 * Provides read-only access to application clientId and
 * clientSecret and read-write access to accessToken and
 * refreshToken. Interacts with QtKeyChain.
 */
class GoogleSettings : public SettingsBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.Google.ExtendedSettings")

public:
    enum class Option {
        NoOption = 0,
        ExportToDBus = 1,
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit GoogleSettings(const KSharedConfigPtr &config, Options options = Option::ExportToDBus);

    void init();
    void setWindowId(WId id);
    void setResourceId(const QString &resourceIdentifier);

    QString appId() const;
    QString clientId() const;
    QString clientSecret() const;

    void addCalendar(const QString &calendar);
    void addTaskList(const QString &taskList);

    KGAPI2::AccountPtr accountPtr();
    // Wallet
    bool isReady() const;
    QKeychain::WritePasswordJob *storeAccount(KGAPI2::AccountPtr account);
    void cleanup();

Q_SIGNALS:
    void accountReady(bool ready);
    void accountChanged();
    void okEnabled(bool enabled);

private:
    void slotWalletOpened(bool success);
    WId m_winId = 0;
    QString m_resourceId;
    bool m_isReady = false;
    KGAPI2::AccountPtr m_account;
    KGAPI2::AccountPtr fetchAccountFromKeychain(const QString &accountName, QKeychain::ReadPasswordJob *job);
};
