/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <migration/migratorbase.h>

/**
 * Dummy migrator that simply completes after 10s and always autostarts.
 * Add to the scheduler to play with the migrationagent.
 */
class DummyMigrator : public MigratorBase
{
    Q_OBJECT
public:
    explicit DummyMigrator(const QString &identifier);

    QString displayName() const override;
    void startWork() override;

    bool shouldAutostart() const override;
    bool canStart() override;
    void pause() override;

    void abort() override;
private Q_SLOTS:
    void onTimerElapsed();
};
