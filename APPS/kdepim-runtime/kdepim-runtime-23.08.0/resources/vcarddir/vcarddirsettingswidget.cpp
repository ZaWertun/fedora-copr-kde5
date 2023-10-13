/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcarddirsettingswidget.h"

#include "settings.h"

#include <KConfigDialogManager>
#include <KLocalizedString>
#include <QUrl>

#include <QPushButton>
#include <QTimer>

VcardDirSettingsWidget::VcardDirSettingsWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
{
    VcardDirResourceSettings::instance(config);

    auto mainWidget = new QWidget(parent);

    ui.setupUi(mainWidget);
    parent->layout()->addWidget(mainWidget);

    ui.kcfg_Path->setMode(KFile::LocalOnly | KFile::Directory);

    connect(ui.kcfg_Path, &KUrlRequester::textChanged, this, &VcardDirSettingsWidget::validate);
    connect(ui.kcfg_ReadOnly, &QCheckBox::toggled, this, &VcardDirSettingsWidget::validate);

    ui.kcfg_Path->setUrl(QUrl::fromLocalFile(VcardDirResourceSettings::self()->path()));
    ui.kcfg_AutosaveInterval->setSuffix(ki18np(" minute", " minutes"));
    mManager = new KConfigDialogManager(mainWidget, VcardDirResourceSettings::self());
}

void VcardDirSettingsWidget::validate()
{
    const QUrl currentUrl = ui.kcfg_Path->url();
    if (currentUrl.isEmpty()) {
        Q_EMIT enableOkButton(false);
        return;
    }

    const QFileInfo file(currentUrl.toLocalFile());
    if (file.exists() && !file.isWritable()) {
        ui.kcfg_ReadOnly->setEnabled(false);
        ui.kcfg_ReadOnly->setChecked(true);
    } else {
        ui.kcfg_ReadOnly->setEnabled(true);
    }
    Q_EMIT enableOkButton(true);
}

void VcardDirSettingsWidget::load()
{
    mManager->updateWidgets();
    QTimer::singleShot(0, this, &VcardDirSettingsWidget::validate);
}

bool VcardDirSettingsWidget::save() const
{
    mManager->updateSettings();
    VcardDirResourceSettings::self()->setPath(ui.kcfg_Path->url().toLocalFile());
    VcardDirResourceSettings::self()->save();
    return true;
}
