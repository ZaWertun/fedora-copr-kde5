/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrieveitemstask.h"

#include "batchfetcher.h"
#include "collectionflagsattribute.h"
#include "highestmodseqattribute.h"
#include "messagehelper.h"
#include "noselectattribute.h"
#include "uidnextattribute.h"
#include "uidvalidityattribute.h"

#include <Akonadi/AgentBase>
#include <Akonadi/CachePolicy>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageParts>
#include <Akonadi/Session>

#include "imapresource_debug.h"

#include <KLocalizedString>

#include <KIMAP/ExpungeJob>
#include <KIMAP/SearchJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KIMAP/StatusJob>

RetrieveItemsTask::RetrieveItemsTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
    , m_fetchedMissingBodies(-1)
{
}

RetrieveItemsTask::~RetrieveItemsTask() = default;

void RetrieveItemsTask::setFetchMissingItemBodies(bool enabled)
{
    m_fetchMissingBodies = enabled;
}

void RetrieveItemsTask::doStart(KIMAP::Session *session)
{
    emitPercent(0);
    // Prevent fetching items from noselect folders.
    if (collection().hasAttribute("noselect")) {
        NoSelectAttribute *noselect = static_cast<NoSelectAttribute *>(collection().attribute("noselect"));
        if (noselect->noSelect()) {
            qCDebug(IMAPRESOURCE_LOG) << "No Select folder";
            itemsRetrievalDone();
            return;
        }
    }

    m_session = session;

    const Akonadi::Collection col = collection();
    // Only with emails we can be sure that RID is persistent and thus we can use
    // it for merging. For other potential content types (like Kolab events etc.)
    // use GID instead.
    QStringList cts = col.contentMimeTypes();
    cts.removeOne(Akonadi::Collection::mimeType());
    cts.removeOne(KMime::Message::mimeType());
    if (!cts.isEmpty()) {
        setItemMergingMode(Akonadi::ItemSync::GIDMerge);
    } else {
        setItemMergingMode(Akonadi::ItemSync::RIDMerge);
    }

    if (m_fetchMissingBodies
        && col.cachePolicy().localParts().contains(QLatin1String(Akonadi::MessagePart::Body))) { // disconnected mode, make sure we really have the body cached
        auto session = new Akonadi::Session(resourceName().toLatin1() + "_body_checker", this);
        auto fetchJob = new Akonadi::ItemFetchJob(col, session);
        fetchJob->fetchScope().setCheckForCachedPayloadPartsOnly();
        fetchJob->fetchScope().fetchPayloadPart(Akonadi::MessagePart::Body);
        fetchJob->fetchScope().setFetchModificationTime(false);
        connect(fetchJob, &Akonadi::ItemFetchJob::result, this, &RetrieveItemsTask::fetchItemsWithoutBodiesDone);
        connect(fetchJob, &Akonadi::ItemFetchJob::result, session, &Akonadi::Session::deleteLater);
    } else {
        startRetrievalTasks();
    }
}

BatchFetcher *RetrieveItemsTask::createBatchFetcher(MessageHelper::Ptr messageHelper,
                                                    const KIMAP::ImapSet &set,
                                                    const KIMAP::FetchJob::FetchScope &scope,
                                                    int batchSize,
                                                    KIMAP::Session *session)
{
    return new BatchFetcher(messageHelper, set, scope, batchSize, session);
}

void RetrieveItemsTask::fetchItemsWithoutBodiesDone(KJob *job)
{
    QVector<qint64> uids;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
        return;
    } else {
        int i = 0;
        auto fetch = static_cast<Akonadi::ItemFetchJob *>(job);
        const Akonadi::Item::List lstItems = fetch->items();
        for (const Akonadi::Item &item : lstItems) {
            if (!item.cachedPayloadParts().contains(Akonadi::MessagePart::Body)) {
                qCWarning(IMAPRESOURCE_LOG) << "Item " << item.id() << " is missing the payload! Cached payloads: " << item.cachedPayloadParts();
                uids.append(item.remoteId().toInt());
                i++;
            }
        }
        if (i > 0) {
            qCWarning(IMAPRESOURCE_LOG) << "Number of items missing the body: " << i;
        }
    }
    onFetchItemsWithoutBodiesDone(uids);
}

void RetrieveItemsTask::onFetchItemsWithoutBodiesDone(const QVector<qint64> &items)
{
    m_messageUidsMissingBody = items;
    startRetrievalTasks();
}

