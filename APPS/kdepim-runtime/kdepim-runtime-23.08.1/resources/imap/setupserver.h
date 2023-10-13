/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
   SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2006-2008 Omat Holding B.V. <info@omat.nl>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class KJob;
#include <Akonadi/Collection>
#include <QDialog>

#include <QRegularExpressionValidator>
class QPushButton;
class QComboBox;
namespace Ui
{
class SetupServerView;
}

namespace MailTransport
{
class ServerTest;
}
namespace KIdentityManagement
{
class IdentityCombo;
}
class FolderArchiveSettingPage;
class ImapResourceBase;

/**
 * @class SetupServer
 * These contain the account settings
 * @author Tom Albers <tomalbers@kde.nl>
 */
class SetupServer : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parentResource The resource this dialog belongs to
     * @param parent Parent WId
     */
    explicit SetupServer(ImapResourceBase *parentResource, WId parent);

    /**
     * Destructor
     */
    ~SetupServer() override;

    Q_REQUIRED_RESULT bool shouldClearCache() const;

private:
    /**
     * Call this if you want the settings saved from this page.
     */
    void applySettings();
    void slotMailCheckboxChanged();
    void slotEncryptionRadioChanged();
    void slotSubcriptionCheckboxChanged();
    void slotShowServerInfo();
    void readSettings();
    void populateDefaultAuthenticationOptions();

    ImapResourceBase *const m_parentResource;
    Ui::SetupServerView *const m_ui;
    MailTransport::ServerTest *m_serverTest = nullptr;
    bool m_subscriptionsChanged = false;
    bool m_shouldClearCache = false;
    QString m_vacationFileName;
    KIdentityManagement::IdentityCombo *m_identityCombobox = nullptr;
    QString m_oldResourceName;
    QRegularExpressionValidator mValidator;
    Akonadi::Collection mOldTrash;
    FolderArchiveSettingPage *m_folderArchiveSettingPage = nullptr;
    QPushButton *mOkButton = nullptr;

private Q_SLOTS:
    void slotTest();
    void slotFinished(const QVector<int> &testResult);
    void slotCustomSieveChanged();

    void slotServerChanged();
    void slotTestChanged();
    void slotComplete();
    void slotSafetyChanged();
    void slotManageSubscriptions();
    void slotEnableWidgets();
    void targetCollectionReceived(const Akonadi::Collection::List &collections);
    void localFolderRequestJobFinished(KJob *job);
    void populateDefaultAuthenticationOptions(QComboBox *combo);
};
