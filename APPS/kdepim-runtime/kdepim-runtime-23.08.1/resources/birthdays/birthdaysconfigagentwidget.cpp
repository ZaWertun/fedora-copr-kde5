/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "birthdaysconfigagentwidget.h"
#include "settings.h"
#include <Akonadi/Tag>
#include <KConfigDialogManager>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QIcon>
#include <QPushButton>

namespace
{
static const char myConfigGroupName[] = "BirthdaysSettingsDialog";
}

BirthdaysConfigAgentWidget::BirthdaysConfigAgentWidget(const KSharedConfigPtr &config, QWidget *parent, const QVariantList &args)
    : Akonadi::AgentConfigurationBase(config, parent, args)
{
    // setWindowIcon(QIcon::fromTheme(QStringLiteral("view-calendar-birthday")));

    Settings::instance(config);

    auto mainWidget = new QWidget(parent);
    ui.setupUi(mainWidget);
    parent->layout()->addWidget(mainWidget);

    mManager = new KConfigDialogManager(mainWidget, Settings::self());
    mManager->updateWidgets();
    ui.kcfg_AlarmDays->setSuffix(ki18np(" day", " days"));
}

BirthdaysConfigAgentWidget::~BirthdaysConfigAgentWidget() = default;

void BirthdaysConfigAgentWidget::load()
{
    const QStringList categories = Settings::self()->filterCategories();
    ui.FilterCategories->setSelectionFromStringList(categories);
}

bool BirthdaysConfigAgentWidget::save() const
{
    mManager->updateSettings();

    Settings::self()->setFilterCategories(ui.FilterCategories->tagToStringList());
    Settings::self()->save();
    return true;
}

QSize BirthdaysConfigAgentWidget::restoreDialogSize() const
{
    const auto group = config()->group(myConfigGroupName);
    const QSize size = group.readEntry("Size", QSize(600, 400));
    return size;
}

void BirthdaysConfigAgentWidget::saveDialogSize(const QSize &size)
{
    auto group = config()->group(myConfigGroupName);
    group.writeEntry("Size", size);
}