void RetrieveItemsTask::startRetrievalTasks()
{
    const QString mailBox = mailBoxForCollection(collection());
    qCDebug(IMAPRESOURCE_LOG) << "Starting retrieval for " << mailBox;
    m_time.start();

    // Now is the right time to expunge the messages marked \\Deleted from this mailbox.
    const bool hasACL = serverCapabilities().contains(QLatin1String("ACL"));
    const KIMAP::Acl::Rights rights = myRights(collection());
    if (isAutomaticExpungeEnabled() && (!hasACL || (rights & KIMAP::Acl::Expunge) || (rights & KIMAP::Acl::Delete))) {
        if (m_session->selectedMailBox() != mailBox) {
            triggerPreExpungeSelect(mailBox);
        } else {
            triggerExpunge(mailBox);
        }
    } else {
        // Always select to get the stats updated
        triggerFinalSelect(mailBox);
    }
}

void RetrieveItemsTask::triggerPreExpungeSelect(const QString &mailBox)
{
    auto select = new KIMAP::SelectJob(m_session);
    select->setMailBox(mailBox);
    select->setCondstoreEnabled(serverSupportsCondstore());
    connect(select, &KJob::result, this, &RetrieveItemsTask::onPreExpungeSelectDone);
    select->start();
}

void RetrieveItemsTask::onPreExpungeSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto select = static_cast<KIMAP::SelectJob *>(job);
        if (select->isOpenReadOnly()) {
            qCDebug(IMAPRESOURCE_LOG) << "Mailbox is opened readonly, not expunging";
            // Treat this SELECT as if it was triggerFinalSelect()
            onFinalSelectDone(job);
        } else {
            triggerExpunge(select->mailBox());
        }
    }
}

void RetrieveItemsTask::triggerExpunge(const QString &mailBox)
{
    Q_UNUSED(mailBox)
    auto expunge = new KIMAP::ExpungeJob(m_session);
    connect(expunge, &KJob::result, this, &RetrieveItemsTask::onExpungeDone);
    expunge->start();
}

void RetrieveItemsTask::onExpungeDone(KJob *job)
{
    // We can ignore the error, we just had a wrong expunge so some old messages will just reappear.
    // TODO we should probably hide messages that are marked as deleted (skipping will not work because we rely on the message count)
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Expunge failed: " << job->errorString();
    }
    // Except for network errors.
    if (job->error() && m_session->state() == KIMAP::Session::Disconnected) {
        cancelTask(job->errorString());
        return;
    }

    // We have to re-select the mailbox to update all the stats after the expunge
    // (the EXPUNGE command doesn't return enough for our needs)
    triggerFinalSelect(m_session->selectedMailBox());
}

void RetrieveItemsTask::triggerFinalSelect(const QString &mailBox)
{
    auto select = new KIMAP::SelectJob(m_session);
    select->setMailBox(mailBox);
    select->setCondstoreEnabled(serverSupportsCondstore());
    connect(select, &KJob::result, this, &RetrieveItemsTask::onFinalSelectDone);
    select->start();
}

void RetrieveItemsTask::onFinalSelectDone(KJob *job)
{
    auto select = qobject_cast<KIMAP::SelectJob *>(job);

    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << select->mailBox() << ":" << job->errorString();
        cancelTask(select->mailBox() + QStringLiteral(" : ") + job->errorString());
        return;
    }

    m_mailBox = select->mailBox();
    m_messageCount = select->messageCount();
    m_uidValidity = select->uidValidity();
    m_nextUid = select->nextUid();
    m_highestModSeq = select->highestModSequence();
    m_flags = select->permanentFlags();

    // This is known to happen with Courier IMAP.
    if (m_nextUid < 0) {
        auto status = new KIMAP::StatusJob(m_session);
        status->setMailBox(m_mailBox);
        status->setDataItems({"UIDNEXT"});
        connect(status, &KJob::result, this, &RetrieveItemsTask::onStatusDone);
        status->start();
    } else {
        prepareRetrieval();
    }
}

void RetrieveItemsTask::onStatusDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    auto status = qobject_cast<KIMAP::StatusJob *>(job);
    const QList<QPair<QByteArray, qint64>> results = status->status();
    for (const auto &val : results) {
        if (val.first == "UIDNEXT") {
            m_nextUid = val.second;
            break;
        }
    }

    prepareRetrieval();
}

