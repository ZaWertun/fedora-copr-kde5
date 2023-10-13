/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaldirsettingswidget.h"

#include "settings.h"

#include <KConfigDialogManager>
#include <KLocalizedString>
#include <QUrl>

#include <QFontDatabase>
#include <QPushButton>
#include <QTimer>

IcalDirSettingsWidget::IcalDirSettingsWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
{
    IcalDirResourceSettings::instance(config);

    auto mainWidget = new QWidget(parent);

    ui.setupUi(mainWidget);
    parent->layout()->addWidget(mainWidget);

    ui.kcfg_Path->setMode(KFile::LocalOnly | KFile::Directory);

    connect(ui.kcfg_Path, &KUrlRequester::textChanged, this, &IcalDirSettingsWidget::validate);
    connect(ui.kcfg_ReadOnly, &QCheckBox::toggled, this, &IcalDirSettingsWidget::validate);

    ui.kcfg_Path->setUrl(QUrl::fromLocalFile(IcalDirResourceSettings::self()->path()));
    ui.kcfg_AutosaveInterval->setSuffix(ki18np(" minute", " minutes"));
    mManager = new KConfigDialogManager(mainWidget, IcalDirResourceSettings::self());
    ui.readOnlyLabel->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    ui.runingLabel->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    ui.pathLabel->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
}

void IcalDirSettingsWidget::validate()
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

void IcalDirSettingsWidget::load()
{
    mManager->updateWidgets();
    QTimer::singleShot(0, this, &IcalDirSettingsWidget::validate);
}

bool IcalDirSettingsWidget::save() const
{
    mManager->updateSettings();
    IcalDirResourceSettings::self()->setPath(ui.kcfg_Path->url().toLocalFile());
    IcalDirResourceSettings::self()->save();
    return true;
}
