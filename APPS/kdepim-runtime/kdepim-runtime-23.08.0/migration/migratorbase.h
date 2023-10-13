/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <KConfig>
#include <KConfigGroup>
#include <QFile>
#include <QObject>

class NullableConfigGroup
{
public:
    NullableConfigGroup() = default;

    explicit NullableConfigGroup(const KConfigGroup &grp)
        : mConfigGroup(grp)
    {
    }

    KConfigGroup &configGroup()
    {
        return mConfigGroup;
    }

    template<typename T>
    inline T readEntry(const QString &key, const T &aDefault) const
    {
        if (mConfigGroup.isValid()) {
            return mConfigGroup.readEntry<T>(key, aDefault);
        }
        return aDefault;
    }

    template<typename T>
    inline void writeEntry(const QString &key, const T &value)
    {
        if (mConfigGroup.isValid()) {
            mConfigGroup.writeEntry<T>(key, value);
        }
    }

private:
    KConfigGroup mConfigGroup;
};

/**
 * Base class for generic migration jobs in akonadi.
 *
 * MigrationJobs can be run standalone from commandline using a small wrapper application or using the
 * Akonadi Migration Agent.
 *
 * Each migrator should assign a unique identifier for it's state (this identifier must never change).
 *
 * The work done by the migrator may be paused, and the migrator may persist it's state to resume migrations after a reboot.
 *
 * TODO: The migrator base ensures that no migrator can be run multiple times by locking it over dbus.
 *
 * The status is stored in the akonadi instance config directory, meaning the status is stored per akonadi instance.
 * This is the only reason why this MigratorBase is currently specific to akonadi migration jobs.
 */
class MigratorBase : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor with default config and logfile
     */
    explicit MigratorBase(const QString &identifier, QObject *parent = nullptr);

    /**
     * Constructor that allows to inject a configfile and logfile.
     *
     * Pass and empty string to disable config and log.
     */
    explicit MigratorBase(const QString &identifier, const QString &configFile, const QString &logFile, QObject *parent = nullptr);

    ~MigratorBase() override;

    QString identifier() const;

    /**
     * Translated, human readable display name of migrator.
     */
    virtual QString displayName() const;

    /**
     * Translated, human readable description of migrator.
     */
    virtual QString description() const;

    /**
     * Returns the filename of the logfile used by this migrator.
     *
     * Returns QString() if there is no logfile set.
     */
    QString logfile() const;

    enum MigrationState {
        None,
        InProgress,
        Paused,
        Complete,
        NeedsUpdate,
        Aborted,
        Failed,
    };

    enum MessageType {
        Success,
        Skip,
        Info,
        Warning,
        Error,
    };

    /**
     * Read migration state.
     *
     * @return MigrationState.
     */
    MigrationState migrationState() const;

    /**
     * Return false if this job cannot start (i.e. due to missing dependencies).
     */
    virtual bool canStart();

    /**
     * Mandatory updates that the Migration Agent should autostart should return true
     */
    virtual bool shouldAutostart() const;

    /**
     * Start migration.
     *
     * Implement startWork instead.
     *
     * Note that this will directly (blocking) call startWork().
     */
    void start();

    /**
     * Pause migration.
     */
    virtual void pause();

    /**
     * Resume migration.
     */
    virtual void resume();

    /**
     * Abort migration.
     */
    virtual void abort();

    /**
     * progress in percent
     */
    int progress() const;

    /**
     * Status
     */
    QString status() const;

Q_SIGNALS:
    // Signal for state changes
    void stateChanged(MigratorBase::MigrationState);

    // Signal for log window
    void message(MigratorBase::MessageType type, const QString &msg);

    // Signal for progress bar
    void progress(int progress);

    // Signal for scheduling. The migrator has finished for some reason (success, failure, ...) and we can forget about it and move on.
    void stoppedProcessing();

protected:
    /**
     * Reimplement to start work.
     */
    virtual void startWork() = 0;

    void setMigrationState(MigratorBase::MigrationState state);

    void setProgress(int);

private Q_SLOTS:
    /**
     * Logs a message, that appears in the logfile and potentially in a log window.
     * Do not call this directly. Emit the message signal instead, which is connected to this slot.
     */
    void logMessage(MigratorBase::MessageType type, const QString &msg);

private:
    NullableConfigGroup config();
    void saveState();
    void loadState();

    void setLogfile(const QString &);

    const QString mIdentifier;
    MigrationState mMigrationState = None;
    QScopedPointer<QFile> mLogFile;
    QScopedPointer<KConfig> mConfig;
    int mProgress;
};

Q_DECLARE_METATYPE(MigratorBase::MigrationState)
