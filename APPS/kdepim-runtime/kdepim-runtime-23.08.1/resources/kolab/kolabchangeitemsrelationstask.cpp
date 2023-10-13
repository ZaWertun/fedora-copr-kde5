/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabchangeitemsrelationstask.h"
#include "kolabresource_debug.h"
#include "kolabresource_trace.h"
#include "pimkolab/kolabformat/kolabobject.h"
#include <imapflags.h>

#include <KIMAP/AppendJob>
#include <KIMAP/ImapSet>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>
#include <KIMAP/StoreJob>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/RelationFetchJob>

#include "kolabhelpers.h"

KolabChangeItemsRelationsTask::KolabChangeItemsRelationsTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : KolabRelationResourceTask(resource, parent)
{
}

void KolabChangeItemsRelationsTask::startRelationTask(KIMAP::Session *session)
{
    qCDebug(KOLABRESOURCE_TRACE);
    mSession = session;
    mAddedRelations = resourceState()->addedRelations();
    mRemovedRelations = resourceState()->removedRelations();

    processNextRelation();
}

void KolabChangeItemsRelationsTask::processNextRelation()
{
    qCDebug(KOLABRESOURCE_TRACE) << mAddedRelations.size() << mRemovedRelations.size();
    Akonadi::Relation relation;
    if (!mAddedRelations.isEmpty()) {
        relation = mAddedRelations.takeFirst();
        mAdding = true;
    } else if (!mRemovedRelations.isEmpty()) {
        relation = mRemovedRelations.takeFirst();
        mAdding = false;
    } else {
        qCDebug(KOLABRESOURCE_TRACE) << "Processing done";
        changeProcessed();
        return;
    }
    qCDebug(KOLABRESOURCE_TRACE) << "Processing " << (mAdding ? " add " : " remove ") << relation;

    // We have to fetch it again in case it changed since the notification was emitted (which is likely)
    // Otherwise we get an empty remoteid for new tags that were immediately applied on an item
    auto fetch = new Akonadi::RelationFetchJob(relation);
    connect(fetch, &KJob::result, this, &KolabChangeItemsRelationsTask::onRelationFetchDone);
}

void KolabChangeItemsRelationsTask::onRelationFetchDone(KJob *job)
{
    qCDebug(KOLABRESOURCE_TRACE);
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "RelatonFetch failed: " << job->errorString();
        processNextRelation();
        return;
    }

    const Akonadi::Relation::List relations = static_cast<Akonadi::RelationFetchJob *>(job)->relations();
    if (relations.size() != 1) {
        qCWarning(KOLABRESOURCE_LOG) << "Invalid number of relations retrieved: " << relations.size();
        processNextRelation();
        return;
    }

    Akonadi::Relation relation = relations.first();
    if (mAdding) {
        addRelation(relation);
    } else {
        removeRelation(relation);
    }
}

void KolabChangeItemsRelationsTask::addRelation(const Akonadi::Relation &relation)
{
    auto fetchJob = new Akonadi::ItemFetchJob(Akonadi::Item::List() << relation.left() << relation.right());
    fetchJob->fetchScope().setCacheOnly(true);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::All);
    fetchJob->fetchScope().setFetchGid(true);
    fetchJob->fetchScope().fetchFullPayload(true);
    fetchJob->setProperty("relation", QVariant::fromValue(relation));
    connect(fetchJob, &KJob::result, this, &KolabChangeItemsRelationsTask::onItemsFetched);
}

void KolabChangeItemsRelationsTask::onItemsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to fetch items for relation: " << job->errorString();
        processNextRelation();
        return;
    }
    auto fetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    if (fetchJob->items().size() != 2) {
        qCWarning(KOLABRESOURCE_LOG) << "Invalid number of items retrieved: " << fetchJob->items().size();
        processNextRelation();
        return;
    }

    const Akonadi::Item::List items = fetchJob->items();
    const auto relation = job->property("relation").value<Akonadi::Relation>();
    Akonadi::Item leftItem = items[0] == relation.left() ? items[0] : items[1];
    Akonadi::Item rightItem = items[0] == relation.right() ? items[0] : items[1];
    const QString left = KolabHelpers::createMemberUrl(leftItem, resourceState()->userName());
    const QString right = KolabHelpers::createMemberUrl(rightItem, resourceState()->userName());
    if (left.isEmpty() || right.isEmpty()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to add relation, invalid member: " << left << " : " << right;
        processNextRelation();
        return;
    }
    QStringList members;
    members.reserve(2);
    members << left << right;

    const QLatin1String productId("Akonadi-Kolab-Resource");
    const KMime::Message::Ptr message = Kolab::KolabObjectWriter::writeRelation(relation, members, Kolab::KolabV3, productId);

    auto appendJob = new KIMAP::AppendJob(mSession);
    appendJob->setMailBox(mailBoxForCollection(relationCollection()));
    appendJob->setContent(message->encodedContent(true));
    appendJob->setInternalDate(message->date()->dateTime());
    connect(appendJob, &KJob::result, this, &KolabChangeItemsRelationsTask::onChangeCommitted);
    appendJob->start();
}

void KolabChangeItemsRelationsTask::removeRelation(const Akonadi::Relation &relation)
{
    qCDebug(KOLABRESOURCE_TRACE);
    mCurrentRelation = relation;
    const QString mailBox = mailBoxForCollection(relationCollection());

    if (mSession->selectedMailBox() != mailBox) {
        auto select = new KIMAP::SelectJob(mSession);
        select->setMailBox(mailBox);

        connect(select, &KJob::result, this, &KolabChangeItemsRelationsTask::onSelectDone);

        select->start();
    } else {
        triggerStoreJob();
    }
}

void KolabChangeItemsRelationsTask::onSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Failed to select mailbox: " << job->errorString();
        cancelTask(job->errorString());
    } else {
        triggerStoreJob();
    }
}

void KolabChangeItemsRelationsTask::triggerStoreJob()
{
    KIMAP::ImapSet set;
    set.add(mCurrentRelation.remoteId().toLong());

    qCDebug(KOLABRESOURCE_TRACE) << set.toImapSequenceSet();

    auto store = new KIMAP::StoreJob(mSession);
    store->setUidBased(true);
    store->setSequenceSet(set);
    store->setFlags(QList<QByteArray>() << ImapFlags::Deleted);
    store->setMode(KIMAP::StoreJob::AppendFlags);
    connect(store, &KJob::result, this, &KolabChangeItemsRelationsTask::onChangeCommitted);
    store->start();
}

void KolabChangeItemsRelationsTask::onChangeCommitted(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Error while storing change";
        cancelTask(job->errorString());
    } else {
        processNextRelation();
    }
}
