/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsupdateitemstagsjob.h"

#include <Akonadi/AttributeFactory>
#include <Akonadi/Tag>
#include <Akonadi/TagAttribute>
#include <Akonadi/TagFetchJob>
#include <Akonadi/TagFetchScope>

#include "ewsclient.h"
#include "ewsglobaltagswritejob.h"
#include "ewsid.h"
#include "ewsitemhandler.h"
#include "ewsresource.h"
#include "ewstagstore.h"

using namespace Akonadi;

constexpr unsigned ChunkSize = 10;

EwsUpdateItemsTagsJob::EwsUpdateItemsTagsJob(const Akonadi::Item::List &items, EwsTagStore *tagStore, EwsClient &client, EwsResource *parent)
    : EwsJob(parent)
    , mItems(items)
    , mTagStore(tagStore)
    , mClient(client)
    , mChunkedJob(ChunkSize)
{
}

EwsUpdateItemsTagsJob::~EwsUpdateItemsTagsJob() = default;

void EwsUpdateItemsTagsJob::start()
{
    Tag::List unknownTags;

    /* In the Exchange world it is not possible to add or remove individual tags from an item
     * - it is necessary to write the full list of tags at once. Unfortunately the tags objects
     * attached to an item only contain the id. They're missing any persistent identification such
     * as the uid. If the EWS resource hasn't seen these tags yet it is necessary to fetch them
     * first before any further processing.
     */
    for (const Item &item : std::as_const(mItems)) {
        const auto tags{item.tags()};
        for (const Tag &tag : tags) {
            if (!mTagStore->containsId(tag.id())) {
                unknownTags.append(tag);
            }
        }
    }

    if (!unknownTags.empty()) {
        auto job = new TagFetchJob(unknownTags, this);
        job->fetchScope().setFetchRemoteId(true);
        connect(job, &TagFetchJob::result, this, &EwsUpdateItemsTagsJob::itemsTagsChangedTagsFetched);
    } else {
        doUpdateItemsTags();
    }
}

void EwsUpdateItemsTagsJob::itemsTagsChangedTagsFetched(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorString());
        emitResult();
        return;
    }

    auto tagJob = qobject_cast<TagFetchJob *>(job);
    if (!tagJob) {
        setErrorMsg(QStringLiteral("Invalid TagFetchJob job object"));
        emitResult();
        return;
    }

    /* All unknown tags have been fetched and can be written to Exchange. */
    mTagStore->addTags(tagJob->tags());

    auto res = qobject_cast<EwsResource *>(parent());
    Q_ASSERT(res);
    auto writeJob = new EwsGlobalTagsWriteJob(mTagStore, mClient, res->rootCollection(), this);
    connect(writeJob, &EwsGlobalTagsWriteJob::result, this, &EwsUpdateItemsTagsJob::globalTagsWriteFinished);
    writeJob->start();
}

void EwsUpdateItemsTagsJob::globalTagsWriteFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorString());
        emitResult();
        return;
    }

    doUpdateItemsTags();
}

void EwsUpdateItemsTagsJob::doUpdateItemsTags()
{
    QVector<EwsUpdateItemRequest::ItemChange> itemChanges;
    itemChanges.reserve(mItems.size());

    for (const Item &item : std::as_const(mItems)) {
        EwsUpdateItemRequest::ItemChange ic(EwsId(item.remoteId(), item.remoteRevision()), EwsItemHandler::mimeToItemType(item.mimeType()));
        if (!item.tags().isEmpty()) {
            QStringList tagList;
            QStringList categoryList;
            tagList.reserve(item.tags().count());
            const auto tags{item.tags()};
            for (const Tag &tag : tags) {
                Q_ASSERT(mTagStore->containsId(tag.id()));
                tagList.append(QString::fromLatin1(mTagStore->tagRemoteId(tag.id())));
                QString name = mTagStore->tagName(tag.id());
                if (!name.isEmpty()) {
                    categoryList.append(name);
                }
            }
            EwsUpdateItemRequest::Update *upd = new EwsUpdateItemRequest::SetUpdate(EwsResource::tagsProperty, tagList);
            ic.addUpdate(upd);
            upd = new EwsUpdateItemRequest::SetUpdate(EwsPropertyField(QStringLiteral("item:Categories")), categoryList);
            ic.addUpdate(upd);
        } else {
            EwsUpdateItemRequest::Update *upd = new EwsUpdateItemRequest::DeleteUpdate(EwsResource::tagsProperty);
            ic.addUpdate(upd);
            upd = new EwsUpdateItemRequest::DeleteUpdate(EwsPropertyField(QStringLiteral("item:Categories")));
            ic.addUpdate(upd);
        }
        itemChanges.append(ic);
    }

    mChunkedJob.setItems(itemChanges);
    mChunkedJob.start(
        [this](EwsUpdateItemRequest::ItemChange::List::const_iterator firstChange, EwsUpdateItemRequest::ItemChange::List::const_iterator lastChange) {
            auto req = new EwsUpdateItemRequest(mClient, this);
            req->addItemChanges(firstChange, lastChange);
            return req;
        },
        [](EwsUpdateItemRequest *req) {
            return req->responses();
        },
        [this](unsigned int progress) {
            Q_EMIT reportPercent(progress);
        },
        [this](bool success, const QString &error) {
            updateItemsTagsRequestFinished(success, error);
        });
}

void EwsUpdateItemsTagsJob::updateItemsTagsRequestFinished(bool success, const QString &error)
{
    if (!success) {
        setErrorMsg(error);
        emitResult();
        return;
    }

    auto responses = mChunkedJob.responses();
    Q_ASSERT(mItems.count() == responses.count());

    auto itemIt = mItems.begin();
    for (const auto &resp : responses) {
        if (resp.isSuccess()) {
            itemIt->setRemoteRevision(resp.itemId().changeKey());
        }
        ++itemIt;
    }

    emitResult();
}
