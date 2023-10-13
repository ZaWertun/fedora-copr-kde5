/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <KGAPI/Types>
#include <QDialog>

namespace Ui
{
class GoogleSettingsDialog;
}
namespace KGAPI2
{
class Job;
}
class GoogleResource;
class GoogleSettings;

class GoogleSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GoogleSettingsDialog(GoogleResource *resource, GoogleSettings *settings, WId wId);
    ~GoogleSettingsDialog() override;

protected:
    bool handleError(KGAPI2::Job *job);
    void accountChanged();

private:
    void slotConfigure();
    void slotAuthJobFinished(KGAPI2::Job *job);
    void slotSaveSettings();
    void slotReloadCalendars();
    void slotReloadTaskLists();

    GoogleResource *const m_resource;
    GoogleSettings *const m_settings;
    Ui::GoogleSettingsDialog *const m_ui;
    KGAPI2::AccountPtr m_account;
};
