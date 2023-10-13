/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsglobaltagsreadjob.h"

#include "ewsclient.h"
#include "ewsgetfolderrequest.h"
#include "ewsresource.h"
#include "ewstagstore.h"

#include "ewsresource_debug.h"

using namespace Akonadi;

EwsGlobalTagsReadJob::EwsGlobalTagsReadJob(EwsTagStore *tagStore, EwsClient &client, const Collection &rootCollection, QObject *parent)
    : EwsJob(parent)
    , mTagStore(tagStore)
    , mClient(client)
    , mRootCollection(rootCollection)
{
}

EwsGlobalTagsReadJob::~EwsGlobalTagsReadJob() = default;

void EwsGlobalTagsReadJob::start()
{
    auto req = new EwsGetFolderRequest(mClient, this);
    req->setFolderIds(EwsId::List() << EwsId(EwsDIdMsgFolderRoot));
    EwsFolderShape shape(EwsShapeIdOnly);
    shape << EwsResource::globalTagsProperty << EwsResource::globalTagsVersionProperty;
    req->setFolderShape(shape);
    connect(req, &EwsGetFolderRequest::result, this, &EwsGlobalTagsReadJob::getFolderRequestFinished);
    req->start();
}

void EwsGlobalTagsReadJob::getFolderRequestFinished(KJob *job)
{
    auto req = qobject_cast<EwsGetFolderRequest *>(job);

    if (!req) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid EwsGetFolderRequest job object");
        setErrorMsg(QStringLiteral("Invalid EwsGetFolderRequest job object"));
        emitResult();
        return;
    }

    if (req->error()) {
        setErrorMsg(req->errorString());
        emitResult();
        return;
    }

    if (req->responses().size() != 1) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Invalid number of responses received");
        setErrorMsg(QStringLiteral("Invalid number of responses received"));
        emitResult();
        return;
    }

    EwsFolder folder = req->responses().first().folder();
    bool status = mTagStore->readTags(folder[EwsResource::globalTagsProperty].toStringList(), folder[EwsResource::globalTagsVersionProperty].toInt());
    if (!status) {
        qCWarning(EWSRES_LOG) << QStringLiteral("Incorrect server tag data");
        setErrorMsg(QStringLiteral("Incorrect server tag data"));
        emitResult();
    }

    mTags = mTagStore->tags();

    emitResult();
}
