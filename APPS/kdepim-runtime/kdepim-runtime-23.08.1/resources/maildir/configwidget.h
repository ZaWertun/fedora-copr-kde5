/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include "ui_settings.h"
class QPushButton;
class KConfigDialogManager;
namespace Akonadi_Maildir_Resource
{
class MaildirSettings;
}
class FolderArchiveSettingPage;
class ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(Akonadi_Maildir_Resource::MaildirSettings *settings, const QString &identifier, QWidget *parent = nullptr);
    ~ConfigWidget() override;

    void load();
    bool save() const;

Q_SIGNALS:
    void okEnabled(bool enabled);

private Q_SLOTS:
    void checkPath();

private:
    Ui::ConfigWidget ui;
    KConfigDialogManager *mManager = nullptr;
    FolderArchiveSettingPage *mFolderArchiveSettingPage = nullptr;
    Akonadi_Maildir_Resource::MaildirSettings *mSettings = nullptr;
    bool mToplevelIsContainer = false;
    QPushButton *mOkButton = nullptr;
};
