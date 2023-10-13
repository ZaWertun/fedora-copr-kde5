/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/AgentConfigurationBase>
class KConfigDialogManager;

namespace Ui
{
class TomboyNotesAgentConfigWidget;
}

class TomboyNotesConfigWidget : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    explicit TomboyNotesConfigWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args);
    ~TomboyNotesConfigWidget() override;

    void load() override;
    bool save() const override;
    QSize restoreDialogSize() const override;
    void saveDialogSize(const QSize &size) override;

private:
    Ui::TomboyNotesAgentConfigWidget *const ui;

    KConfigDialogManager *mManager = nullptr;
};
AKONADI_AGENTCONFIG_FACTORY(TomboyNotesConfigWidgetFactory, "tomboynotesconfig.json", TomboyNotesConfigWidget)
