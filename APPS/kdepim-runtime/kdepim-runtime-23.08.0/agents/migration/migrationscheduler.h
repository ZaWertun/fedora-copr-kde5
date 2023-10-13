/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "migratorbase.h"
#include <QAbstractItemModel>
class QObject;
#include <QPointer>
#include <QSharedPointer>
#include <QStandardItemModel>

class MigrationExecutor;
class KJobTrackerInterface;
class MigratorModel;

class LogModel : public QStandardItemModel
{
    Q_OBJECT
public Q_SLOTS:
    void message(MigratorBase::MessageType type, const QString &msg);
};

class Row : public QObject
{
    Q_OBJECT
public:
    QSharedPointer<MigratorBase> mMigrator;
    MigratorModel &mModel;

    explicit Row(const QSharedPointer<MigratorBase> &migrator, MigratorModel &model);

    bool operator==(const Row &other) const;

private:
    void stateChanged(MigratorBase::MigrationState);
    void progress(int);
};

/**
 * The model serves as container for the migrators and exposes the status of each migrator.
 *
 * It can be plugged into a Listview to inform about the migration progress.
 */
class MigratorModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles {
        IdentifierRole = Qt::UserRole + 1,
        LogfileRole,
    };
    bool addMigrator(const QSharedPointer<MigratorBase> &migrator);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QSharedPointer<MigratorBase> migrator(const QString &identifier) const;
    QList<QSharedPointer<MigratorBase>> migrators() const;

private:
    enum Columns {
        Name = 0,
        Progress = 1,
        State = 2,
        ColumnCount,
    };
    friend class Row;
    int positionOf(const Row &);
    void columnChanged(const Row &, int column);
    QList<QSharedPointer<Row>> mMigrators;
};

/**
 * Scheduler for migration jobs.
 *
 * Status information is exposed via getModel, which returns a list model containing all migrators with basic information.
 * Additionally a logmodel is available via getLogModel for each migrator. The logmodel is continuously filled with information, and can be requested and
 * displayed at any time.
 *
 * Migrators which return true on shouldAutostart() automatically enter a queue to be processed one after the other.
 * When manually triggered it is possible though to run multiple jobs in parallel.
 */
class MigrationScheduler : public QObject
{
    Q_OBJECT
public:
    explicit MigrationScheduler(KJobTrackerInterface *jobTracker = nullptr, QObject *parent = nullptr);
    ~MigrationScheduler() override;

    void addMigrator(const QSharedPointer<MigratorBase> &migrator);

    // A model for the view
    QAbstractItemModel &model();
    QStandardItemModel &logModel(const QString &identifier);

    // Control
    void start(const QString &identifier);
    void pause(const QString &identifier);
    void abort(const QString &identifier);

private:
    void checkForAutostart(const QSharedPointer<MigratorBase> &migrator);

    QScopedPointer<MigratorModel> mModel;
    QHash<QString, QSharedPointer<LogModel>> mLogModel;
    QPointer<MigrationExecutor> mAutostartExecutor;
    KJobTrackerInterface *mJobTracker = nullptr;
};