void RetrieveItemsTask::prepareRetrieval()
{
    // Handle invalid UIDNEXT in case even STATUS is not able to retrieve it
    if (m_nextUid < 0) {
        qCWarning(IMAPRESOURCE_LOG) << "Server bug: Your IMAP Server delivered an invalid UIDNEXT value.";
        m_nextUid = 0;
    }

    // The select job retrieves highestmodseq whenever it's available, but in case of no CONDSTORE support we ignore it
    if (!serverSupportsCondstore()) {
        m_localHighestModSeq = 0;
    }

    Akonadi::Collection col = collection();
    bool modifyNeeded = false;

    // Get the current uid validity value and store it
    int oldUidValidity = 0;
    if (!col.hasAttribute("uidvalidity")) {
        auto currentUidValidity = new UidValidityAttribute(m_uidValidity);
        col.addAttribute(currentUidValidity);
        modifyNeeded = true;
    } else {
        UidValidityAttribute *currentUidValidity = static_cast<UidValidityAttribute *>(col.attribute("uidvalidity"));
        oldUidValidity = currentUidValidity->uidValidity();
        if (oldUidValidity != m_uidValidity) {
            currentUidValidity->setUidValidity(m_uidValidity);
            modifyNeeded = true;
        }
    }

    // Get the current uid next value and store it
    int oldNextUid = 0;
    if (m_nextUid > 0) { // this can fail with faulty servers that don't deliver uidnext
        if (auto currentNextUid = col.attribute<UidNextAttribute>()) {
            oldNextUid = currentNextUid->uidNext();
            if (oldNextUid != m_nextUid) {
                currentNextUid->setUidNext(m_nextUid);
                modifyNeeded = true;
            }
        } else {
            col.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(m_nextUid);
            modifyNeeded = true;
        }
    }

    // Store the mailbox flags
    if (!col.hasAttribute("collectionflags")) {
        auto flagsAttribute = new Akonadi::CollectionFlagsAttribute(m_flags);
        col.addAttribute(flagsAttribute);
        modifyNeeded = true;
    } else {
        Akonadi::CollectionFlagsAttribute *flagsAttribute = static_cast<Akonadi::CollectionFlagsAttribute *>(col.attribute("collectionflags"));
        const QList<QByteArray> oldFlags = flagsAttribute->flags();
        if (oldFlags != m_flags) {
            flagsAttribute->setFlags(m_flags);
            modifyNeeded = true;
        }
    }

    qint64 oldHighestModSeq = 0;
    if (serverSupportsCondstore() && m_highestModSeq > 0) {
        if (!col.hasAttribute("highestmodseq")) {
            auto attr = new HighestModSeqAttribute(m_highestModSeq);
            col.addAttribute(attr);
            modifyNeeded = true;
        } else {
            auto attr = col.attribute<HighestModSeqAttribute>();
            if (attr->highestModSequence() < m_highestModSeq) {
                oldHighestModSeq = attr->highestModSequence();
                attr->setHighestModSeq(m_highestModSeq);
                modifyNeeded = true;
            } else if (attr->highestModSequence() == m_highestModSeq) {
                oldHighestModSeq = attr->highestModSequence();
            } else if (attr->highestModSequence() > m_highestModSeq) {
                // This situation should not happen. If it does, update the highestModSeq
                // attribute, but rather do a full sync
                attr->setHighestModSeq(m_highestModSeq);
                modifyNeeded = true;
            }
        }
    }
    m_localHighestModSeq = oldHighestModSeq;

    if (modifyNeeded) {
        m_modifiedCollection = col;
    }

    KIMAP::FetchJob::FetchScope scope;
    scope.parts.clear();
    scope.mode = KIMAP::FetchJob::FetchScope::FullHeaders;

    if (col.cachePolicy().localParts().contains(QLatin1String(Akonadi::MessagePart::Body))) {
        scope.mode = KIMAP::FetchJob::FetchScope::Full;
    }

    const qint64 realMessageCount = col.statistics().count();

    qCDebug(IMAPRESOURCE_LOG) << "Starting message retrieval. Elapsed(ms): " << m_time.elapsed();
    qCDebug(IMAPRESOURCE_LOG) << "UidValidity: " << m_uidValidity << "Local UidValidity: " << oldUidValidity;
    qCDebug(IMAPRESOURCE_LOG) << "MessageCount: " << m_messageCount << "Local message count: " << realMessageCount;
    qCDebug(IMAPRESOURCE_LOG) << "UidNext: " << m_nextUid << "Local UidNext: " << oldNextUid;
    qCDebug(IMAPRESOURCE_LOG) << "HighestModSeq: " << m_highestModSeq << "Local HighestModSeq: " << oldHighestModSeq;

    /*
     * A synchronization has 3 mandatory steps:
     * * If uidvalidity changed the local cache must be invalidated
     * * New messages can be fetched using uidNext and the last known fetched uid
     * * flag changes and removals can be detected by listing all messages that weren't part of the previous step
     *
     * Everything else is optimizations.
     *
     * TODO: Note that the local message count can be larger than the remote message count although no messages
     * have been deleted remotely, if we locally have messages that were not yet uploaded.
     * We cannot differentiate that from remotely removed messages, so we have to do a full flag
     * listing in that case. This can be optimized once we support QRESYNC and therefore have a way
     * to determine whether messages have been removed.
     */

    if (m_messageCount == 0) {
        // Shortcut:
        // If no messages are present on the server, clear local cash and finish
        m_incremental = false;
        if (realMessageCount > 0) {
            qCDebug(IMAPRESOURCE_LOG) << "No messages present so we are done, deleting local messages.";
            itemsRetrieved(Akonadi::Item::List());
        } else {
            qCDebug(IMAPRESOURCE_LOG) << "No messages present so we are done";
        }
        taskComplete();
    } else if (oldUidValidity != m_uidValidity || m_nextUid <= 0) {
        // If uidvalidity has changed our local cache is worthless and has to be refetched completely
        if (oldUidValidity != 0 && oldUidValidity != m_uidValidity) {
            qCDebug(IMAPRESOURCE_LOG) << "UIDVALIDITY check failed (" << oldUidValidity << "|" << m_uidValidity << ")";
        }
        if (m_nextUid <= 0) {
            qCDebug(IMAPRESOURCE_LOG) << "Invalid UIDNEXT";
        }
        qCDebug(IMAPRESOURCE_LOG) << "Fetching complete mailbox " << m_mailBox;
        setTotalItems(m_messageCount);
        retrieveItems(KIMAP::ImapSet(1, m_nextUid), scope, false, true);
    } else if (m_nextUid <= 0) {
        // This is a compatibility codepath for Courier IMAP. It probably introduces problems, but at least it syncs.
        // Since we don't have uidnext available, we simply use the messagecount. This will miss simultaneously added/removed messages.
        // qCDebug(IMAPRESOURCE_LOG) << "Running courier imap compatibility codepath";
        if (m_messageCount > realMessageCount) {
            // Get new messages
            retrieveItems(KIMAP::ImapSet(realMessageCount + 1, m_messageCount), scope, false, false);
        } else if (m_messageCount == realMessageCount) {
            m_uidBasedFetch = false;
            m_incremental = true;
            setTotalItems(m_messageCount);
            listFlagsForImapSet(KIMAP::ImapSet(1, m_messageCount));
        } else {
            m_uidBasedFetch = false;
            m_incremental = false;
            setTotalItems(m_messageCount);
            listFlagsForImapSet(KIMAP::ImapSet(1, m_messageCount));
        }
    } else if (!m_messageUidsMissingBody.isEmpty()) {
        // fetch missing uids
        m_fetchedMissingBodies = 0;
        setTotalItems(m_messageUidsMissingBody.size());
        KIMAP::ImapSet imapSet;
        imapSet.add(m_messageUidsMissingBody);
        retrieveItems(imapSet, scope, true, true);
    } else if (m_nextUid > oldNextUid && ((realMessageCount + m_nextUid - oldNextUid) == m_messageCount) && realMessageCount > 0) {
        // Optimization:
        // New messages are available, but we know no messages have been removed.
        // Fetch new messages, and then check for changed flags and removed messages
        // We can make an incremental update and use modseq.
        qCDebug(IMAPRESOURCE_LOG) << "Incrementally fetching new messages: UidNext: " << m_nextUid << " Old UidNext: " << oldNextUid << " message count "
                                  << m_messageCount << realMessageCount;
        setTotalItems(qMax(1ll, m_messageCount - realMessageCount));
        m_flagsChanged = !(m_highestModSeq == oldHighestModSeq);
        retrieveItems(KIMAP::ImapSet(qMax(1, oldNextUid), m_nextUid), scope, true, true);
    } else if (m_nextUid > oldNextUid && m_messageCount > (realMessageCount + m_nextUid - oldNextUid) && realMessageCount > 0) {
        // Error recovery:
        // New messages are available, but not enough to justify the difference between the local and remote message count.
        // This can be triggered if we i.e. clear the local cache, but the keep the annotations.
        // If we didn't catch this case, we end up inserting flags only for every missing message.
        qCWarning(IMAPRESOURCE_LOG) << m_mailBox << ": detected inconsistency in local cache, we're missing some messages. Server: " << m_messageCount
                                    << " Local: " << realMessageCount;
        qCWarning(IMAPRESOURCE_LOG) << m_mailBox << ": refetching complete mailbox";
        setTotalItems(m_messageCount);
        retrieveItems(KIMAP::ImapSet(1, m_nextUid), scope, false, true);
    } else if (m_nextUid > oldNextUid) {
        // New messages are available. Fetch new messages, and then check for changed flags and removed messages
        qCDebug(IMAPRESOURCE_LOG) << "Fetching new messages: UidNext: " << m_nextUid << " Old UidNext: " << oldNextUid;
        setTotalItems(m_messageCount);
        retrieveItems(KIMAP::ImapSet(qMax(1, oldNextUid), m_nextUid), scope, false, true);
    } else if (m_messageCount == realMessageCount && oldNextUid == m_nextUid) {
        // Optimization:
        // We know no messages were added or removed (if the message count and uidnext is still the same)
        // We only check the flags incrementally and can make use of modseq
        m_uidBasedFetch = true;
        m_incremental = true;
        m_flagsChanged = !(m_highestModSeq == oldHighestModSeq);
        // Workaround: If the server doesn't support CONDSTORE we would end up syncing all flags during every sync.
        // Instead we only sync flags when new messages are available or removed and skip this step.
        // WARNING: This sacrifices consistency as we will not detect flag changes until a new message enters the mailbox.
        if (m_incremental && !serverSupportsCondstore()) {
            qCDebug(IMAPRESOURCE_LOG) << "Avoiding flag sync due to missing CONDSTORE support";
            taskComplete();
            return;
        }
        setTotalItems(m_messageCount);
        listFlagsForImapSet(KIMAP::ImapSet(1, m_nextUid));
    } else if (m_messageCount > realMessageCount) {
        // Error recovery:
        // We didn't detect any new messages based on the uid, but according to the message count there are new ones.
        // Our local cache is invalid and has to be refetched.
        qCWarning(IMAPRESOURCE_LOG) << m_mailBox << ": detected inconsistency in local cache, we're missing some messages. Server: " << m_messageCount
                                    << " Local: " << realMessageCount;
        qCWarning(IMAPRESOURCE_LOG) << m_mailBox << ": refetching complete mailbox";
        setTotalItems(m_messageCount);
        retrieveItems(KIMAP::ImapSet(1, m_nextUid), scope, false, true);
    } else {
        // Shortcut:
        // No new messages are available. Directly check for changed flags and removed messages.
        m_uidBasedFetch = true;
        m_incremental = false;
        setTotalItems(m_messageCount);
        listFlagsForImapSet(KIMAP::ImapSet(1, m_nextUid));
    }
}

