/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#include "debug.h"
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2/o0globals.h"
#include "o2/o2.h"
#include "o2/o2requestor.h"
#include <chrono>

using namespace std::chrono_literals;

O2Requestor::O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent)
    : QObject(parent)
    , reply_(nullptr)
    , status_(Idle)
{
    manager_ = manager;
    authenticator_ = authenticator;
    if (authenticator) {
        timedReplies_.setIgnoreSslErrors(authenticator->ignoreSslErrors());
    }
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(authenticator, &O2::refreshFinished, this, &O2Requestor::onRefreshFinished, Qt::QueuedConnection);
}

O2Requestor::~O2Requestor() = default;

int O2Requestor::get(const QNetworkRequest &req)
{
    if (-1 == setup(req, QNetworkAccessManager::GetOperation)) {
        return -1;
    }
    reply_ = manager_->get(request_);
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::finished, this, &O2Requestor::onRequestFinished, Qt::QueuedConnection);
    return id_;
}

int O2Requestor::post(const QNetworkRequest &req, const QByteArray &data)
{
    if (-1 == setup(req, QNetworkAccessManager::PostOperation)) {
        return -1;
    }
    data_ = data;
    reply_ = manager_->post(request_, data_);
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::finished, this, &O2Requestor::onRequestFinished, Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::put(const QNetworkRequest &req, const QByteArray &data)
{
    if (-1 == setup(req, QNetworkAccessManager::PutOperation)) {
        return -1;
    }
    data_ = data;
    reply_ = manager_->put(request_, data_);
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::finished, this, &O2Requestor::onRequestFinished, Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

void O2Requestor::onRefreshFinished(QNetworkReply::NetworkError error)
{
    if (status_ != Requesting) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::onRefreshFinished: No pending request";
        return;
    }
    if (QNetworkReply::NoError == error) {
        QTimer::singleShot(100ms, this, &O2Requestor::retry);
    } else {
        error_ = error;
        QTimer::singleShot(10ms, this, &O2Requestor::finish);
    }
}

void O2Requestor::onRequestFinished()
{
    auto senderReply = qobject_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError error = senderReply->error();
    if (status_ == Idle) {
        return;
    }
    if (reply_ != senderReply) {
        return;
    }
    if (error == QNetworkReply::NoError) {
        QTimer::singleShot(10ms, this, &O2Requestor::finish);
    }
}

void O2Requestor::onRequestError(QNetworkReply::NetworkError error)
{
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::onRequestError: Error" << (int)error;
    if (status_ == Idle) {
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        return;
    }
    int httpStatus = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::onRequestError: HTTP status" << httpStatus
                                       << reply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << reply_->readAll();
    if ((status_ == Requesting) && (httpStatus == 401)) {
        // Call O2::refresh. Note the O2 instance might live in a different thread
        if (QMetaObject::invokeMethod(authenticator_, "refresh")) {
            return;
        }
        qCCritical(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::onRequestError: Invoking remote refresh failed";
    }
    error_ = error;
    QTimer::singleShot(10ms, this, &O2Requestor::finish);
}

void O2Requestor::onUploadProgress(qint64 uploaded, qint64 total)
{
    if (status_ == Idle) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::onUploadProgress: No pending request";
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        return;
    }
    Q_EMIT uploadProgress(id_, uploaded, total);
}

int O2Requestor::setup(const QNetworkRequest &req, QNetworkAccessManager::Operation operation)
{
    static int currentId;
    QUrl url;

    if (status_ != Idle) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::setup: Another request pending";
        return -1;
    }

    request_ = req;
    operation_ = operation;
    id_ = currentId++;
    url_ = url = req.url();
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
#else
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String(O2_OAUTH2_ACCESS_TOKEN), authenticator_->token());
    url.setQuery(query);
#endif
    request_.setUrl(url);
    status_ = Requesting;
    error_ = QNetworkReply::NoError;
    return id_;
}

void O2Requestor::finish()
{
    QByteArray data;
    if (status_ == Idle) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::finish: No pending request";
        return;
    }
    data = reply_->readAll();
    status_ = Idle;
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    Q_EMIT finished(id_, error_, data);
}

void O2Requestor::retry()
{
    if (status_ != Requesting) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2Requestor::retry: No pending request";
        return;
    }
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    QUrl url = url_;
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
#else
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String(O2_OAUTH2_ACCESS_TOKEN), authenticator_->token());
    url.setQuery(query);
#endif
    request_.setUrl(url);
    status_ = ReRequesting;
    switch (operation_) {
    case QNetworkAccessManager::GetOperation:
        reply_ = manager_->get(request_);
        break;
    case QNetworkAccessManager::PostOperation:
        reply_ = manager_->post(request_, data_);
        break;
    default:
        reply_ = manager_->put(request_, data_);
    }
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::finished, this, &O2Requestor::onRequestFinished, Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
}
