/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "migrationscheduler.h"

#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <QDebug>
#include <QIcon>

#include "migrationexecutor.h"

void LogModel::message(MigratorBase::MessageType type, const QString &msg)
{
    switch (type) {
    case MigratorBase::Success: {
        auto item = new QStandardItem(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), msg);
        item->setEditable(false);
        appendRow(item);
        break;
    }
    case MigratorBase::Skip: {
        auto item = new QStandardItem(QIcon::fromTheme(QStringLiteral("dialog-ok")), msg);
        item->setEditable(false);
        appendRow(item);
        break;
    }
    case MigratorBase::Info: {
        auto item = new QStandardItem(QIcon::fromTheme(QStringLiteral("dialog-information")), msg);
        item->setEditable(false);
        appendRow(item);
        break;
    }
    case MigratorBase::Warning: {
        auto item = new QStandardItem(QIcon::fromTheme(QStringLiteral("dialog-warning")), msg);
        item->setEditable(false);
        appendRow(item);
        break;
    }
    case MigratorBase::Error: {
        auto item = new QStandardItem(QIcon::fromTheme(QStringLiteral("dialog-error")), msg);
        item->setEditable(false);
        appendRow(item);
        break;
    }
    default:
        qCritical() << "unknown type " << type;
    }
}

Row::Row(const QSharedPointer<MigratorBase> &migrator, MigratorModel &model)
    : QObject()
    , mMigrator(migrator)
    , mModel(model)
{
    connect(migrator.data(), &MigratorBase::stateChanged, this, &Row::stateChanged);
    connect(migrator.data(), qOverload<int>(&MigratorBase::progress), this, &Row::progress);
}

bool Row::operator==(const Row &other) const
{
    return mMigrator->identifier() == other.mMigrator->identifier();
}

void Row::stateChanged(MigratorBase::MigrationState /*newState*/)
{
    mModel.columnChanged(*this, MigratorModel::State);
}

void Row::progress(int /*prog*/)
{
    mModel.columnChanged(*this, MigratorModel::Progress);
}

int MigratorModel::positionOf(const Row &row)
{
    int pos = 0;
    for (const QSharedPointer<Row> &r : std::as_const(mMigrators)) {
        if (row == *r) {
            return pos;
        }
        pos++;
    }
    return -1;
}

void MigratorModel::columnChanged(const Row &row, int col)
{
    const int p = positionOf(row);
    Q_ASSERT(p >= 0);
    if (p >= 0) {
        const QModelIndex idx = index(p, col);
        Q_EMIT dataChanged(idx, idx);
    }
}

bool MigratorModel::addMigrator(const QSharedPointer<MigratorBase> &m)
{
    if (migrator(m->identifier())) {
        qWarning() << "Model already contains a migrator with the identifier: " << m;
        return false;
    }
    const int pos = mMigrators.size();
    beginInsertRows(QModelIndex(), pos, pos);
    mMigrators.append(QSharedPointer<Row>(new Row(m, *this)));
    endInsertRows();
    return true;
}

int MigratorModel::columnCount(const QModelIndex & /*parent*/) const
{
    return ColumnCount;
}

int MigratorModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mMigrators.size();
    }
    return 0;
}

QModelIndex MigratorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= rowCount(parent) || row < 0) {
        return {};
    }
    return createIndex(row, column, static_cast<void *>(mMigrators.at(row).data()));
}

QModelIndex MigratorModel::parent(const QModelIndex & /*child*/) const
{
    return {};
}

QVariant MigratorModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case Name:
            return i18nc("Name of the migrator in this row", "Name");
        case Progress:
            return i18nc("Progress of the migrator in %", "Progress");
        case State:
            return i18nc("Current status of the migrator (done, in progress, ...)", "Status");
        default:
            Q_ASSERT(false);
        }
    }
    return {};
}

QVariant MigratorModel::data(const QModelIndex &index, int role) const
{
    const Row *row = static_cast<Row *>(index.internalPointer());
    const QSharedPointer<MigratorBase> migrator(row->mMigrator);
    if (!migrator) {
        qWarning() << "migrator not found";
        return {};
    }
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case Name:
            return migrator->displayName();
        case Progress:
            return QStringLiteral("%1 %").arg(migrator->progress());
        case State:
            return migrator->status();
        default:
            Q_ASSERT(false);
            return {};
        }
    case IdentifierRole:
        return migrator->identifier();
    case LogfileRole:
        return migrator->logfile();
    case Qt::ToolTipRole:
        return migrator->description();
    default:
        break;
    }
    return {};
}

QSharedPointer<MigratorBase> MigratorModel::migrator(const QString &identifier) const
{
    for (const QSharedPointer<Row> &row : std::as_const(mMigrators)) {
        if (row->mMigrator->identifier() == identifier) {
            return row->mMigrator;
        }
    }
    return {};
}

QList<QSharedPointer<MigratorBase>> MigratorModel::migrators() const
{
    QList<QSharedPointer<MigratorBase>> migrators;
    migrators.reserve(mMigrators.count());
    for (const QSharedPointer<Row> &row : std::as_const(mMigrators)) {
        migrators << row->mMigrator;
    }
    return migrators;
}

MigrationScheduler::MigrationScheduler(KJobTrackerInterface *jobTracker, QObject *parent)
    : QObject(parent)
    , mModel(new MigratorModel)
    , mJobTracker(jobTracker)
{
}

MigrationScheduler::~MigrationScheduler()
{
    delete mAutostartExecutor;
}

void MigrationScheduler::addMigrator(const QSharedPointer<MigratorBase> &migrator)
{
    if (mModel->addMigrator(migrator)) {
        QSharedPointer<LogModel> logModel(new LogModel);
        connect(migrator.data(), &MigratorBase::message, logModel.data(), &LogModel::message);
        mLogModel.insert(migrator->identifier(), logModel);
        if (migrator->shouldAutostart()) {
            checkForAutostart(migrator);
        }
    }
}

QAbstractItemModel &MigrationScheduler::model()
{
    return *mModel;
}

QStandardItemModel &MigrationScheduler::logModel(const QString &identifier)
{
    Q_ASSERT(mLogModel.contains(identifier));
    return *mLogModel.value(identifier);
}

void MigrationScheduler::checkForAutostart(const QSharedPointer<MigratorBase> &migrator)
{
    if (migrator->migrationState() != MigratorBase::Complete) {
        if (!mAutostartExecutor) {
            mAutostartExecutor = new MigrationExecutor;
            if (mJobTracker) {
                mJobTracker->registerJob(mAutostartExecutor);
            }

            mAutostartExecutor->start();
        }

        mAutostartExecutor->add(migrator);
    }
}

void MigrationScheduler::start(const QString &identifier)
{
    // TODO create separate executor?
    const QSharedPointer<MigratorBase> m = mModel->migrator(identifier);
    if (m) {
        m->start();
    }
}

void MigrationScheduler::pause(const QString &identifier)
{
    const QSharedPointer<MigratorBase> m = mModel->migrator(identifier);
    if (m) {
        m->pause();
    }
}

void MigrationScheduler::abort(const QString &identifier)
{
    const QSharedPointer<MigratorBase> m = mModel->migrator(identifier);
    if (m) {
        m->abort();
    }
}
