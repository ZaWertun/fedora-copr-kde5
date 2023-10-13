/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboycollectionsdownloadjob.h"
#include "debug.h"
#include <Akonadi/CachePolicy>
#include <Akonadi/NoteUtils>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

TomboyCollectionsDownloadJob::TomboyCollectionsDownloadJob(const QString &collectionName, QNetworkAccessManager *manager, int refreshInterval, QObject *parent)
    : TomboyJobBase(manager, parent)
    , mCollectionName(collectionName)
    , mRefreshInterval(refreshInterval)
{
}

Akonadi::Collection::List TomboyCollectionsDownloadJob::collections() const
{
    return mResultCollections;
}

void TomboyCollectionsDownloadJob::start()
{
    // Get user information
    QNetworkRequest request = QNetworkRequest(QUrl(mContentURL));
    mReply = mRequestor->get(request, QList<O0RequestParameter>());

    connect(mReply, &QNetworkReply::finished, this, &TomboyCollectionsDownloadJob::onRequestFinished);
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyCollectionsDownloadJob: Start network request";
}

void TomboyCollectionsDownloadJob::onRequestFinished()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyCollectionsDownloadJob: Network request finished";
    checkReplyError();
    if (error() != TomboyJobError::NoError) {
        setErrorText(mReply->errorString());
        emitResult();
        return;
    }

    // Parse received data as JSON
    const QJsonDocument document = QJsonDocument::fromJson(mReply->readAll(), nullptr);

    const QJsonObject jo = document.object();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyCollectionsDownloadJob: " << jo;
    const QJsonValue collectionRevision = jo[QLatin1String("latest-sync-revision")];
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyCollectionsDownloadJob: " << collectionRevision;

    Akonadi::Collection c;
    c.setParentCollection(Akonadi::Collection::root());
    c.setRemoteId(mContentURL);
    c.setName(mCollectionName);
    c.setRemoteRevision(QString::number(collectionRevision.toInt()));
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyCollectionsDownloadJob: Sync revision " << collectionRevision.toString();

    Akonadi::CachePolicy cachePolicy;
    cachePolicy.setInheritFromParent(false);
    cachePolicy.setSyncOnDemand(false);
    cachePolicy.setCacheTimeout(-1);
    cachePolicy.setIntervalCheckTime(mRefreshInterval);
    cachePolicy.setLocalParts(QStringList() << QStringLiteral("ALL"));
    c.setCachePolicy(cachePolicy);

    c.setContentMimeTypes({Akonadi::NoteUtils::noteMimeType()});

    mResultCollections << c;

    emitResult();
}
