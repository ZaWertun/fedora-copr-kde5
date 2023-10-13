/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabchangetagtask.h"
#include "kolabresource_debug.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

KolabChangeTagTask::KolabChangeTagTask(const ResourceStateInterface::Ptr &resource, const QSharedPointer<TagConverter> &tagConverter, QObject *parent)
    : KolabRelationResourceTask(resource, parent)
    , mTagConverter(tagConverter)
{
}

void KolabChangeTagTask::startRelationTask(KIMAP::Session *session)
{
    mSession = session;

    auto fetch = new Akonadi::ItemFetchJob(resourceState()->tag());
    fetch->fetchScope().setCacheOnly(true);
    fetch->fetchScope().setFetchGid(true);
    fetch->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::All);
    fetch->fetchScope().fetchFullPayload(true);
    connect(fetch, &KJob::result, this, &KolabChangeTagTask::onItemsFetchDone);
}

void KolabChangeTagTask::onItemsFetchDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "ItemFetch failed: " << job->errorString();
        cancelTask(job->errorString());
        return;
    }

    const Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob *>(job)->items();

    auto changeHelper = new TagChangeHelper(this);

    connect(changeHelper, &TagChangeHelper::applyCollectionChanges, this, &KolabChangeTagTask::onApplyCollectionChanged);
    connect(changeHelper, &TagChangeHelper::cancelTask, this, &KolabChangeTagTask::onCancelTask);
    connect(changeHelper, &TagChangeHelper::changeCommitted, this, &KolabChangeTagTask::onChangeCommitted);

    changeHelper->start(resourceState()->tag(), mTagConverter->createMessage(resourceState()->tag(), items, resourceState()->userName()), mSession);
}

void KolabChangeTagTask::onApplyCollectionChanged(const Akonadi::Collection &collection)
{
    mRelationCollection = collection;
    applyCollectionChanges(collection);
}

void KolabChangeTagTask::onCancelTask(const QString &errorText)
{
    cancelTask(errorText);
}

void KolabChangeTagTask::onChangeCommitted()
{
    changeProcessed();
}
