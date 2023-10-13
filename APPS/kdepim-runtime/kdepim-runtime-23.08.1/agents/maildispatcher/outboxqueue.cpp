/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outboxqueue.h"

#include <QTimer>

#include "maildispatcher_debug.h"
#include <KLocalizedString>

#include <Akonadi/AddressAttribute>
#include <Akonadi/Attribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageFlags>
#include <Akonadi/Monitor>
#include <Akonadi/SpecialMailCollections>
#include <Akonadi/SpecialMailCollectionsRequestJob>

#include <KMime/Message>

#include <Akonadi/DispatchModeAttribute>
#include <Akonadi/SentBehaviourAttribute>
#include <Akonadi/TransportAttribute>
#include <MailTransport/TransportManager>
#include <chrono>

using namespace std::chrono_literals;

using namespace Akonadi;
using namespace MailTransport;

static const int OUTBOX_DISCOVERY_RETRIES = 3; // number of times we try to find or create the outbox
static const int OUTBOX_DISCOVERY_WAIT_TIME = 5000; // number of ms to wait before retrying

void OutboxQueue::initQueue()
{
    mTotalSize = 0;
    mQueue.clear();

    qCDebug(MAILDISPATCHER_LOG) << "Fetching items in collection" << mOutbox.id();
    auto job = new ItemFetchJob(mOutbox);
    job->fetchScope().fetchAllAttributes();
    job->fetchScope().fetchFullPayload(false);
    connect(job, &ItemFetchJob::result, this, &OutboxQueue::collectionFetched);
}

void OutboxQueue::addIfComplete(const Item &item)
{
    if (mIgnore.contains(item.id())) {
        qCDebug(MAILDISPATCHER_LOG) << "Item" << item.id() << "is ignored.";
        return;
    }

    if (mQueue.contains(item)) {
        qCDebug(MAILDISPATCHER_LOG) << "Item" << item.id() << "already in queue!";
        return;
    }

    if (!item.hasAttribute<AddressAttribute>()) {
        qCWarning(MAILDISPATCHER_LOG) << "Item " << item.id() << "does not have the required attribute Address.";
        return;
    }

    if (!item.hasAttribute<DispatchModeAttribute>()) {
        qCWarning(MAILDISPATCHER_LOG) << "Item " << item.id() << "does not have the required attribute DispatchMode.";
        return;
    }

    if (!item.hasAttribute<SentBehaviourAttribute>()) {
        qCWarning(MAILDISPATCHER_LOG) << "Item " << item.id() << "does not have the required attribute SentBehaviour.";
        return;
    }

    if (!item.hasAttribute<TransportAttribute>()) {
        qCWarning(MAILDISPATCHER_LOG) << "Item" << item.id() << "does not have the required attribute Transport.";
        return;
    }

    if (!item.hasFlag(Akonadi::MessageFlags::Queued)) {
        qCDebug(MAILDISPATCHER_LOG) << "Item " << item.id() << "has no '$QUEUED' flag.";
        return;
    }

    const auto dispatchModeAttribute = item.attribute<DispatchModeAttribute>();
    Q_ASSERT(dispatchModeAttribute);
    if (dispatchModeAttribute->dispatchMode() == DispatchModeAttribute::Manual) {
        qCDebug(MAILDISPATCHER_LOG) << "Item " << item.id() << "is queued to be sent manually.";
        return;
    }

    const auto transportAttribute = item.attribute<TransportAttribute>();
    Q_ASSERT(transportAttribute);
    if (TransportManager::self()->transportById(transportAttribute->transportId(), false) == nullptr) {
        qCWarning(MAILDISPATCHER_LOG) << "Item " << item.id() << "has invalid transport.";
        return;
    }

    const auto sentBehaviourAttribute = item.attribute<SentBehaviourAttribute>();
    Q_ASSERT(sentBehaviourAttribute);
    if (sentBehaviourAttribute->sentBehaviour() == SentBehaviourAttribute::MoveToCollection && !sentBehaviourAttribute->moveToCollection().isValid()) {
        qCWarning(MAILDISPATCHER_LOG) << "Item " << item.id() << "has invalid sent-mail collection.";
        return;
    }

    // This check requires fetchFullPayload. -> slow (?)
    /*
    if ( !item.hasPayload<KMime::Message::Ptr>() ) {
      qCWarning(MAILDISPATCHER_LOG) << "Item" << item.id() << "does not have KMime::Message::Ptr payload.";
      return;
    }
    */

    if (dispatchModeAttribute->dispatchMode() == DispatchModeAttribute::Automatic && dispatchModeAttribute->sendAfter().isValid()
        && dispatchModeAttribute->sendAfter() > QDateTime::currentDateTime()) {
        // All the above was OK, so accept it for the future.
        qCDebug(MAILDISPATCHER_LOG) << "Item" << item.id() << "is accepted to be sent in the future.";
        mFutureMap.insert(dispatchModeAttribute->sendAfter(), item);
        Q_ASSERT(!mFutureItems.contains(item));
        mFutureItems.insert(item);
        checkFuture();
        return;
    }

    qCDebug(MAILDISPATCHER_LOG) << "Item " << item.id() << "is accepted into the queue (size" << item.size() << ").";
    Q_ASSERT(!mQueue.contains(item));
    mTotalSize += item.size();
    mQueue.append(item);
    Q_EMIT newItems();
}

