/*
 *   SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>
 *   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>
 *   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#pragma once

#include "ui_popsettings.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif
class Settings;

namespace MailTransport
{
class ServerTest;
}

class KJob;

class AccountWidget : public QWidget, private Ui::PopPage
{
    Q_OBJECT

public:
    AccountWidget(Settings &settings, const QString &identifier, QWidget *parent);
    ~AccountWidget() override;

    void loadSettings();
    void saveSettings();

Q_SIGNALS:
    void okEnabled(bool enabled);

private Q_SLOTS:
    void slotEnablePopInterval(bool state);
    void slotLeaveOnServerClicked();
    void slotEnableLeaveOnServerDays(bool state);
    void slotEnableLeaveOnServerCount(bool state);
    void slotEnableLeaveOnServerSize(bool state);
    void slotPipeliningClicked();
    void slotPopEncryptionChanged(QAbstractButton *button);
    void slotCheckPopCapabilities();
    void slotPopCapabilities(const QVector<int> &);
    void slotLeaveOnServerDaysChanged(int value);
    void slotLeaveOnServerCountChanged(int value);

    void targetCollectionReceived(Akonadi::Collection::List collections);
    void localFolderRequestJobFinished(KJob *job);
    void walletOpenedForLoading(QKeychain::Job *baseJob);
    void walletOpenedForSaving();
    void slotAccepted();

private:
    void setupWidgets();
    void checkHighest(QButtonGroup *);
    void enablePopFeatures();
    void populateDefaultAuthenticationOptions();

private:
    QButtonGroup *encryptionButtonGroup = nullptr;
    MailTransport::ServerTest *mServerTest = nullptr;
    QRegularExpressionValidator mValidator;
    bool mServerTestFailed = false;
    QString mInitalPassword;
    const QString mIdentifier;
    Settings &mSettings;
};
