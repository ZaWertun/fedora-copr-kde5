/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "migratorbase.h"
#include "migration_debug.h"
#include <Akonadi/ServerManager>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

static QString messageTypeToString(MigratorBase::MessageType type)
{
    switch (type) {
    case MigratorBase::Success:
        return QStringLiteral("Success");
    case MigratorBase::Skip:
        return QStringLiteral("Skipped");
    case MigratorBase::Info:
        return QStringLiteral("Info   ");
    case MigratorBase::Warning:
        return QStringLiteral("WARNING");
    case MigratorBase::Error:
        return QStringLiteral("ERROR  ");
    }
    Q_ASSERT(false);
    return {};
}

static QMap<QString, MigratorBase::MigrationState> fillMigrationStateMapping()
{
    QMap<QString, MigratorBase::MigrationState> map;
    map.insert(QStringLiteral("Complete"), MigratorBase::Complete);
    map.insert(QStringLiteral("Aborted"), MigratorBase::Aborted);
    map.insert(QStringLiteral("InProgress"), MigratorBase::InProgress);
    map.insert(QStringLiteral("Failed"), MigratorBase::Failed);
    return map;
}

static QMap<QString, MigratorBase::MigrationState> migrationStateMapping = fillMigrationStateMapping();

static QString stateToIdentifier(MigratorBase::MigrationState state)
{
    Q_ASSERT(migrationStateMapping.values().contains(state));
    return migrationStateMapping.key(state);
}

static MigratorBase::MigrationState identifierToState(const QString &identifier)
{
    Q_ASSERT(migrationStateMapping.contains(identifier));
    return migrationStateMapping.value(identifier);
}

MigratorBase::MigratorBase(const QString &identifier, QObject *parent)
    : QObject(parent)
    , mIdentifier(identifier)
    , mConfig(new KConfig(Akonadi::ServerManager::addNamespace(QStringLiteral("akonadi-migrationrc"))))
{
    const QString logFileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QCoreApplication::applicationName()
        + QLatin1Char('/') + identifier + QStringLiteral("migration.log");
    QFileInfo fileInfo(logFileName);
    QDir().mkpath(fileInfo.absolutePath());
    setLogfile(logFileName);
    connect(this, &MigratorBase::message, this, &MigratorBase::logMessage);
    loadState();
}

MigratorBase::MigratorBase(const QString &identifier, const QString &configFile, const QString &logFile, QObject *parent)
    : QObject(parent)
    , mIdentifier(identifier)
    , mMigrationState(None)
{
    if (!configFile.isEmpty()) {
        mConfig.reset(new KConfig(configFile));
    }
    setLogfile(logFile);
    connect(this, &MigratorBase::message, this, &MigratorBase::logMessage);
    loadState();
}

MigratorBase::~MigratorBase() = default;

void MigratorBase::setLogfile(const QString &logfile)
{
    if (!logfile.isEmpty()) {
        mLogFile.reset(new QFile(logfile));
        if (!mLogFile->open(QFile::Append)) {
            mLogFile.reset();
            qCWarning(MIGRATION_LOG) << "Unable to open log file: " << logfile;
        }
    } else {
        mLogFile.reset();
    }
}

QString MigratorBase::identifier() const
{
    return mIdentifier;
}

QString MigratorBase::displayName() const
{
    return {};
}

QString MigratorBase::description() const
{
    return {};
}

QString MigratorBase::logfile() const
{
    if (mLogFile) {
        return mLogFile->fileName();
    }
    return {};
}

bool MigratorBase::canStart()
{
    if (mIdentifier.isEmpty()) {
        Q_EMIT message(Error, i18n("Missing Identifier"));
        return false;
    }
    return true;
}