void OutboxQueue::checkFuture()
{
    qCDebug(MAILDISPATCHER_LOG) << "The future is here." << mFutureMap.count() << "items in futureMap.";
    Q_ASSERT(mFutureTimer);
    mFutureTimer->stop();
    // By default, re-check in one hour.
    mFutureTimer->setInterval(1h);

    // Check items in ascending order of date.
    while (!mFutureMap.isEmpty()) {
        QMultiMap<QDateTime, Item>::iterator it = mFutureMap.begin();
        qCDebug(MAILDISPATCHER_LOG) << "Item with due date" << it.key();
        if (it.key() > QDateTime::currentDateTime()) {
            const int secs = QDateTime::currentDateTime().secsTo(it.key()) + 1;
            qCDebug(MAILDISPATCHER_LOG) << "Future, in" << secs << "seconds.";
            Q_ASSERT(secs >= 0);
            if (secs < 60 * 60) {
                mFutureTimer->setInterval(secs * 1000);
            }
            break; // all others are in the future too
        }
        if (!mFutureItems.contains(it.value())) {
            qCDebug(MAILDISPATCHER_LOG) << "Item disappeared.";
        } else {
            qCDebug(MAILDISPATCHER_LOG) << "Due date is here. Queuing.";
            addIfComplete(it.value());
            mFutureItems.remove(it.value());
        }
        mFutureMap.erase(it);
    }

    qCDebug(MAILDISPATCHER_LOG) << "Timer set to checkFuture again in" << mFutureTimer->interval() / 1000 << "seconds"
                                << "(that is" << mFutureTimer->interval() / 1000 / 60 << "minutes).";

    mFutureTimer->start();
}

void OutboxQueue::collectionFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(MAILDISPATCHER_LOG) << "Failed to fetch outbox collection.  Queue will be empty until the outbox changes.";
        return;
    }

    const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob *>(job);
    Q_ASSERT(fetchJob);
    qCDebug(MAILDISPATCHER_LOG) << "Fetched" << fetchJob->items().count() << "items.";

    const Akonadi::Item::List lst = fetchJob->items();
    for (const Item &item : lst) {
        addIfComplete(item);
    }
}

void OutboxQueue::itemFetched(KJob *job)
{
    if (job->error()) {
        qCDebug(MAILDISPATCHER_LOG) << "Error fetching item:" << job->errorString() << ". Trying next item in queue.";
        fetchOne();
    }

    const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob *>(job);
    Q_ASSERT(fetchJob);
    if (fetchJob->items().count() != 1) {
        qCDebug(MAILDISPATCHER_LOG) << "Fetched" << fetchJob->items().count() << ", expected 1. Trying next item in queue.";
        fetchOne();
    }

    if (!fetchJob->items().isEmpty()) {
        Q_EMIT itemReady(fetchJob->items().at(0));
    }
}

void OutboxQueue::localFoldersChanged()
{
    // Called on startup, and whenever the local folders change.

    if (SpecialMailCollections::self()->hasDefaultCollection(SpecialMailCollections::Outbox)) {
        // Outbox is ready, init the queue from it.
        const Collection collection = SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::Outbox);
        Q_ASSERT(collection.isValid());

        if (mOutbox != collection) {
            mMonitor->setCollectionMonitored(mOutbox, false);
            mMonitor->setCollectionMonitored(collection, true);
            mOutbox = collection;
            qCDebug(MAILDISPATCHER_LOG) << "Changed outbox to" << mOutbox.id();
            initQueue();
        }
    } else {
        // Outbox is not ready. Request it, since otherwise we will not know when
        // new messages appear.
        // (Note that we are a separate process, so we get no notification when
        // MessageQueueJob requests the Outbox.)
        mMonitor->setCollectionMonitored(mOutbox, false);
        mOutbox = Collection(-1);

        auto job = new SpecialMailCollectionsRequestJob(this);
        job->requestDefaultCollection(SpecialMailCollections::Outbox);
        connect(job, &SpecialMailCollectionsRequestJob::result, this, &OutboxQueue::localFoldersRequestResult);

        qCDebug(MAILDISPATCHER_LOG) << "Requesting outbox folder.";
        job->start();
    }

    // make sure we have a place to dump the sent mails as well
    if (!SpecialMailCollections::self()->hasDefaultCollection(SpecialMailCollections::SentMail)) {
        auto job = new SpecialMailCollectionsRequestJob(this);
        job->requestDefaultCollection(SpecialMailCollections::SentMail);

        qCDebug(MAILDISPATCHER_LOG) << "Requesting sent-mail folder";
        job->start();
    }
}

