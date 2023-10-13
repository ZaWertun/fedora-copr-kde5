/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QDialog;
#include <passwordrequesterinterface.h>

class ImapResourceBase;

class SettingsPasswordRequester : public PasswordRequesterInterface
{
    Q_OBJECT

public:
    explicit SettingsPasswordRequester(ImapResourceBase *resource, QObject *parent = nullptr);
    ~SettingsPasswordRequester() override;

    void requestPassword(RequestType request = StandardRequest, const QString &serverError = QString()) override;
    void cancelPasswordRequests() override;

private Q_SLOTS:
    void askUserInput(const QString &serverError);
    void onPasswordRequestCompleted(const QString &password, bool userRejected);
    void onDialogDestroyed();
    void onSettingsDialogFinished(int result);

private:
    void slotCancelClicked();
    void slotYesClicked();
    void slotNoClicked();
    QString requestManualAuth(bool *userRejected);

    ImapResourceBase *const m_resource;
    QDialog *m_requestDialog = nullptr;
    QDialog *m_settingsDialog = nullptr;
};
