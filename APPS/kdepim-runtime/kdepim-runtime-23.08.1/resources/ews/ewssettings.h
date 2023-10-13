/*
    SPDX-FileCopyrightText: 2017-2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#ifdef EWSSETTINGS_UNITTEST
#include "ewssettings_ut_mock.h"
#else
#include "ewssettingsbase.h"
#endif

#include <QPointer>
#include <QTimer>

namespace KWallet
{
class Wallet;
}

class EwsAbstractAuth;

class EwsSettings : public EwsSettingsBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.Ews.Wallet")
public:
    explicit EwsSettings(WId windowId);
    ~EwsSettings() override;

    void requestPassword();
    void requestMap();

    EwsAbstractAuth *loadAuth(QObject *parent);
public Q_SLOTS:
    Q_SCRIPTABLE void setPassword(const QString &password);
    Q_SCRIPTABLE void setMap(const QMap<QString, QString> &map);
    Q_SCRIPTABLE void setTestPassword(const QString &password);
Q_SIGNALS:
    void passwordRequestFinished(const QString &password);
    void mapRequestFinished(const QMap<QString, QString> &map);
private Q_SLOTS:
    void onWalletOpened(bool success);

private:
    QString readPassword() const;
    QMap<QString, QString> readMap() const;
    void satisfyPasswordReadRequest(bool success);
    void satisfyPasswordWriteRequest(bool success);
    void satisfyMapReadRequest(bool success);
    void satisfyMapWriteRequest(bool success);
    bool requestWalletOpen();
    WId mWindowId;

    QString mPassword;
    bool mPasswordReadPending = false;
    bool mPasswordWritePending = false;

    QMap<QString, QString> mMap;
    bool mMapReadPending = false;
    bool mMapWritePending = false;

    QPointer<KWallet::Wallet> mWallet;
    QTimer mWalletTimer;
};