void OutboxQueue::localFoldersRequestResult(KJob *job)
{
    if (job->error()) {
        // We tried to create the outbox, but that failed. This could be because some
        // other process, the mail app, for example, tried to create it at the
        // same time. So try again, once or twice, but wait a little in between, longer
        // each time. If we still haven't managed to create it after a few retries,
        // error hard.

        if (++mOutboxDiscoveryRetries <= OUTBOX_DISCOVERY_RETRIES) {
            const int timeout = OUTBOX_DISCOVERY_WAIT_TIME * mOutboxDiscoveryRetries;
            qCWarning(MAILDISPATCHER_LOG) << "Failed to get outbox folder. Retrying in: " << timeout;
            QTimer::singleShot(timeout, this, &OutboxQueue::localFoldersChanged);
        } else {
            qCWarning(MAILDISPATCHER_LOG) << "Failed to get outbox folder. Giving up.";
            Q_EMIT error(i18n("Could not access the outbox folder (%1).", job->errorString()));
        }
        return;
    }

    localFoldersChanged();
}

void OutboxQueue::itemAdded(const Item &item)
{
    addIfComplete(item);
}

void OutboxQueue::itemChanged(const Item &item)
{
    addIfComplete(item);
    // TODO: if the item is moved out of the outbox, will I get itemChanged?
}

void OutboxQueue::itemMoved(const Item &item, const Collection &source, const Collection &destination)
{
    if (source == mOutbox) {
        itemRemoved(item);
    } else if (destination == mOutbox) {
        addIfComplete(item);
    }
}

void OutboxQueue::itemRemoved(const Item &removedItem)
{
    // @p item has size=0, so get the size from our own copy.
    const int index = mQueue.indexOf(removedItem);
    if (index == -1) {
        // Item was not in queue at all.
        return;
    }

    Item item(mQueue.takeAt(index));
    qCDebug(MAILDISPATCHER_LOG) << "Item" << item.id() << "(size" << item.size() << ") was removed from the queue.";
    mTotalSize -= item.size();

    mFutureItems.remove(removedItem);
}

void OutboxQueue::itemProcessed(const Item &item, bool result)
{
    Q_ASSERT(mIgnore.contains(item.id()));
    if (!result) {
        // Give the user a chance to re-send the item if it failed.
        mIgnore.remove(item.id());
    }
}

OutboxQueue::OutboxQueue(QObject *parent)
    : QObject(parent)
{
    mMonitor = new Monitor(this);
    mMonitor->itemFetchScope().fetchAllAttributes();
    mMonitor->itemFetchScope().fetchFullPayload(false);
    connect(mMonitor, &Monitor::itemAdded, this, &OutboxQueue::itemAdded);
    connect(mMonitor, &Monitor::itemChanged, this, &OutboxQueue::itemChanged);
    connect(mMonitor, &Monitor::itemMoved, this, &OutboxQueue::itemMoved);
    connect(mMonitor, &Monitor::itemRemoved, this, &OutboxQueue::itemRemoved);

    connect(SpecialMailCollections::self(), &SpecialMailCollections::defaultCollectionsChanged, this, &OutboxQueue::localFoldersChanged);
    localFoldersChanged();

    mFutureTimer = new QTimer(this);
    connect(mFutureTimer, &QTimer::timeout, this, &OutboxQueue::checkFuture);
    mFutureTimer->start(1h); // 1 hour
}

OutboxQueue::~OutboxQueue() = default;

bool OutboxQueue::isEmpty() const
{
    return mQueue.isEmpty();
}

int OutboxQueue::count() const
{
    const int numberOfElement(mQueue.count());
    if (numberOfElement == 0) {
        // TODO Is this asking for too much?
        Q_ASSERT(mTotalSize == 0);
    }
    return numberOfElement;
}

qulonglong OutboxQueue::totalSize() const
{
    return mTotalSize;
}

void OutboxQueue::fetchOne()
{
    if (isEmpty()) {
        qCDebug(MAILDISPATCHER_LOG) << "Empty queue.";
        return;
    }

    const Item item = mQueue.takeFirst();

    mTotalSize -= item.size();
    Q_ASSERT(!mIgnore.contains(item.id()));
    mIgnore.insert(item.id());

    auto job = new ItemFetchJob(item);
    job->fetchScope().fetchAllAttributes();
    job->fetchScope().fetchFullPayload();
    connect(job, &ItemFetchJob::result, this, &OutboxQueue::itemFetched);
}

#include "moc_outboxqueue.cpp"
