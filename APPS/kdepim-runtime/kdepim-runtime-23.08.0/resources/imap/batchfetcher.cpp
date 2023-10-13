/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "batchfetcher.h"

#include "imapresource_debug.h"
#include <KIMAP/Session>
BatchFetcher::BatchFetcher(MessageHelper::Ptr messageHelper,
                           const KIMAP::ImapSet &set,
                           const KIMAP::FetchJob::FetchScope &scope,
                           int batchSize,
                           KIMAP::Session *session)
    : KJob(session)
    , m_currentSet(set)
    , m_scope(scope)
    , m_session(session)
    , m_batchSize(batchSize)
    , m_messageHelper(messageHelper)
{
}

BatchFetcher::~BatchFetcher() = default;

void BatchFetcher::setUidBased(bool uidBased)
{
    m_uidBased = uidBased;
}

void BatchFetcher::setSearchUids(const KIMAP::ImapInterval &interval)
{
    m_searchUidInterval = interval;

    // We look up the UIDs ourselves
    m_currentSet = KIMAP::ImapSet();

    // MS Exchange can't handle big results so we have to split the search into small chunks
    m_searchInChunks = m_session->serverGreeting().contains("Microsoft Exchange");
}

void BatchFetcher::setGmailExtensionsEnabled(bool enable)
{
    m_gmailEnabled = enable;
}

static const int maxAmountOfUidToSearchInOneTime = 2000;

void BatchFetcher::start()
{
    if (m_searchUidInterval.size()) {
        // Search in chunks also Exchange can handle
        const KIMAP::ImapInterval::Id firstUidToSearch = m_searchUidInterval.begin();
        const KIMAP::ImapInterval::Id lastUidToSearch =
            m_searchInChunks ? qMin(firstUidToSearch + maxAmountOfUidToSearchInOneTime - 1, m_searchUidInterval.end()) : m_searchUidInterval.end();

        // Prepare next chunk
        const KIMAP::ImapInterval::Id intervalBegin = lastUidToSearch + 1;
        // Or are we already done?
        if (intervalBegin > m_searchUidInterval.end()) {
            m_searchUidInterval = KIMAP::ImapInterval();
        } else {
            m_searchUidInterval.setBegin(intervalBegin);
        }

        // Resolve the uid to sequence numbers
        auto search = new KIMAP::SearchJob(m_session);
        search->setUidBased(true);
        search->setTerm(KIMAP::Term(KIMAP::Term::Uid, KIMAP::ImapSet(firstUidToSearch, lastUidToSearch)));
        connect(search, &KIMAP::SearchJob::result, this, &BatchFetcher::onUidSearchDone);
        search->start();
    } else {
        fetchNextBatch();
    }
}

void BatchFetcher::onUidSearchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Search job failed: " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }

    auto search = static_cast<KIMAP::SearchJob *>(job);
    m_uidBased = search->isUidBased();
    m_currentSet.add(search->results());

    // More to search?
    start();
}

void BatchFetcher::fetchNextBatch()
{
    if (m_fetchInProgress) {
        m_continuationRequested = true;
        return;
    }
    m_continuationRequested = false;
    Q_ASSERT(m_batchSize > 0);
    if (m_currentSet.isEmpty()) {
        qCDebug(IMAPRESOURCE_LOG) << "fetch complete";
        emitResult();
        return;
    }

    auto fetch = new KIMAP::FetchJob(m_session);
    if (m_scope.changedSince != 0) {
        qCDebug(IMAPRESOURCE_LOG) << "Fetching all messages in one batch.";
        fetch->setSequenceSet(m_currentSet);
        m_currentSet = KIMAP::ImapSet();
    } else {
        KIMAP::ImapSet toFetch;
        qint64 counter = 0;
        KIMAP::ImapSet newSet;

        // Take a chunk from the set
        const auto intervals{m_currentSet.intervals()};
        for (const KIMAP::ImapInterval &interval : intervals) {
            if (!interval.hasDefinedEnd()) {
                // If we get an interval without a defined end we simply fetch everything
                qCDebug(IMAPRESOURCE_LOG) << "Received interval without defined end, fetching everything in one batch";
                toFetch.add(interval);
                newSet = KIMAP::ImapSet();
                break;
            }
            const qint64 wantedItems = m_batchSize - counter;
            if (counter < m_batchSize) {
                if (interval.size() <= wantedItems) {
                    counter += interval.size();
                    toFetch.add(interval);
                } else {
                    counter += wantedItems;
                    toFetch.add(KIMAP::ImapInterval(interval.begin(), interval.begin() + wantedItems - 1));
                    newSet.add(KIMAP::ImapInterval(interval.begin() + wantedItems, interval.end()));
                }
            } else {
                newSet.add(interval);
            }
        }
        qCDebug(IMAPRESOURCE_LOG) << "Fetching " << toFetch.intervals().size() << " intervals";
        fetch->setSequenceSet(toFetch);
        m_currentSet = newSet;
    }

    fetch->setUidBased(m_uidBased);
    fetch->setScope(m_scope);
    fetch->setGmailExtensionsEnabled(m_gmailEnabled);
    connect(fetch, &KIMAP::FetchJob::messagesAvailable, this, &BatchFetcher::onMessagesAvailable);
    connect(fetch, &KJob::result, this, &BatchFetcher::onHeadersFetchDone);
    m_fetchInProgress = true;
    fetch->start();
}

void BatchFetcher::onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages)
{
    auto fetch = static_cast<KIMAP::FetchJob *>(sender());

    Akonadi::Item::List addedItems;
    for (auto msg = messages.cbegin(), end = messages.cend(); msg != end; ++msg) {
        // qDebug( 5327 ) << "Flags: " << i.flags();
        bool ok;
        const auto item = m_messageHelper->createItemFromMessage(msg->message, msg->uid, msg->size, msg->attributes, msg->flags, fetch->scope(), ok);
        if (ok) {
            m_fetchedItemsInCurrentBatch++;
            addedItems << item;
        }
    }
    //     qCDebug(IMAPRESOURCE_LOG) << addedItems.size();
    if (!addedItems.isEmpty()) {
        Q_EMIT itemsRetrieved(addedItems);
    }
}

void BatchFetcher::onHeadersFetchDone(KJob *job)
{
    m_fetchInProgress = false;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Fetch job failed " << job->errorString();
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    if (m_currentSet.isEmpty()) {
        emitResult();
        return;
    }
    // Fetch more if we didn't deliver enough yet.
    // This can happen because no message is in the fetched uid range, or if the translation failed
    if (m_fetchedItemsInCurrentBatch < m_batchSize) {
        fetchNextBatch();
    } else {
        m_fetchedItemsInCurrentBatch = 0;
        // Also fetch more if we already got a continuation request during the fetch.
        // This can happen if we deliver too many items during a previous batch (after using )
        // Note that m_fetchedItemsInCurrentBatch will be off by the items that we delivered already.
        if (m_continuationRequested) {
            fetchNextBatch();
        }
    }
}
