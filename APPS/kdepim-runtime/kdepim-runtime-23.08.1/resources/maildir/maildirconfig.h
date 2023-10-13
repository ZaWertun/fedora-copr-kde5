/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/AgentConfigurationBase>

#include "configwidget.h"
#include "settings.h"

#include <QScopedPointer>

class MaildirConfig : public Akonadi::AgentConfigurationBase
{
    Q_OBJECT
public:
    MaildirConfig(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args);
    ~MaildirConfig() override;

    void load() override;
    bool save() const override;

private:
    QScopedPointer<Akonadi_Maildir_Resource::MaildirSettings> mSettings;
    QScopedPointer<ConfigWidget> mWidget;
};
