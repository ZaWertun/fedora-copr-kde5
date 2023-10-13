/*
    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "compactpage.h"
#include "lockmethodpage.h"
#include "settings.h"
#include "singlefileresourceconfigbase.h"

#include <KLocalizedString>

class MBoxConfigBase : public SingleFileResourceConfigBase<Settings>
{
public:
    MBoxConfigBase(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
        : SingleFileResourceConfigBase<Settings>(config, parent, args)
    {
        mWidget->setFilter(QStringLiteral("*.mbox|") + i18nc("Filedialog filter for *.mbox", "MBox file"));
        mWidget->addPage(i18n("Compact frequency"), new CompactPage(mSettings->path()));
        mWidget->addPage(i18n("Lock method"), new LockMethodPage());
    }
};

class MBoxConfig : public MBoxConfigBase
{
    Q_OBJECT
public:
    using MBoxConfigBase::MBoxConfigBase;
};

AKONADI_AGENTCONFIG_FACTORY(MBoxConfigFactory, "mboxconfig.json", MBoxConfig)

#include "mboxconfig.moc"
