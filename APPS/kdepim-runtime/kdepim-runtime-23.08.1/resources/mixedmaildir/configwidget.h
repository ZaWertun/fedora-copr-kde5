/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ui_settings.h"
class QPushButton;
class KConfigDialogManager;
class Settings;
class ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(Settings *settings, QWidget *parent = nullptr);

    void load(Settings *settings);
    void save(Settings *settings) const;

Q_SIGNALS:
    void okEnabled(bool enabled);

private Q_SLOTS:
    void checkPath();

private:
    Ui::ConfigWidget ui;
    KConfigDialogManager *mManager = nullptr;
    QPushButton *mOkButton = nullptr;
    bool mToplevelIsContainer = false;
};
