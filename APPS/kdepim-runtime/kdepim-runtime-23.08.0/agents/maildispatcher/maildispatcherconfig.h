/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/AgentConfigurationBase>

class KNotifyConfigWidget;
class MailDispatcherConfig : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    explicit MailDispatcherConfig(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &cfg);

    bool save() const override;

private:
    KNotifyConfigWidget *const mWidget;
};

AKONADI_AGENTCONFIG_FACTORY(MailDispatcherConfigFactory, "maildispatcherconfig.json", MailDispatcherConfig)
