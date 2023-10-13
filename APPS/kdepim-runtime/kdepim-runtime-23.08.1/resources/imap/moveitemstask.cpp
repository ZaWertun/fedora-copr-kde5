/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "moveitemstask.h"

#include <QUuid>

#include "imapresource_debug.h"
#include <KLocalizedString>

#include <KIMAP/CopyJob>
#include <KIMAP/MoveJob>
#include <KIMAP/SearchJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KIMAP/StoreJob>

#include <KMime/Message>

#include "imapflags.h"
#include "uidnextattribute.h"

MoveItemsTask::MoveItemsTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

MoveItemsTask::~MoveItemsTask() = default;

void MoveItemsTask::doStart(KIMAP::Session *session)
{
    const auto itemsToMove = items();
    if (std::any_of(itemsToMove.begin(), itemsToMove.end(), [](const Akonadi::Item &item) {
            return item.remoteId().isEmpty();
        })) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed: messages has no rid";
        emitError(i18n("Cannot move message, it does not exist on the server."));
        changeProcessed();
        return;
    }

    if (sourceCollection().remoteId().isEmpty()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed: source collection has no rid";
        emitError(i18n("Cannot move message out of '%1', '%1' does not exist on the server.", sourceCollection().name()));
        changeProcessed();
        return;
    }

    if (targetCollection().remoteId().isEmpty()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed: target collection has no rid";
        emitError(i18n("Cannot move message to '%1', '%1' does not exist on the server.", targetCollection().name()));
        changeProcessed();
        return;
    }

    const QString oldMailBox = mailBoxForCollection(sourceCollection());
    const QString newMailBox = mailBoxForCollection(targetCollection());

    if (oldMailBox == newMailBox) {
        qCDebug(IMAPRESOURCE_LOG) << "Nothing to do, same mailbox";
        changeProcessed();
        return;
    }

    if (session->selectedMailBox() != oldMailBox) {
        auto select = new KIMAP::SelectJob(session);

        select->setMailBox(oldMailBox);
        connect(select, &KIMAP::SelectJob::result, this, &MoveItemsTask::onSelectDone);

        select->start();
    } else {
        startMove(session);
    }
}

void MoveItemsTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Select failed: " << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto select = static_cast<KIMAP::SelectJob *>(job);
        startMove(select->session());
    }
}

void MoveItemsTask::startMove(KIMAP::Session *session)
{
    const QString newMailBox = mailBoxForCollection(targetCollection());

    KIMAP::ImapSet set;

    // save message id, might be needed later to search for the
    // resulting message uid.
    const Akonadi::Item::List lstItems = items();
    for (const Akonadi::Item &item : lstItems) {
        try {
            auto msg = item.payload<KMime::Message::Ptr>();
            const QByteArray messageId = msg->messageID()->asUnicodeString().toUtf8();
            if (!messageId.isEmpty()) {
                m_messageIds.insert(item.id(), messageId);
            }

            set.add(item.remoteId().toLong());
        } catch (const Akonadi::PayloadException &e) {
            Q_UNUSED(e)
            qCWarning(IMAPRESOURCE_LOG) << "Move failed, payload exception " << item.id() << item.remoteId();
            cancelTask(i18n("Failed to move item, it has no message payload. Remote id: %1", item.remoteId()));
            return;
        }
    }

    const bool canMove = serverCapabilities().contains(QLatin1String("MOVE"), Qt::CaseInsensitive);
    if (canMove) {
        auto job = new KIMAP::MoveJob(session);
        job->setUidBased(true);
        job->setSequenceSet(set);
        job->setMailBox(newMailBox);
        connect(job, &KIMAP::Job::result, this, &MoveItemsTask::onMoveDone);
        job->start();
    } else {
        auto job = new KIMAP::CopyJob(session);
        job->setUidBased(true);
        job->setSequenceSet(set);
        job->setMailBox(newMailBox);
        connect(job, &KIMAP::Job ::result, this, &MoveItemsTask::onCopyDone);
        job->start();
    }
    m_oldSet = set;
}

void MoveItemsTask::onCopyDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto copy = static_cast<KIMAP::CopyJob *>(job);

        m_newUids = imapSetToList(copy->resultingUids());

        // Mark the old one ready for deletion
        auto store = new KIMAP::StoreJob(copy->session());

        store->setUidBased(true);
        store->setSequenceSet(m_oldSet);
        store->setFlags(QList<QByteArray>() << ImapFlags::Deleted);
        store->setMode(KIMAP::StoreJob::AppendFlags);

        connect(store, &KIMAP::StoreJob::result, this, &MoveItemsTask::onStoreFlagsDone);

        store->start();
    }
}

void MoveItemsTask::onMoveDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
    } else {
        auto move = static_cast<KIMAP::MoveJob *>(job);

        m_newUids = imapSetToList(move->resultingUids());

        if (!m_newUids.isEmpty()) {
            recordNewUid();
        } else {
            // Let's go for a search to find a new UID :-)

            // We did a move, so we're very likely not in the right mailbox
            auto select = new KIMAP::SelectJob(move->session());
            select->setMailBox(mailBoxForCollection(targetCollection()));
            connect(select, &KIMAP::SelectJob::result, this, &MoveItemsTask::onPreSearchSelectDone);
            select->start();
        }
    }
}

