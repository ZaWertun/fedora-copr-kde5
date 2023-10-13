/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kolabrelationresourcetask.h"
#include <Akonadi/Tag>

namespace Kolab
{
class KolabObjectReader;
struct RelationMember;
} // namespace Kolab

class KolabRetrieveTagTask : public KolabRelationResourceTask
{
    Q_OBJECT
public:
    enum RetrieveType {
        RetrieveTags,
        RetrieveRelations,
    };

    explicit KolabRetrieveTagTask(const ResourceStateInterface::Ptr &resource, RetrieveType type, QObject *parent = nullptr);

protected:
    void startRelationTask(KIMAP::Session *session) override;

private:
    KIMAP::Session *mSession = nullptr;
    Akonadi::Tag::List mTags;
    QHash<QString, Akonadi::Item::List> mTagMembers;
    Akonadi::Relation::List mRelations;
    const RetrieveType mRetrieveType;

private Q_SLOTS:
    void onFinalSelectDone(KJob *job);
    void onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages);
    void onHeadersFetchDone(KJob *job);

private:
    void extractTag(const Kolab::KolabObjectReader &reader, qint64 remoteUid);
    void extractRelation(const Kolab::KolabObjectReader &reader, qint64 remoteUid);
    Akonadi::Item extractMember(const Kolab::RelationMember &member);
    void taskComplete();
};
