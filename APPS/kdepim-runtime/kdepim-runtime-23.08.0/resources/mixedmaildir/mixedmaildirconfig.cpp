/*  SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <Akonadi/AgentConfigurationBase>

#include "configwidget.h"
#include "settings.h"

class MixedMaildirConfig : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    MixedMaildirConfig(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
        : Akonadi::AgentConfigurationBase(config, parent, args)
    {
        Settings::instance(config);
        mSettings.reset(Settings::self());
        mWidget.reset(new ConfigWidget(mSettings.data(), parent));
        connect(mWidget.data(), &ConfigWidget::okEnabled, this, &Akonadi::AgentConfigurationBase::enableOkButton);
    }

    void load() override
    {
        Akonadi::AgentConfigurationBase::load();
        mWidget->load(mSettings.data());
    }

    Q_REQUIRED_RESULT bool save() const override
    {
        mWidget->save(mSettings.data());
        mSettings->save();
        return Akonadi::AgentConfigurationBase::save();
    }

private:
    QScopedPointer<Settings> mSettings;
    QScopedPointer<ConfigWidget> mWidget;
};

AKONADI_AGENTCONFIG_FACTORY(MixedMaildirConfigFactory, "mixedmaildirconfig.json", MixedMaildirConfig)

#include "mixedmaildirconfig.moc"
