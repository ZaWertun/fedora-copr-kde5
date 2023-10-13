/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configwidget.h"
#include "resources/folderarchivesettings/folderarchivesettingpage.h"
#include "settings.h"

#include <maildir.h>

#include <KConfigDialogManager>
#include <KLineEdit>
#include <QVBoxLayout>

using KPIM::Maildir;
using namespace Akonadi_Maildir_Resource;

ConfigWidget::ConfigWidget(MaildirSettings *settings, const QString &identifier, QWidget *parent)
    : QWidget(parent)
    , mSettings(settings)
    , mToplevelIsContainer(false)
{
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    ui.setupUi(mainWidget);
    mFolderArchiveSettingPage = new FolderArchiveSettingPage(identifier, this);
    ui.tabWidget->addTab(mFolderArchiveSettingPage, i18n("Archive Folder"));

    ui.kcfg_Path->setMode(KFile::Directory | KFile::ExistingOnly);
    ui.kcfg_Path->setUrl(QUrl::fromLocalFile(mSettings->path()));

    connect(ui.kcfg_Path->lineEdit(), &QLineEdit::textChanged, this, &ConfigWidget::checkPath);
    ui.kcfg_Path->lineEdit()->setFocus();
    checkPath();
}

ConfigWidget::~ConfigWidget() = default;

void ConfigWidget::checkPath()
{
    if (ui.kcfg_Path->url().isEmpty()) {
        ui.statusLabel->setText(i18n("The selected path is empty."));
        Q_EMIT okEnabled(false);
        return;
    }
    bool ok = false;
    mToplevelIsContainer = false;
    QDir d(ui.kcfg_Path->url().toLocalFile());

    if (d.exists()) {
        Maildir md(d.path());
        if (!md.isValid(false)) {
            Maildir md2(d.path(), true);
            if (md2.isValid(false)) {
                ui.statusLabel->setText(i18n("The selected path contains valid Maildir folders."));
                mToplevelIsContainer = true;
                ok = true;
            } else {
                ui.statusLabel->setText(md.lastError());
            }
        } else {
            ui.statusLabel->setText(i18n("The selected path is a valid Maildir."));
            ok = true;
        }
    } else {
        d.cdUp();
        if (d.exists()) {
            ui.statusLabel->setText(i18n("The selected path does not exist yet, a new Maildir will be created."));
            mToplevelIsContainer = true;
            ok = true;
        } else {
            ui.statusLabel->setText(i18n("The selected path does not exist."));
        }
    }
    Q_EMIT okEnabled(ok);
}

void ConfigWidget::load()
{
    mFolderArchiveSettingPage->loadSettings();
    mManager = new KConfigDialogManager(this, mSettings);
    mManager->updateWidgets();
}

bool ConfigWidget::save() const
{
    mFolderArchiveSettingPage->writeSettings();
    mManager->updateSettings();
    QString path = ui.kcfg_Path->url().isLocalFile() ? ui.kcfg_Path->url().toLocalFile() : ui.kcfg_Path->url().path();
    mSettings->setPath(path);
    mSettings->setTopLevelIsContainer(mToplevelIsContainer);
    mSettings->save();

    if (ui.kcfg_Path->url().isLocalFile()) {
        QDir d(path);
        if (!d.exists()) {
            d.mkpath(ui.kcfg_Path->url().toLocalFile());
        }
    }

    return true;
}
