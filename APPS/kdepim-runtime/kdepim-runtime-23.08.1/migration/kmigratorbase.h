/*
    SPDX-FileCopyrightText: 2009 Jonathan Armond <jon.armond@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/AgentInstance>
#include <QEventLoopLocker>
#include <QObject>

class QFile;
class KJob;

/**
 * Base class for akonadi resource migrators.
 */
class KMigratorBase : public QObject
{
    Q_OBJECT
public:
    enum MigrationState {
        None,
        Bridged,
        Complete,
    };

    enum MessageType {
        Success,
        Skip,
        Info,
        Warning,
        Error,
    };

    Q_ENUM(MigrationState)

    KMigratorBase();
    ~KMigratorBase() override;

    /**
     * Read resource migration state.
     *
     * @return MigrationState and None if the resource with @param identifier as identifier is not available.
     */
    MigrationState migrationState(const QString &identifier) const;
    /**
     * Set resource migration state.
     *
     * Persists migration state in the resource config.
     * @param resId and @param state is registered under @param identifier.
     * Additionally all bridged resources are registered in the @param type and @param identifier.
     */
    void setMigrationState(const QString &identifier, MigrationState state, const QString &resId, const QString &type);

    virtual void migrateNext() = 0;

protected:
    KJob *createAgentInstance(const QString &typeId, QObject *receiver, const char *slot);
    virtual void migrationFailed(const QString &errorMsg, const Akonadi::AgentInstance &instance = Akonadi::AgentInstance()) = 0;

Q_SIGNALS:
    void message(KMigratorBase::MessageType type, const QString &msg);

protected Q_SLOTS:
    virtual void migrate() = 0;

private:
    void logMessage(KMigratorBase::MessageType type, const QString &msg);
    QFile *m_logFile = nullptr;
    QEventLoopLocker eventLoopLocker;
};
