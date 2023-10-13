/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsglobaltagswritejob.h"

#include <Akonadi/Collection>

#include "ewsid.h"
#include "ewsresource.h"
#include "ewstagstore.h"
#include "ewsupdatefolderrequest.h"

using namespace Akonadi;

EwsGlobalTagsWriteJob::EwsGlobalTagsWriteJob(EwsTagStore *tagStore, EwsClient &client, const Collection &rootCollection, QObject *parent)
    : EwsJob(parent)
    , mTagStore(tagStore)
    , mClient(client)
    , mRootCollection(rootCollection)
{
}

EwsGlobalTagsWriteJob::~EwsGlobalTagsWriteJob() = default;

void EwsGlobalTagsWriteJob::start()
{
    QStringList tagList = mTagStore->serialize();

    auto req = new EwsUpdateFolderRequest(mClient, this);
    EwsUpdateFolderRequest::FolderChange fc(EwsId(mRootCollection.remoteId(), mRootCollection.remoteRevision()), EwsFolderTypeMail);
    EwsUpdateFolderRequest::Update *upd = new EwsUpdateFolderRequest::SetUpdate(EwsResource::globalTagsProperty, tagList);
    fc.addUpdate(upd);
    upd = new EwsUpdateFolderRequest::SetUpdate(EwsResource::globalTagsVersionProperty, QString::number(mTagStore->version()));
    fc.addUpdate(upd);
    req->addFolderChange(fc);
    connect(req, &EwsUpdateFolderRequest::result, this, &EwsGlobalTagsWriteJob::updateFolderRequestFinished);
    req->start();
}

void EwsGlobalTagsWriteJob::updateFolderRequestFinished(KJob *job)
{
    if (job->error()) {
        setErrorMsg(job->errorText());
    }

    emitResult();
}
