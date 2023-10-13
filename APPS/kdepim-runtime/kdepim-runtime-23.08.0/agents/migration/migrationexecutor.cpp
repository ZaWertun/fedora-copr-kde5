/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "migrationexecutor.h"

#include <KLocalizedString>

MigrationExecutor::MigrationExecutor()
    : KJob()
{
    setCapabilities(Suspendable);
}

void MigrationExecutor::start()
{
    setPercent(0);
    Q_EMIT description(this, i18nc("User visible name of ongoing Akonadi migration jobs", "PIM Maintenance"));
}

void MigrationExecutor::add(const QSharedPointer<MigratorBase> &migrator)
{
    mTotalAmount++;
    mQueue.enqueue(migrator.toWeakRef());
    executeNext();
}

void MigrationExecutor::executeNext()
{
    if (mCurrentMigrator || mSuspended) {
        return;
    }
    QSharedPointer<MigratorBase> migrator;
    while (!migrator && !mQueue.isEmpty()) {
        mCurrentMigrator = mQueue.dequeue();
        migrator = mCurrentMigrator.toStrongRef();
    }
    if (migrator) {
        Q_EMIT infoMessage(this, i18nc("PIM-Maintenance is in progress.", "In progress..."));
        connect(migrator.data(), &MigratorBase::stoppedProcessing, this, &MigrationExecutor::onStoppedProcessing);
        migrator->start();
    } else {
        // Reset the notification status, otherwise we get notification "In progress...[finished]"
        // without any description that it's related to PIM-Maintenance
        Q_EMIT infoMessage(this, i18n("PIM Maintenance"));
        emitResult();
    }
}

void MigrationExecutor::onStoppedProcessing()
{
    mAlreadyProcessed++;
    Q_ASSERT(mTotalAmount > 0);
    // TODO: setProcessedAmount would be better, but we need support for suitable units first (there's only files, folders, bytes).
    setPercent(mAlreadyProcessed * 100.0 / mTotalAmount);
    mCurrentMigrator.clear();
    executeNext();
}

bool MigrationExecutor::doSuspend()
{
    if (mCurrentMigrator) {
        QSharedPointer<MigratorBase> migrator = mCurrentMigrator.toStrongRef();
        if (migrator) {
            migrator->pause();
        } else {
            mCurrentMigrator.clear();
        }
    }
    Q_EMIT infoMessage(this, i18nc("PIM-Maintenance is paused.", "Paused."));
    mSuspended = true;
    return true;
}

bool MigrationExecutor::doResume()
{
    mSuspended = false;
    if (mCurrentMigrator) {
        QSharedPointer<MigratorBase> migrator = mCurrentMigrator.toStrongRef();
        if (migrator) {
            migrator->resume();
        } else {
            mCurrentMigrator.clear();
        }
    }
    executeNext();
    return true;
}
