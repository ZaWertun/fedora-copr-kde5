/*
    SPDX-FileCopyrightText: 2011 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrieveitemsjob.h"
#include "maildirresource_debug.h"
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/MessageFlags>
#include <Akonadi/TransactionSequence>
#include <Akonadi/VectorHelper>
#include <KMime/Message>
#include <QDirIterator>

RetrieveItemsJob::RetrieveItemsJob(const Akonadi::Collection &collection, const KPIM::Maildir &md, QObject *parent)
    : Job(parent)
    , m_collection(collection)
    , m_maildir(md)
    , m_mimeType(KMime::Message::mimeType())
{
    Q_ASSERT(m_collection.isValid());
    Q_ASSERT(m_maildir.isValid());
}

void RetrieveItemsJob::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

void RetrieveItemsJob::doStart()
{
    Q_ASSERT(!m_mimeType.isEmpty());
    auto job = new Akonadi::ItemFetchJob(m_collection, this);
    connect(job, &Akonadi::ItemFetchJob::result, this, &RetrieveItemsJob::localListDone);
}

void RetrieveItemsJob::localListDone(KJob *job)
{
    if (job->error()) {
        qCDebug(MAILDIRRESOURCE_LOG) << "Error during RetrieveItemsJob::localListDone " << job->errorString();
        return; // handled by base class
    }

    const Akonadi::Item::List items = qobject_cast<Akonadi::ItemFetchJob *>(job)->items();
    m_localItems.reserve(items.size());
    for (const Akonadi::Item &item : items) {
        if (!item.remoteId().isEmpty()) {
            m_localItems.insert(item.remoteId(), item);
        }
    }

    m_listingPath = m_maildir.path() + QLatin1String("/new/");
    delete m_entryIterator;
    m_entryIterator = new QDirIterator(m_maildir.pathToNew(), QDir::Files);
    m_previousMtime = m_collection.remoteRevision().toLongLong();
    m_highestMtime = 0;
    QMetaObject::invokeMethod(this, &RetrieveItemsJob::processEntry, Qt::QueuedConnection);
}

void RetrieveItemsJob::processEntry()
{
    Akonadi::TransactionSequence *lastTrx = nullptr;

    while (m_entryIterator->hasNext() || m_listingPath.endsWith(QLatin1String("/new/"))) {
        if (!m_entryIterator->hasNext()) {
            m_listingPath = m_maildir.path() + QLatin1String("/cur/");
            delete m_entryIterator;
            m_entryIterator = new QDirIterator(m_maildir.pathToCurrent(), QDir::Files);
            if (!m_entryIterator->hasNext()) {
                break;
            }
        }
        m_entryIterator->next();

        const QFileInfo entryInfo = m_entryIterator->fileInfo();
        const QString fileName = entryInfo.fileName();
        const qint64 currentMtime = entryInfo.lastModified().toMSecsSinceEpoch();
        m_highestMtime = qMax(m_highestMtime, currentMtime);
        if (currentMtime <= m_previousMtime) {
            auto localItemIter = m_localItems.find(fileName);
            if (localItemIter != m_localItems.end()) { // old, we got this one already
                m_localItems.erase(localItemIter);
                continue;
            }
        }

        Akonadi::Item item;
        item.setRemoteId(fileName);
        item.setMimeType(m_mimeType);
        const qint64 entrySize = entryInfo.size();
        if (entrySize >= 0) {
            item.setSize(entrySize);
        }

        auto msg = new KMime::Message;
        msg->setHead(KMime::CRLFtoLF(m_maildir.readEntryHeadersFromFile(m_listingPath + fileName)));
        msg->parse();

        const Akonadi::Item::Flags flags = m_maildir.readEntryFlags(fileName);
        for (const Akonadi::Item::Flag &flag : flags) {
            item.setFlag(flag);
        }

        item.setPayload(KMime::Message::Ptr(msg));
        Akonadi::MessageFlags::copyMessageFlags(*msg, item);
        auto localItemIter = m_localItems.find(fileName);
        Akonadi::TransactionSequence *trx = transaction();
        if (localItemIter == m_localItems.end()) { // new item
            new Akonadi::ItemCreateJob(item, m_collection, trx);
        } else { // modification
            item.setId((*localItemIter).id());
            new Akonadi::ItemModifyJob(item, trx);
            m_localItems.erase(localItemIter);
        }
        if (trx != lastTrx) {
            lastTrx = trx;
            QMetaObject::invokeMethod(this, &RetrieveItemsJob::processEntry, Qt::QueuedConnection);
            return;
        }
    }

    entriesProcessed();
    // connect(job, &Akonadi::ItemCreateJob::result, this, &RetrieveItemsJob::processEntryDone);
}

void RetrieveItemsJob::processEntryDone(KJob *)
{
    processEntry();
}

void RetrieveItemsJob::entriesProcessed()
{
    delete m_entryIterator;
    m_entryIterator = nullptr;
    if (!m_localItems.isEmpty()) {
        auto job = new Akonadi::ItemDeleteJob(Akonadi::valuesToVector(m_localItems), transaction());
        m_maildir.removeCachedKeys(m_localItems.keys());
        // We ensure m_transaction is valid by calling transaction() above,
        // however calling it again here could cause it to give us another transaction
        // object (see transaction() comment for details)
        m_transaction->setIgnoreJobFailure(job);
    }

    // update mtime
    if (m_highestMtime != m_previousMtime) {
        Akonadi::Collection newCol(m_collection);
        newCol.setRemoteRevision(QString::number(m_highestMtime));
        auto job = new Akonadi::CollectionModifyJob(newCol, transaction());
        m_transaction->setIgnoreJobFailure(job);
    }

    if (!m_transaction) { // no jobs created here -> done
        emitResult();
    } else {
        connect(m_transaction, &Akonadi::TransactionSequence::result, this, &RetrieveItemsJob::transactionDone);
        m_transaction->commit();
    }
}

Akonadi::TransactionSequence *RetrieveItemsJob::transaction()
{
    // Commit transaction every 100 items, otherwise we are forcing server to
    // hold the database transaction opened for potentially massive amount of
    // operations, which slowly overloads the database journal causing simple
    // INSERT to take several seconds
    if (++m_transactionSize >= 100) {
        qCDebug(MAILDIRRESOURCE_LOG) << "Commit!";
        m_transaction->commit();
        m_transaction = nullptr;
        m_transactionSize = 0;
    }

    if (!m_transaction) {
        m_transaction = new Akonadi::TransactionSequence(this);
        m_transaction->setAutomaticCommittingEnabled(false);
    }
    return m_transaction;
}

void RetrieveItemsJob::transactionDone(KJob *job)
{
    if (job->error()) {
        qCDebug(MAILDIRRESOURCE_LOG) << "Error during transaction " << job->errorString();
        return; // handled by base class
    }
    emitResult();
}