void RetrieveItemsTask::retrieveItems(const KIMAP::ImapSet &set, const KIMAP::FetchJob::FetchScope &scope, bool incremental, bool uidBased)
{
    Q_ASSERT(set.intervals().size() == 1);

    m_incremental = incremental;
    m_uidBasedFetch = uidBased;

    m_batchFetcher = createBatchFetcher(resourceState()->messageHelper(), set, scope, batchSize(), m_session);
    m_batchFetcher->setUidBased(m_uidBasedFetch);
    if (m_uidBasedFetch && set.intervals().size() == 1) {
        m_batchFetcher->setSearchUids(set.intervals().front());
    }
    m_batchFetcher->setProperty("alreadyFetched", set.intervals().at(0).begin());
    connect(m_batchFetcher, &BatchFetcher::itemsRetrieved, this, &RetrieveItemsTask::onItemsRetrieved);
    connect(m_batchFetcher, &KJob::result, this, &RetrieveItemsTask::onRetrievalDone);
    m_batchFetcher->start();
}

void RetrieveItemsTask::onReadyForNextBatch(int size)
{
    Q_UNUSED(size)
    if (m_batchFetcher) {
        m_batchFetcher->fetchNextBatch();
    }
}

void RetrieveItemsTask::onItemsRetrieved(const Akonadi::Item::List &addedItems)
{
    if (m_incremental) {
        itemsRetrievedIncremental(addedItems, Akonadi::Item::List());
    } else {
        itemsRetrieved(addedItems);
    }

    // m_fetchedMissingBodies is -1 if we fetch for other reason, but missing bodies
    if (m_fetchedMissingBodies != -1) {
        const QString mailBox = mailBoxForCollection(collection());
        m_fetchedMissingBodies += addedItems.count();
        Q_EMIT status(Akonadi::AgentBase::Running,
                      i18nc("@info:status", "Fetching missing mail bodies in %3: %1/%2", m_fetchedMissingBodies, m_messageUidsMissingBody.count(), mailBox));
    }
}