void MoveItemsTask::onStoreFlagsDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to mark message as deleted on source server: " << job->errorString();
        emitWarning(
            i18n("Failed to mark the message from '%1' for deletion on the IMAP server. "
                 "It will reappear on next sync.",
                 sourceCollection().name()));
    }

    if (!m_newUids.isEmpty()) {
        recordNewUid();
    } else {
        // Let's go for a search to find the new UID :-)

        // We did a copy we're very likely not in the right mailbox
        auto store = static_cast<KIMAP::StoreJob *>(job);
        auto select = new KIMAP::SelectJob(store->session());
        select->setMailBox(mailBoxForCollection(targetCollection()));

        connect(select, &KIMAP::SelectJob::result, this, &MoveItemsTask::onPreSearchSelectDone);

        select->start();
    }
}

void MoveItemsTask::onPreSearchSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Select failed: " << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    auto select = static_cast<KIMAP::SelectJob *>(job);
    if (!m_messageIds.isEmpty()) {
        // Use at most 250 search terms, otherwise the request might get too long and rejected
        // by the server.
        static const int batchSize = 250;
        for (int batchIdx = 0; batchIdx < m_messageIds.size(); batchIdx += batchSize) {
            const auto count = std::min<int>(batchSize, m_messageIds.size() - batchIdx);

            auto search = new KIMAP::SearchJob(select->session());
            search->setUidBased(true);
            if (batchIdx == m_messageIds.size() - count) {
                search->setProperty("IsLastSearchJob", true);
            }

            QVector<KIMAP::Term> subterms;
            subterms.reserve(count);
            auto it = m_messageIds.begin();
            std::advance(it, batchIdx);
            auto end = it;
            std::advance(end, count);
            for (; it != end; ++it) {
                subterms.push_back(KIMAP::Term(QStringLiteral("Message-ID"), QString::fromLatin1(it.value())));
            }
            search->setTerm(KIMAP::Term(KIMAP::Term::Or, subterms));
            connect(search, &KIMAP::SearchJob::result, this, &MoveItemsTask::onSearchDone);

            search->start();
        }
    } else {
        auto search = new KIMAP::SearchJob(select->session());
        search->setUidBased(true);
        search->setProperty("IsLastSearchJob", true);

        Akonadi::Collection c = targetCollection();
        auto uidNext = c.attribute<UidNextAttribute>();
        if (!uidNext) {
            cancelTask(i18n("Could not determine the UID for the newly created message on the server"));
            search->deleteLater();
            return;
        }
        search->setTerm(KIMAP::Term(KIMAP::Term::And, {KIMAP::Term(KIMAP::Term::New), KIMAP::Term(KIMAP::Term::Uid, KIMAP::ImapSet(uidNext->uidNext(), 0))}));
        connect(search, &KIMAP::SearchJob::result, this, &MoveItemsTask::onSearchDone);
        search->start();
    }
}

void MoveItemsTask::onSearchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Search failed: " << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    auto search = static_cast<KIMAP::SearchJob *>(job);
    m_newUids.append(search->results());

    if (search->property("IsLastSearchJob").toBool() == true) {
        recordNewUid();
    }
}

void MoveItemsTask::recordNewUid()
{
    // Create the item resulting of the operation, since at that point
    // the first part of the move succeeded
    const QVector<qint64> oldUids = imapSetToList(m_oldSet);

    Akonadi::Item::List newItems;
    for (int i = 0; i < oldUids.count(); ++i) {
        const QString oldUid = QString::number(oldUids.at(i));
        Akonadi::Item item;
        const Akonadi::Item::List lstItems = items();
        for (const Akonadi::Item &it : lstItems) {
            if (it.remoteId() == oldUid) {
                item = it;
                break;
            }
        }
        Q_ASSERT(item.isValid());

        // Update the item content with the new UID from the copy
        // if we didn't manage to get a valid UID from the server, use a random RID instead
        // this will make ItemSync clean up the mess during the next sync (while empty RIDs are protected as not yet existing on the server)
        if (m_newUids.count() <= i) {
            item.setRemoteId(QUuid::createUuid().toString());
        } else {
            item.setRemoteId(QString::number(m_newUids.at(i)));
        }
        newItems << item;
    }

    changesCommitted(newItems);

    Akonadi::Collection c = targetCollection();

    // Get the current uid next value and store it
    UidNextAttribute *uidAttr = nullptr;
    int oldNextUid = 0;
    if (c.hasAttribute("uidnext")) {
        uidAttr = static_cast<UidNextAttribute *>(c.attribute("uidnext"));
        oldNextUid = uidAttr->uidNext();
    }

    // If the uid we just got back is the expected next one of the box
    // then update the property to the probable next uid to keep the cache in sync.
    // If not something happened in our back, so we don't update and a refetch will
    // happen at some point.
    if (!m_newUids.isEmpty() && m_newUids.last() == oldNextUid) {
        if (uidAttr == nullptr) {
            uidAttr = new UidNextAttribute(m_newUids.last() + 1);
            c.addAttribute(uidAttr);
        } else {
            uidAttr->setUidNext(m_newUids.last() + 1);
        }

        applyCollectionChanges(c);
    }
}

QVector<qint64> MoveItemsTask::imapSetToList(const KIMAP::ImapSet &set)
{
    QVector<qint64> list;
    const KIMAP::ImapInterval::List lstInterval = set.intervals();
    list.reserve(lstInterval.count());
    for (const KIMAP::ImapInterval &interval : lstInterval) {
        if (!interval.hasDefinedEnd()) {
            qCWarning(IMAPRESOURCE_LOG) << "Trying to convert an infinite imap interval to a finite list";
            continue;
        }
        for (qint64 i = interval.begin(), end = interval.end(); i <= end; ++i) {
            list << i;
        }
    }

    return list;
}
