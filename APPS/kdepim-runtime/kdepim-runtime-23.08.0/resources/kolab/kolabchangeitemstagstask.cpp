/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabchangeitemstagstask.h"
#include "kolabresource_debug.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/TagFetchJob>
#include <Akonadi/TagFetchScope>

KolabChangeItemsTagsTask::KolabChangeItemsTagsTask(const ResourceStateInterface::Ptr &resource,
                                                   const QSharedPointer<TagConverter> &tagConverter,
                                                   QObject *parent)
    : KolabRelationResourceTask(resource, parent)
    , mTagConverter(tagConverter)
{
}

void KolabChangeItemsTagsTask::startRelationTask(KIMAP::Session *session)
{
    mSession = session;

    // It's entirely possible that we don't have an rid yet

    // compile a set of changed tags
    const auto addedTags{resourceState()->addedTags()};
    for (const Akonadi::Tag &tag : addedTags) {
        mChangedTags.append(tag);
    }
    const auto removedTags{resourceState()->removedTags()};
    for (const Akonadi::Tag &tag : removedTags) {
        mChangedTags.append(tag);
    }
    qCDebug(KOLABRESOURCE_LOG) << mChangedTags;

    processNextTag();
}

void KolabChangeItemsTagsTask::processNextTag()
{
    if (mChangedTags.isEmpty()) {
        changeProcessed();
        return;
    }

    // "take first"
    const Akonadi::Tag tag = mChangedTags.takeFirst();

    // We have to fetch it again in case it changed since the notification was emitted (which is likely)
    // Otherwise we get an empty remoteid for new tags that were immediately applied on an item
    auto fetch = new Akonadi::TagFetchJob(tag);
    fetch->fetchScope().setFetchRemoteId(true);
    connect(fetch, &KJob::result, this, &KolabChangeItemsTagsTask::onTagFetchDone);
}

void KolabChangeItemsTagsTask::onTagFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "TagFetch failed: " << job->errorString();
        // TODO: we could continue for the other tags?
        cancelTask(job->errorString());
        return;
    }

    const Akonadi::Tag::List tags = static_cast<Akonadi::TagFetchJob *>(job)->tags();
    if (tags.size() != 1) {
        qCWarning(KOLABRESOURCE_LOG) << "Invalid number of tags retrieved: " << tags.size();
        // TODO: we could continue for the other tags?
        cancelTask(job->errorString());
        return;
    }

    auto fetch = new Akonadi::ItemFetchJob(tags.first());
    fetch->fetchScope().setCacheOnly(true);
    fetch->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::All);
    fetch->fetchScope().setFetchGid(true);
    fetch->fetchScope().fetchFullPayload(true);
    fetch->setProperty("tag", QVariant::fromValue(tags.first()));
    connect(fetch, &KJob::result, this, &KolabChangeItemsTagsTask::onItemsFetchDone);
}

void KolabChangeItemsTagsTask::onItemsFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "ItemFetch failed: " << job->errorString();
        // TODO: we could continue for the other tags?
        cancelTask(job->errorString());
        return;
    }

    auto changeHelper = new TagChangeHelper(this);

    connect(changeHelper, &TagChangeHelper::applyCollectionChanges, this, &KolabChangeItemsTagsTask::onApplyCollectionChanged);
    connect(changeHelper, &TagChangeHelper::cancelTask, this, &KolabChangeItemsTagsTask::onCancelTask);
    connect(changeHelper, &TagChangeHelper::changeCommitted, this, &KolabChangeItemsTagsTask::onChangeCommitted);

    const Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob *>(job)->items();
    qCDebug(KOLABRESOURCE_LOG) << items.size();
    const auto tag = job->property("tag").value<Akonadi::Tag>();
    {
        qCDebug(KOLABRESOURCE_LOG) << "Writing " << tag.name() << " with " << items.size() << " members to the server: ";
        for (const Akonadi::Item &item : items) {
            qCDebug(KOLABRESOURCE_LOG) << "member(localid, remoteid): " << item.id() << item.remoteId();
        }
    }
    Q_ASSERT(tag.isValid());
    changeHelper->start(tag, mTagConverter->createMessage(tag, items, resourceState()->userName()), mSession);
}

void KolabChangeItemsTagsTask::onApplyCollectionChanged(const Akonadi::Collection &collection)
{
    mRelationCollection = collection;
    applyCollectionChanges(collection);
}

void KolabChangeItemsTagsTask::onCancelTask(const QString &errorText)
{
    // TODO: we could continue for the other tags?
    cancelTask(errorText);
}

void KolabChangeItemsTagsTask::onChangeCommitted()
{
    processNextTag();
}
