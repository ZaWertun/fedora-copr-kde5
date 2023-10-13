/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"
#include "singlefileresourceconfigbase.h"

class ICalConfigBase : public SingleFileResourceConfigBase<SETTINGS_NAMESPACE::Settings>
{
public:
    ICalConfigBase(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
        : SingleFileResourceConfigBase<SETTINGS_NAMESPACE::Settings>(config, parent, args)
    {
        mWidget->setFilter(QStringLiteral("text/calendar"));
    }
};

class ICalConfig : public ICalConfigBase
{
    Q_OBJECT
public:
    ~ICalConfig() override = default;

    using ICalConfigBase::ICalConfigBase;
};

AKONADI_AGENTCONFIG_FACTORY(ICalConfigFactory, "icalconfig.json", ICalConfig)

#include "icalconfig.moc"