void MigratorBase::start()
{
    if (mMigrationState == InProgress) {
        qCWarning(MIGRATION_LOG) << "already running";
        return;
    }
    if (!canStart()) {
        Q_EMIT message(Error, i18n("Failed to start migration because migrator is not ready"));
        Q_EMIT stoppedProcessing();
        return;
    }
    // TODO acquire dbus lock
    logMessage(Info, displayName());
    Q_EMIT message(Info, i18n("Starting migration..."));
    setMigrationState(InProgress);
    setProgress(0);
    startWork();
}

void MigratorBase::pause()
{
    qCWarning(MIGRATION_LOG) << "pause is not implemented";
}

void MigratorBase::resume()
{
    qCWarning(MIGRATION_LOG) << "resume is not implemented";
}

void MigratorBase::abort()
{
    qCWarning(MIGRATION_LOG) << "abort is not implemented";
}

void MigratorBase::logMessage(MigratorBase::MessageType type, const QString &msg)
{
    if (mLogFile) {
        mLogFile->write(QString(QLatin1Char('[') + QDateTime::currentDateTime().toString() + QStringLiteral("] ") + messageTypeToString(type)
                                + QStringLiteral(": ") + msg + QLatin1Char('\n'))
                            .toUtf8());
        mLogFile->flush();
    }
}

bool MigratorBase::shouldAutostart() const
{
    return false;
}

void MigratorBase::setMigrationState(MigratorBase::MigrationState state)
{
    mMigrationState = state;
    switch (state) {
    case Complete:
        setProgress(100);
        Q_EMIT message(Success, i18n("Migration complete"));
        Q_EMIT stoppedProcessing();
        break;
    case Aborted:
        Q_EMIT message(Skip, i18n("Migration aborted"));
        Q_EMIT stoppedProcessing();
        break;
    case InProgress:
        break;
    case Failed:
        Q_EMIT message(Error, i18n("Migration failed"));
        Q_EMIT stoppedProcessing();
        break;
    case Paused:
        Q_EMIT message(Info, i18n("Migration paused"));
        Q_EMIT stateChanged(mMigrationState);
        return;
    default:
        qCWarning(MIGRATION_LOG) << "invalid state " << state;
        Q_ASSERT(false);
        return;
    }
    saveState();
    Q_EMIT stateChanged(mMigrationState);
}

MigratorBase::MigrationState MigratorBase::migrationState() const
{
    return mMigrationState;
}

void MigratorBase::saveState()
{
    config().writeEntry(QStringLiteral("MigrationState"), stateToIdentifier(mMigrationState));
}

void MigratorBase::loadState()
{
    const QString state = config().readEntry(QStringLiteral("MigrationState"), QString());
    if (!state.isEmpty()) {
        mMigrationState = identifierToState(state);
    }

    if (mMigrationState == InProgress) {
        Q_EMIT message(Warning, i18n("This migration has already been started once but was aborted"));
        mMigrationState = NeedsUpdate;
    }
    switch (mMigrationState) {
    case Complete:
        mProgress = 100;
        break;
    default:
        mProgress = 0;
    }
}

NullableConfigGroup MigratorBase::config()
{
    if (mConfig) {
        return NullableConfigGroup(mConfig->group(mIdentifier));
    }
    return {};
}

int MigratorBase::progress() const
{
    return mProgress;
}

void MigratorBase::setProgress(int prog)
{
    if (mProgress != prog) {
        mProgress = prog;
        Q_EMIT progress(prog);
    }
}

QString MigratorBase::status() const
{
    switch (mMigrationState) {
    case None:
        return i18nc("@info:status", "Not started");
    case InProgress:
        return i18nc("@info:status", "Running...");
    case Complete:
        return i18nc("@info:status", "Complete");
    case Aborted:
        return i18nc("@info:status", "Aborted");
    case Paused:
        return i18nc("@info:status", "Paused");
    case NeedsUpdate:
        return i18nc("@info:status", "Needs Update");
    case Failed:
        return i18nc("@info:status", "Failed");
    }
    return {};
}
