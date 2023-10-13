/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboyserverauthenticatejob.h"
#include "debug.h"
#include <KLocalizedString>
#include <QJsonDocument>
#include <QJsonObject>

TomboyServerAuthenticateJob::TomboyServerAuthenticateJob(QNetworkAccessManager *manager, QObject *parent)
    : TomboyJobBase(manager, parent)
    , mWebView(new QWebEngineView((QWidget *)nullptr))
{
    mWebView->setContextMenuPolicy(Qt::NoContextMenu);
    // Connect the o2 authenfication signals
    connect(mO1, &O1::linkingFailed, this, &TomboyServerAuthenticateJob::onLinkingFailed);
    connect(mO1, &O1::linkingSucceeded, this, &TomboyServerAuthenticateJob::onLinkingSucceeded);
    connect(mO1, &O1::openBrowser, this, &TomboyServerAuthenticateJob::onOpenBrowser);
    connect(mO1, &O1::closeBrowser, mWebView, &QWebEngineView::close);
}

TomboyServerAuthenticateJob::~TomboyServerAuthenticateJob()
{
    delete mWebView;
}

void TomboyServerAuthenticateJob::start()
{
    mO1->link();
}

QString TomboyServerAuthenticateJob::getRequestToken() const
{
    return mO1->getRequestToken();
}

QString TomboyServerAuthenticateJob::getRequestTokenSecret() const
{
    return mO1->getRequestTokenSecret();
}

QString TomboyServerAuthenticateJob::getContentUrl() const
{
    return mContentURL;
}

QString TomboyServerAuthenticateJob::getUserURL() const
{
    return mUserURL;
}

void TomboyServerAuthenticateJob::onLinkingFailed()
{
    setError(TomboyJobError::PermanentError);
    setErrorText(i18n("Authorization failed. It could be an SSL error."));
    emitResult();
}

void TomboyServerAuthenticateJob::onLinkingSucceeded()
{
    QNetworkRequest request = QNetworkRequest(QUrl(mApiURL));
    mReply = mRequestor->get(request, QList<O0RequestParameter>());

    connect(mReply, &QNetworkReply::finished, this, &TomboyServerAuthenticateJob::onApiRequestFinished);
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyServerAuthenticateJob: Start network request";
}

void TomboyServerAuthenticateJob::onOpenBrowser(const QUrl &url)
{
    mWebView->setUrl(url);
    mWebView->show();
}

void TomboyServerAuthenticateJob::onApiRequestFinished()
{
    checkReplyError();
    if (error() != TomboyJobError::NoError) {
        setErrorText(mReply->errorString());
        emitResult();
        return;
    }

    // Parse received data as JSON and get user-href
    const QJsonDocument document = QJsonDocument::fromJson(mReply->readAll(), nullptr);
    const QJsonObject jo = document.object();
    mUserURL = jo[QLatin1String("user-ref")].toObject()[QLatin1String("api-ref")].toString();

    QNetworkRequest request = QNetworkRequest(QUrl(mUserURL));
    mReply = mRequestor->get(request, QList<O0RequestParameter>());

    connect(mReply, &QNetworkReply::finished, this, &TomboyServerAuthenticateJob::onUserRequestFinished);
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "TomboyServerAuthenticateJob: Start network request";
}

void TomboyServerAuthenticateJob::onUserRequestFinished()
{
    checkReplyError();
    if (error() != TomboyJobError::NoError) {
        setErrorText(mReply->errorString());
        emitResult();
        return;
    }

    // Parse received data as JSON and get contentURL
    QJsonDocument document = QJsonDocument::fromJson(mReply->readAll(), nullptr);
    QJsonObject jo = document.object();
    mContentURL = jo[QLatin1String("notes-ref")].toObject()[QLatin1String("api-ref")].toString();

    setError(TomboyJobError::NoError);
    emitResult();
}
