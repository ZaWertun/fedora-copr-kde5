/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabretrievetagstask.h"
#include "kolabresource_debug.h"
#include "kolabresource_trace.h"
#include "tagchangehelper.h"

#include "pimkolab/kolabformat/kolabobject.h"
#include <KIMAP/FetchJob>
#include <KIMAP/SelectJob>
#include <imapflags.h>

KolabRetrieveTagTask::KolabRetrieveTagTask(const ResourceStateInterface::Ptr &resource, RetrieveType type, QObject *parent)
    : KolabRelationResourceTask(resource, parent)
    , mRetrieveType(type)
{
}

void KolabRetrieveTagTask::startRelationTask(KIMAP::Session *session)
{
    mSession = session;
    const QString mailBox = mailBoxForCollection(relationCollection());

    auto select = new KIMAP::SelectJob(session);
    select->setMailBox(mailBox);
    connect(select, &KJob::result, this, &KolabRetrieveTagTask::onFinalSelectDone);
    select->start();
}

void KolabRetrieveTagTask::onFinalSelectDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    auto select = static_cast<KIMAP::SelectJob *>(job);
    auto fetch = new KIMAP::FetchJob(select->session());

    if (select->messageCount() == 0) {
        taskComplete();
        return;
    }

    KIMAP::ImapSet set;
    set.add(KIMAP::ImapInterval(1, 0));
    fetch->setSequenceSet(set);
    fetch->setUidBased(false);

    KIMAP::FetchJob::FetchScope scope;
    scope.parts.clear();
    scope.mode = KIMAP::FetchJob::FetchScope::Full;
    fetch->setScope(scope);
    connect(fetch, &KIMAP::FetchJob::messagesAvailable, this, &KolabRetrieveTagTask::onMessagesAvailable);
    connect(fetch, &KJob::result, this, &KolabRetrieveTagTask::onHeadersFetchDone);
    fetch->start();
}

void KolabRetrieveTagTask::onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages)
{
    auto fetch = static_cast<KIMAP::FetchJob *>(sender());
    Q_ASSERT(fetch);

    for (auto it = messages.cbegin(), end = messages.cend(); it != end; ++it) {
        if (it->flags.contains(ImapFlags::Deleted)) {
            continue;
        }
        const KMime::Message::Ptr msg = it->message;
        const Kolab::KolabObjectReader reader(msg);
        switch (reader.getType()) {
        case Kolab::RelationConfigurationObject:
            if (mRetrieveType == RetrieveTags && reader.isTag()) {
                extractTag(reader, it->uid);
            } else if (mRetrieveType == RetrieveRelations && reader.isRelation()) {
                extractRelation(reader, it->uid);
            }
            break;

        default:
            break;
        }
    }
}

Akonadi::Item KolabRetrieveTagTask::extractMember(const Kolab::RelationMember &member)
{
    // TODO should we create a dummy item if it isn't yet available?
    Akonadi::Item i;
    if (!member.gid.isEmpty()) {
        // Reference by GID
        i.setGid(member.gid);
    } else {
        // Reference by imap uid
        if (member.uid < 0) {
            return {};
        }
        i.setRemoteId(QString::number(member.uid));
        qCDebug(KOLABRESOURCE_LOG) << "got member: " << member.uid << member.mailbox;
        Akonadi::Collection parent;
        {
            // The root collection is not part of the mailbox path
            Akonadi::Collection col;
            col.setRemoteId(rootRemoteId());
            col.setParentCollection(Akonadi::Collection::root());
            parent = col;
        }
        for (const QByteArray &part : std::as_const(member.mailbox)) {
            Akonadi::Collection col;
            col.setRemoteId(separatorCharacter() + QString::fromLatin1(part));
            col.setParentCollection(parent);
            parent = col;
        }
        i.setParentCollection(parent);
    }
    return i;
}

void KolabRetrieveTagTask::extractTag(const Kolab::KolabObjectReader &reader, qint64 remoteUid)
{
    Akonadi::Tag tag = reader.getTag();
    tag.setRemoteId(QByteArray::number(remoteUid));
    mTags << tag;

    qCDebug(KOLABRESOURCE_TRACE) << "Extracted tag: " << tag.gid() << " remoteId: " << remoteUid << tag.remoteId();

    Akonadi::Item::List members;
    const QStringList lstMemberUrl = reader.getTagMembers();
    for (const QString &memberUrl : lstMemberUrl) {
        Kolab::RelationMember member = Kolab::parseMemberUrl(memberUrl);
        const Akonadi::Item i = extractMember(member);
        // TODO implement fallback to search if uid is not available
        if (!i.remoteId().isEmpty() || !i.gid().isEmpty()) {
            members << i;
        } else {
            qCWarning(KOLABRESOURCE_LOG) << "Failed to parse member: " << memberUrl;
        }
    }
    mTagMembers.insert(QString::fromLatin1(tag.remoteId()), members);
}

void KolabRetrieveTagTask::extractRelation(const Kolab::KolabObjectReader &reader, qint64 remoteUid)
{
    Akonadi::Item::List members;
    const QStringList lstMemberUrl = reader.getTagMembers();
    for (const QString &memberUrl : lstMemberUrl) {
        Kolab::RelationMember member = Kolab::parseMemberUrl(memberUrl);
        const Akonadi::Item i = extractMember(member);
        // TODO implement fallback to search if uid is not available
        if (!i.remoteId().isEmpty() || !i.gid().isEmpty()) {
            members << i;
        } else {
            qCWarning(KOLABRESOURCE_LOG) << "Failed to parse member: " << memberUrl;
        }
    }
    if (members.size() != 2) {
        qCWarning(KOLABRESOURCE_LOG) << "Wrong numbers of members for a relation, expected 2: " << members.size();
        return;
    }

    Akonadi::Relation relation = reader.getRelation();
    relation.setType(Akonadi::Relation::GENERIC);
    relation.setRemoteId(QByteArray::number(remoteUid));
    relation.setLeft(members.at(0));
    relation.setRight(members.at(1));
    mRelations << relation;
}

void KolabRetrieveTagTask::onHeadersFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Fetch job failed " << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    taskComplete();
}

void KolabRetrieveTagTask::taskComplete()
{
    if (mRetrieveType == RetrieveTags) {
        qCDebug(KOLABRESOURCE_LOG) << "Fetched tags: " << mTags.size() << mTagMembers.size();
        resourceState()->tagsRetrieved(mTags, mTagMembers);
    } else if (mRetrieveType == RetrieveRelations) {
        qCDebug(KOLABRESOURCE_LOG) << "Fetched relations:" << mRelations.size();
        resourceState()->relationsRetrieved(mRelations);
    }

    deleteLater();
}
