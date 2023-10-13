/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "migrationscheduler.h"
class QItemSelectionModel;
#include <QWidget>

class MigrationStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MigrationStatusWidget(MigrationScheduler &scheduler, QWidget *parent = nullptr);

public Q_SLOTS:
    void onItemActivated(const QModelIndex &);

private:
    void startSelected();
    void pauseSelected();
    void abortSelected();
    MigrationScheduler &mScheduler;
    QItemSelectionModel *mSelectionModel = nullptr;
};
