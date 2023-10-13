/*
   SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <Akonadi/AgentConfigurationBase>

#include "accountwidget.h"
#include "settings.h"

class Pop3Config : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    Pop3Config(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
        : Akonadi::AgentConfigurationBase(config, parent, args)
        , mSettings(config, Settings::Option::NoOption)
        , mWidget(mSettings, identifier(), parent)
    {
        connect(&mWidget, &AccountWidget::okEnabled, this, &Akonadi::AgentConfigurationBase::enableOkButton);
    }

    void load() override
    {
        Akonadi::AgentConfigurationBase::load();
        mWidget.loadSettings();
    }

    Q_REQUIRED_RESULT bool save() const override
    {
        const_cast<Pop3Config *>(this)->mWidget.saveSettings();
        return Akonadi::AgentConfigurationBase::save();
    }

    Settings mSettings;
    AccountWidget mWidget;
};

AKONADI_AGENTCONFIG_FACTORY(Pop3ConfigFactory, "pop3config.json", Pop3Config)

#include "pop3config.moc"
