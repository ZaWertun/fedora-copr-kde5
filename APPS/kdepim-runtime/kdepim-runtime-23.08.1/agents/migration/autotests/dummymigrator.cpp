/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "dummymigrator.h"
#include <QDebug>
#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

DummyMigrator::DummyMigrator(const QString &identifier)
    : MigratorBase(QLatin1String("dummymigrator") + identifier, QString(), QString())
{
}

QString DummyMigrator::displayName() const
{
    return QStringLiteral("dummymigrator");
}

void DummyMigrator::startWork()
{
    qDebug();
    QTimer::singleShot(10s, this, &DummyMigrator::onTimerElapsed);
}

void DummyMigrator::onTimerElapsed()
{
    qDebug();
    setMigrationState(Complete);
}

bool DummyMigrator::shouldAutostart() const
{
    return true;
}

bool DummyMigrator::canStart()
{
    return true;
}

void DummyMigrator::pause()
{
    qDebug();
    MigratorBase::pause();
}

void DummyMigrator::abort()
{
    qDebug();
    MigratorBase::abort();
}
