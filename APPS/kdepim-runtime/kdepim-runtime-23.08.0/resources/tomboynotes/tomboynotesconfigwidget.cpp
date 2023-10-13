/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>
    SPDX-FileCopyrightText: 2018-2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboynotesconfigwidget.h"
#include "settings.h"
#include "ui_tomboynotesagentconfigwidget.h"
#include <KConfigDialogManager>
#include <KLocalizedString>

TomboyNotesConfigWidget::TomboyNotesConfigWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
    , ui(new Ui::TomboyNotesAgentConfigWidget)
{
    Settings::instance(config);

    auto mainWidget = new QWidget(parent);
    ui->setupUi(mainWidget);
    parent->layout()->addWidget(mainWidget);

    // KSettings stuff. Works not in the initialization list!
    mManager = new KConfigDialogManager(mainWidget, Settings::self());
    mManager->updateWidgets();

    ui->kcfg_ServerURL->setReadOnly(!Settings::self()->requestToken().isEmpty());
}

TomboyNotesConfigWidget::~TomboyNotesConfigWidget()
{
    delete ui;
}

void TomboyNotesConfigWidget::load()
{
}

bool TomboyNotesConfigWidget::save() const
{
    if (ui->kcfg_ServerURL->text() != Settings::self()->serverURL()) {
        Settings::self()->setRequestToken(QString());
        Settings::self()->setRequestTokenSecret(QString());
    }

    if (ui->kcfg_collectionName->text().isEmpty()) {
        ui->kcfg_collectionName->setText(i18n("Tomboy Notes %1", Settings::serverURL()));
    }

    mManager->updateSettings();
    Settings::self()->save();
    return true;
}

QSize TomboyNotesConfigWidget::restoreDialogSize() const
{
    auto group = config()->group("TomboyNotesConfigWidget");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    return size;
}

void TomboyNotesConfigWidget::saveDialogSize(const QSize &size)
{
    auto group = config()->group("TomboyNotesConfigWidget");
    group.writeEntry("Size", size);
}
