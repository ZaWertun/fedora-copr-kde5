/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboyitemuploadjob.h"
#include "debug.h"
#include <KLocalizedString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

TomboyItemUploadJob::TomboyItemUploadJob(const Akonadi::Item &item, JobType jobType, QNetworkAccessManager *manager, QObject *parent)
    : TomboyJobBase(manager, parent)
    , mSourceItem(item)
    , mJobType(jobType)
{
    mSourceItem = Akonadi::Item(item);
    if (item.hasPayload<KMime::Message::Ptr>()) {
        mNoteContent = item.payload<KMime::Message::Ptr>();
    }

    mRemoteRevision = item.parentCollection().remoteRevision().toInt();

    // Create random remote id if adding new item
    if (jobType == JobType::AddItem) {
        mSourceItem.setRemoteId(QUuid::createUuid().toString());
    }
}

Akonadi::Item TomboyItemUploadJob::item() const
{
    return mSourceItem;
}

JobType TomboyItemUploadJob::jobType() const
{
    return mJobType;
}

void TomboyItemUploadJob::start()
{
    // Convert note to JSON
    QJsonObject jsonNote;
    jsonNote[QLatin1String("guid")] = mSourceItem.remoteId();
    switch (mJobType) {
    case JobType::DeleteItem:
        jsonNote[QLatin1String("command")] = QStringLiteral("delete");
        break;
    case JobType::AddItem:
        jsonNote[QLatin1String("create-date")] = getCurrentISOTime();
        // Missing break is intended
        Q_FALLTHROUGH();
    case JobType::ModifyItem:
        jsonNote[QLatin1String("title")] = mNoteContent->headerByType("subject")->asUnicodeString();
        jsonNote[QLatin1String("note-content")] = mNoteContent->mainBodyPart()->decodedText();
        jsonNote[QLatin1String("note-content-version")] = QStringLiteral("1");
        jsonNote[QLatin1String("last-change-date")] = getCurrentISOTime();
    }

    // Create the full JSON object
    QJsonArray noteChanges;
    noteChanges.append(jsonNote);
    QJsonObject postJson;
    postJson[QLatin1String("note-changes")] = noteChanges;
    postJson[QLatin1String("latest-sync-revision")] = QString::number(++mRemoteRevision);
    QJsonDocument postData;
    postData.setObject(postJson);

    // Network request
    QNetworkRequest request = QNetworkRequest(QUrl(mContentURL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json; boundary=7d44e178b0439"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.toJson().length());
    mReply = mRequestor->put(request, QList<O0RequestParameter>(), postData.toJson());
    connect(mReply, &QNetworkReply::finished, this, &TomboyItemUploadJob::onRequestFinished);
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyItemUploadJob: Start network request";
}

void TomboyItemUploadJob::onRequestFinished()
{
    checkReplyError();
    if (error() != TomboyJobError::NoError) {
        setErrorText(mReply->errorString());
        emitResult();
        return;
    }
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyItemUploadJob: Network request finished. No error occurred";

    // Parse received data as JSON
    const QJsonDocument document = QJsonDocument::fromJson(mReply->readAll(), nullptr);

    const QJsonObject jo = document.object();
    const QJsonArray notes = jo[QLatin1String("notes")].toArray();

    // Check if server status is as expected
    bool found = false;
    for (const auto &note : notes) {
        found = (note.toObject()[QLatin1String("guid")].toString() == mSourceItem.remoteId());
        if (found) {
            break;
        }
    }
    if (mJobType == JobType::DeleteItem && found) {
        setError(TomboyJobError::PermanentError);
        setErrorText(i18n("Sync error. Server status not as expected."));
        emitResult();
        return;
    }
    if (mJobType != JobType::DeleteItem && !found) {
        setError(TomboyJobError::PermanentError);
        setErrorText(i18n("Sync error. Server status not as expected."));
        emitResult();
        return;
    }

    setError(TomboyJobError::NoError);
    emitResult();
}

QString TomboyItemUploadJob::getCurrentISOTime() const
{
    QDateTime local = QDateTime::currentDateTime();
    QDateTime utc = local.toUTC();
    utc.setTimeSpec(Qt::LocalTime);
    int utcOffset = utc.secsTo(local);
    local.setOffsetFromUtc(utcOffset);

    return local.toString(Qt::ISODate);
}
