/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include "ui_lockfilepage.h"

class LockMethodPage : public QWidget
{
    Q_OBJECT
public:
    explicit LockMethodPage(QWidget *parent = nullptr);

private:
    void checkAvailableLockMethods();

private:
    Ui::LockFilePage ui;
};
