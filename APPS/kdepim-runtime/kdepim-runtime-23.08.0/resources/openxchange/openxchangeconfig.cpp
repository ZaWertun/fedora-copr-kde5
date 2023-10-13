/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <Akonadi/AgentConfigurationBase>

#include "configwidget.h"
#include "settings.h"

class OpenXChangeConfig : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    OpenXChangeConfig(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
        : Akonadi::AgentConfigurationBase(config, parent, args)
    {
        Settings::instance(config);
        mSettings.reset(Settings::self());
        mWidget.reset(new ConfigWidget(mSettings.data(), parent));
    }

    void load() override
    {
        Akonadi::AgentConfigurationBase::load();
        mWidget->load();
    }

    Q_REQUIRED_RESULT bool save() const override
    {
        mWidget->save();
        return Akonadi::AgentConfigurationBase::save();
    }

    QScopedPointer<Settings> mSettings;
    QScopedPointer<ConfigWidget> mWidget;
};

AKONADI_AGENTCONFIG_FACTORY(OpenXChangeConfigFactory, "openxchangeconfig.json", OpenXChangeConfig)

#include "openxchangeconfig.moc"
