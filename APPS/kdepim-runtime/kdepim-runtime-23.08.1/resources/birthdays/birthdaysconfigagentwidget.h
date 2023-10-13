/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ui_birthdaysconfigwidget.h"
#include <Akonadi/AgentConfigurationBase>

class KConfigDialogManager;

class BirthdaysConfigAgentWidget : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    explicit BirthdaysConfigAgentWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args);
    ~BirthdaysConfigAgentWidget() override;

    void load() override;
    bool save() const override;
    QSize restoreDialogSize() const override;
    void saveDialogSize(const QSize &size) override;

private:
    Ui::BirthdaysConfigWidget ui;
    KConfigDialogManager *mManager = nullptr;
};
AKONADI_AGENTCONFIG_FACTORY(BirthdaysConfigAgentWidgetFactory, "birthdaysconfig.json", BirthdaysConfigAgentWidget)
