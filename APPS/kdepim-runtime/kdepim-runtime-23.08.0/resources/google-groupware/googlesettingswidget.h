/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "googlesettings.h"
#include "ui_googlesettingswidget.h"
#include <KGAPI/Types>

namespace KGAPI2
{
class Job;
}
class GoogleSettingsWidget : public QWidget, private Ui::GoogleSettingsWidget
{
    Q_OBJECT
public:
    explicit GoogleSettingsWidget(GoogleSettings &settings, const QString &identifier, QWidget *parent);
    ~GoogleSettingsWidget() override;

    void loadSettings();
    void saveSettings();

Q_SIGNALS:
    void okEnabled(bool enabled);

protected:
    bool handleError(KGAPI2::Job *job);
    void accountChanged();

private:
    void slotAuthJobFinished(KGAPI2::Job *job);
    void slotReloadCalendars();
    void slotReloadTaskLists();

    GoogleSettings &m_settings;
    KGAPI2::AccountPtr m_account;
    const QString m_identifier;
};
