/*
    SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gidmigrator.h"

#include "gidmigrationjob.h"
#include <KLocalizedString>

GidMigrator::GidMigrator(const QString &mimeType)
    : MigratorBase(QLatin1String("gidmigrator") + mimeType)
    , mMimeType(mimeType)
{
}

GidMigrator::~GidMigrator() = default;

QString GidMigrator::displayName() const
{
    return i18nc("Name of the GID Migrator (intended for advanced users).", "GID Migrator: %1", mMimeType);
}

QString GidMigrator::description() const
{
    return i18n("Ensures that all items with the mimetype %1 have a GID if a GID extractor is available.", mMimeType);
}

bool GidMigrator::canStart()
{
    return MigratorBase::canStart();
}

bool GidMigrator::shouldAutostart() const
{
    return true;
}

void GidMigrator::startWork()
{
    auto job = new GidMigrationJob(QStringList() << mMimeType, this);
    connect(job, &GidMigrationJob::result, this, &GidMigrator::migrationFinished);
}

void GidMigrator::migrationFinished(KJob *job)
{
    if (job->error()) {
        Q_EMIT message(Error, i18n("Migration failed: %1", job->errorString()));
        setMigrationState(Failed);
    } else {
        setMigrationState(Complete);
    }
}
