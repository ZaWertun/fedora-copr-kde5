/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QDialog>
class NewMailNotificationHistoryWidget;
class NewMailNotificationHistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewMailNotificationHistoryDialog(QWidget *parent = nullptr);
    ~NewMailNotificationHistoryDialog() override;

private:
    void readConfig();
    void writeConfig();
    NewMailNotificationHistoryWidget *const mNewHistoryNotificationWidget;
};
