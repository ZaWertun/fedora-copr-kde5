/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboyitemsdownloadjob.h"
#include "debug.h"
#include <Akonadi/NoteUtils>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

TomboyItemsDownloadJob::TomboyItemsDownloadJob(Akonadi::Collection::Id id, QNetworkAccessManager *manager, QObject *parent)
    : TomboyJobBase(manager, parent)
    , mCollectionId(id)
{
}

Akonadi::Item::List TomboyItemsDownloadJob::items() const
{
    return mResultItems;
}

void TomboyItemsDownloadJob::start()
{
    // Get all notes
    QNetworkRequest request = QNetworkRequest(QUrl(mContentURL));
    mReply = mRequestor->get(request, QList<O0RequestParameter>());

    connect(mReply, &QNetworkReply::finished, this, &TomboyItemsDownloadJob::onRequestFinished);
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyItemsDownloadJob: Start network request";
}

void TomboyItemsDownloadJob::onRequestFinished()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyItemsDownloadJob: Network request finished";
    checkReplyError();
    if (error() != TomboyJobError::NoError) {
        setErrorText(mReply->errorString());
        emitResult();
        return;
    }

    // Parse received data as JSON
    const QJsonDocument document = QJsonDocument::fromJson(mReply->readAll(), nullptr);

    const QJsonObject jo = document.object();
    const QJsonArray notes = jo[QLatin1String("notes")].toArray();

    for (const auto &note : notes) {
        Akonadi::Item item(Akonadi::NoteUtils::noteMimeType());
        item.setRemoteId(note.toObject()[QLatin1String("guid")].toString());
        mResultItems << item;
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyItemsDownloadJob: Retrieving note with id" << item.remoteId();
    }

    setError(TomboyJobError::NoError);
    emitResult();
}
