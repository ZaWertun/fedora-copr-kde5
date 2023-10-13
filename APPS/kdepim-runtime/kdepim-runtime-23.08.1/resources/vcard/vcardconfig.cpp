/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"
#include "singlefileresourceconfigbase.h"

class VCardConfigBase : public SingleFileResourceConfigBase<SETTINGS_NAMESPACE::Settings>
{
public:
    VCardConfigBase(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &list)
        : SingleFileResourceConfigBase(config, parent, list)
    {
        mWidget->setFilter(QStringLiteral("*.vcf|") + i18nc("Filedialog filter for *.vcf", "vCard Address Book File"));
    }
};

class VCardConfig : public VCardConfigBase
{
    Q_OBJECT
public:
    ~VCardConfig() override = default;

    using VCardConfigBase::VCardConfigBase;
};

AKONADI_AGENTCONFIG_FACTORY(VCardConfigFactory, "vcardconfig.json", VCardConfig)

#include "vcardconfig.moc"
