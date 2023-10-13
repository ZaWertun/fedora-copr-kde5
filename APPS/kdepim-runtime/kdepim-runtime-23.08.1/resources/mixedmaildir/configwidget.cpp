/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configwidget.h"
#include "settings.h"

#include "libmaildir/maildir.h"

#include <KConfigDialogManager>
#include <KLineEdit>
using KPIM::Maildir;

ConfigWidget::ConfigWidget(Settings *settings, QWidget *parent)
    : QWidget(parent)
    , mManager(new KConfigDialogManager(this, settings))
{
    ui.setupUi(this);
    connect(ui.kcfg_Path->lineEdit(), &QLineEdit::textChanged, this, &ConfigWidget::checkPath);
    ui.kcfg_Path->lineEdit()->setFocus();
}

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
        if (!md.isValid()) {
            Maildir md2(d.path(), true);
            if (md2.isValid()) {
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
            ok = true;
        } else {
            ui.statusLabel->setText(i18n("The selected path does not exist."));
        }
    }
    Q_EMIT okEnabled(ok);
}

void ConfigWidget::load(Settings *settings)
{
    mManager->updateWidgets();
    ui.kcfg_Path->setMode(KFile::Directory | KFile::ExistingOnly);
    ui.kcfg_Path->setUrl(QUrl::fromLocalFile(settings->path()));
    checkPath();
}

void ConfigWidget::save(Settings *settings) const
{
    mManager->updateSettings();
    settings->setPath(ui.kcfg_Path->url().isLocalFile() ? ui.kcfg_Path->url().toLocalFile() : ui.kcfg_Path->url().path());
    settings->setTopLevelIsContainer(mToplevelIsContainer);
}
