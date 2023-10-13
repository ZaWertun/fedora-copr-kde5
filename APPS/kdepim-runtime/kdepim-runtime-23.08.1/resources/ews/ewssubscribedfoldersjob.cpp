/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewssubscribedfoldersjob.h"

#include "ewsclient.h"
#include "ewsgetfolderrequest.h"
#include "ewsresource_debug.h"
#include "ewssettings.h"

EwsSubscribedFoldersJob::EwsSubscribedFoldersJob(EwsClient &client, EwsSettings *settings, QObject *parent)
    : EwsJob(parent)
    , mClient(client)
    , mSettings(settings)
{
}

EwsSubscribedFoldersJob::~EwsSubscribedFoldersJob() = default;

void EwsSubscribedFoldersJob::start()
{
    EwsId::List ids;

    // Before subscribing make sure the subscription list doesn't contain invalid folders.
    // Do this also for the default list in order to transform the distinguished IDs into real ones.
    const auto serverSubscriptionList = mSettings->serverSubscriptionList();
    if (serverSubscriptionList == QStringList() << QStringLiteral("default")) {
        ids = defaultSubscriptionFolders();
    } else {
        ids.reserve(serverSubscriptionList.count());
        for (const QString &id : serverSubscriptionList) {
            ids << EwsId(id);
        }
    }

    auto req = new EwsGetFolderRequest(mClient, this);
    req->setFolderShape(EwsFolderShape(EwsShapeIdOnly));
    req->setFolderIds(ids);
    req->setProperty("ids", QVariant::fromValue<EwsId::List>(ids));
    connect(req, &EwsRequest::result, this, &EwsSubscribedFoldersJob::verifySubFoldersRequestFinished);
    req->start();
}

void EwsSubscribedFoldersJob::verifySubFoldersRequestFinished(KJob *job)
{
    if (!job->error()) {
        auto req = qobject_cast<EwsGetFolderRequest *>(job);
        Q_ASSERT(req);

        mFolders.clear();
        auto sourceIds = req->property("ids").value<EwsId::List>();
        QStringList idList;

        Q_ASSERT(req->responses().size() == sourceIds.size());

        auto it = sourceIds.cbegin();

        const auto responses{req->responses()};
        for (const EwsGetFolderRequest::Response &resp : responses) {
            if (resp.isSuccess()) {
                // Take just the id without the change key as the actual folder version is irrelevant
                // here
                const QString id = resp.folder()[EwsFolderFieldFolderId].value<EwsId>().id();
                mFolders << EwsId(id);
                idList << id;
            } else {
                qCWarningNC(EWSRES_LOG) << QStringLiteral("Invalid folder %1 - skipping").arg(it->id());
            }
            it++;
        }

        // Once verified write the final list back to the configuration.
        mSettings->setServerSubscriptionList(idList);
    } else {
        setErrorMsg(job->errorString(), job->error());
    }
    emitResult();
}

const EwsId::List &EwsSubscribedFoldersJob::defaultSubscriptionFolders()
{
    static const EwsId::List list = {
        EwsId(EwsDIdInbox),
        EwsId(EwsDIdCalendar),
        EwsId(EwsDIdTasks),
        EwsId(EwsDIdContacts),
    };

    return list;
}
