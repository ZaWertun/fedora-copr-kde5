/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QObject>
#include <QTimer>

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Monitor>

class StateMonitorBase : public QObject
{
    Q_OBJECT
public:
    explicit StateMonitorBase(QObject *parent)
        : QObject(parent)
    {
    }

    ~StateMonitorBase() override = default;
Q_SIGNALS:
    void stateReached();
    void errorOccurred();
};

template<typename T>
class CollectionStateMonitor : public StateMonitorBase
{
public:
    using StateComparisonFunc = std::function<bool(const Akonadi::Collection &, const T &)>;
    CollectionStateMonitor(QObject *parent,
                           const QHash<QString, T> &stateHash,
                           const QString &inboxId,
                           const StateComparisonFunc &comparisonFunc,
                           int recheckInterval = 0);
    ~CollectionStateMonitor() override = default;
    Akonadi::Monitor &monitor()
    {
        return mMonitor;
    }

    void forceRecheck();

private:
    void stateChanged(const Akonadi::Collection &col);

    Akonadi::Monitor mMonitor;
    QSet<QString> mPending;
    const QHash<QString, T> &mStateHash;
    StateComparisonFunc mComparisonFunc;
    const QString &mInboxId;
    QTimer mRecheckTimer;
};

template<typename T>
CollectionStateMonitor<T>::CollectionStateMonitor(QObject *parent,
                                                  const QHash<QString, T> &stateHash,
                                                  const QString &inboxId,
                                                  const StateComparisonFunc &comparisonFunc,
                                                  int recheckInterval)
    : StateMonitorBase(parent)
    , mMonitor(this)
    , mPending(stateHash.keyBegin(), stateHash.keyEnd())
    , mStateHash(stateHash)
    , mComparisonFunc(comparisonFunc)
    , mInboxId(inboxId)
    , mRecheckTimer(this)
{
    connect(&mMonitor, &Akonadi::Monitor::collectionAdded, this, [this](const Akonadi::Collection &col, const Akonadi::Collection &) {
        stateChanged(col);
    });
    connect(&mMonitor, qOverload<const Akonadi::Collection &>(&Akonadi::Monitor::collectionChanged), this, [this](const Akonadi::Collection &col) {
        stateChanged(col);
    });
    if (recheckInterval > 0) {
        mRecheckTimer.setInterval(recheckInterval);
        connect(&mRecheckTimer, &QTimer::timeout, this, &CollectionStateMonitor::forceRecheck);
        mRecheckTimer.start();
    }
}

template<typename T>
void CollectionStateMonitor<T>::stateChanged(const Akonadi::Collection &col)
{
    auto remoteId = col.remoteId();
    auto state = mStateHash.find(remoteId);
    if (state == mStateHash.end()) {
        qDebug() << "Cannot find state for collection" << remoteId;
        Q_EMIT errorOccurred();
    }
    if (mComparisonFunc(col, *state)) {
        mPending.remove(remoteId);
    } else {
        mPending.insert(remoteId);
    }
    if (mPending.empty()) {
        Q_EMIT stateReached();
    }
}

template<typename T>
void CollectionStateMonitor<T>::forceRecheck()
{
    auto fetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this);
    fetchJob->setFetchScope(mMonitor.collectionFetchScope());
    if (fetchJob->exec()) {
        const auto collections = fetchJob->collections();
        for (const auto &col : collections) {
            const auto remoteId = col.remoteId();
            const auto state = mStateHash.find(remoteId);
            if (state != mStateHash.end()) {
                stateChanged(col);
            }
        }
    }
}
