/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lockmethodpage.h"
#include "settings.h"

#include <QStandardPaths>

LockMethodPage::LockMethodPage(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    checkAvailableLockMethods();
}

void LockMethodPage::checkAvailableLockMethods()
{
    // FIXME: I guess this whole checking makes only sense on linux machines.

    // Check for procmail lock method.
    if (QStandardPaths::findExecutable(QStringLiteral("lockfile")).isEmpty()) {
        ui.procmail->setEnabled(false);
        if (ui.procmail->isChecked()) { // Select another lock method if necessary
            ui.mutt_dotlock->setChecked(true);
        }
    }

    // Check for mutt lock method.
    if (QStandardPaths::findExecutable(QStringLiteral("mutt_dotlock")).isEmpty()) {
        ui.mutt_dotlock->setEnabled(false);
        ui.mutt_dotlock_privileged->setEnabled(false);
        if (ui.mutt_dotlock->isChecked() || ui.mutt_dotlock_privileged->isChecked()) {
            if (ui.procmail->isEnabled()) {
                ui.procmail->setChecked(true);
            } else {
                ui.none->setChecked(true);
            }
        }
    }
}