void RetrieveItemsTask::onRetrievalDone(KJob *job)
{
    m_batchFetcher = nullptr;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
        m_fetchedMissingBodies = -1;
        return;
    }

    // This is the lowest sequence number that we just fetched.
    const auto alreadyFetchedBegin = job->property("alreadyFetched").value<KIMAP::ImapSet::Id>();

    // If this is the first fetch of a folder, skip getting flags, we
    // already have them all from the previous full fetch. This is not
    // just an optimization, as incremental retrieval assumes nothing
    // will be listed twice.
    if (m_fetchedMissingBodies != -1 || alreadyFetchedBegin <= 1) {
        taskComplete();
        return;
    }

    // Fetch flags of all items that were not fetched by the fetchJob. After
    // that /all/ items in the folder are synced.
    listFlagsForImapSet(KIMAP::ImapSet(1, alreadyFetchedBegin - 1));
}

void RetrieveItemsTask::listFlagsForImapSet(const KIMAP::ImapSet &set)
{
    qCDebug(IMAPRESOURCE_LOG) << "Listing flags " << set.intervals().at(0).begin() << set.intervals().at(0).end();
    qCDebug(IMAPRESOURCE_LOG) << "Starting flag retrieval. Elapsed(ms): " << m_time.elapsed();

    KIMAP::FetchJob::FetchScope scope;
    scope.parts.clear();
    scope.mode = KIMAP::FetchJob::FetchScope::Flags;
    // Only use changeSince when doing incremental listings,
    // otherwise we would overwrite our local data with an incomplete dataset
    if (m_incremental && serverSupportsCondstore()) {
        scope.changedSince = m_localHighestModSeq;
        if (!m_flagsChanged) {
            qCDebug(IMAPRESOURCE_LOG) << "No flag changes.";
            taskComplete();
            return;
        }
    }

    m_batchFetcher = createBatchFetcher(resourceState()->messageHelper(), set, scope, 10 * batchSize(), m_session);
    m_batchFetcher->setUidBased(m_uidBasedFetch);
    if (m_uidBasedFetch && scope.changedSince == 0 && set.intervals().size() == 1) {
        m_batchFetcher->setSearchUids(set.intervals().front());
    }
    connect(m_batchFetcher, &BatchFetcher::itemsRetrieved, this, &RetrieveItemsTask::onItemsRetrieved);
    connect(m_batchFetcher, &KJob::result, this, &RetrieveItemsTask::onFlagsFetchDone);
    m_batchFetcher->start();
}

void RetrieveItemsTask::onFlagsFetchDone(KJob *job)
{
    m_batchFetcher = nullptr;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
    } else {
        taskComplete();
    }
}

void RetrieveItemsTask::taskComplete()
{
    if (m_modifiedCollection.isValid()) {
        qCDebug(IMAPRESOURCE_LOG) << "Applying collection changes";
        applyCollectionChanges(m_modifiedCollection);
    }
    if (m_incremental) {
        // Calling itemsRetrievalDone() before previous call to itemsRetrievedIncremental()
        // behaves like if we called itemsRetrieved(Items::List()), so make sure
        // Akonadi knows we did incremental fetch that came up with no changes
        itemsRetrievedIncremental(Akonadi::Item::List(), Akonadi::Item::List());
    }
    qCDebug(IMAPRESOURCE_LOG) << "Retrieval complete. Elapsed(ms): " << m_time.elapsed();
    itemsRetrievalDone();
}
