/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ui_vcarddiragentsettingswidget.h"
#include <Akonadi/AgentConfigurationBase>

class KConfigDialogManager;

class VcardDirSettingsWidget : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    explicit VcardDirSettingsWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args);

    void load() override;
    bool save() const override;

private:
    void validate();
    Ui::VcardDirsAgentSettingsWidget ui;
    KConfigDialogManager *mManager = nullptr;
};

AKONADI_AGENTCONFIG_FACTORY(VcardDirSettingsWidgetFactory, "vcarddirconfig.json", VcardDirSettingsWidget)
