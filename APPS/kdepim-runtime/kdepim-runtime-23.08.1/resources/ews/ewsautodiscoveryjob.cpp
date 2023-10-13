/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsautodiscoveryjob.h"

#include "ewspoxautodiscoverrequest.h"
#include "ewsresource_debug.h"

#include <KLocalizedString>

EwsAutodiscoveryJob::EwsAutodiscoveryJob(const QString &email,
                                         const QString &username,
                                         const QString &password,
                                         const QString &userAgent,
                                         bool enableNTLMv2,
                                         QObject *parent)
    : EwsJob(parent)
    , mEmail(email)
    , mUsername(username)
    , mPassword(password)
    , mUserAgent(userAgent)
    , mEnableNTLMv2(enableNTLMv2)
{
}

EwsAutodiscoveryJob::~EwsAutodiscoveryJob() = default;

void EwsAutodiscoveryJob::start()
{
    parseEmail();

    if (!mUrlQueue.isEmpty()) {
        sendNextRequest(false);
    }
}

void EwsAutodiscoveryJob::parseEmail()
{
    const int atIndex = mEmail.indexOf(QLatin1Char('@'));
    if (atIndex < 0) {
        setErrorMsg(i18n("Incorrect email address"));
        emitResult();
        return;
    }

    const QString domain = mEmail.mid(atIndex + 1);
    if (domain.isEmpty()) {
        setErrorMsg(i18n("Incorrect email address"));
        emitResult();
        return;
    }

    addUrls(domain);
}

void EwsAutodiscoveryJob::addUrls(const QString &domain)
{
    mUrlQueue.enqueue(QStringLiteral("https://") + domain + QStringLiteral("/autodiscover/autodiscover.xml"));
    mUrlQueue.enqueue(QStringLiteral("https://autodiscover.") + domain + QStringLiteral("/autodiscover/autodiscover.xml"));
    mUrlQueue.enqueue(QStringLiteral("http://") + domain + QStringLiteral("/autodiscover/autodiscover.xml"));
    mUrlQueue.enqueue(QStringLiteral("http://autodiscover.") + domain + QStringLiteral("/autodiscover/autodiscover.xml"));
}

void EwsAutodiscoveryJob::sendNextRequest(bool useCreds)
{
    QUrl url(mUrlQueue.head());
    if (useCreds) {
        url.setUserName(mUsername);
        url.setPassword(mPassword);
    }
    mUsedCreds = useCreds;
    auto req = new EwsPoxAutodiscoverRequest(url, mEmail, mUserAgent, mEnableNTLMv2, this);
    connect(req, &EwsPoxAutodiscoverRequest::result, this, &EwsAutodiscoveryJob::autodiscoveryRequestFinished);
    req->start();
}

void EwsAutodiscoveryJob::autodiscoveryRequestFinished(KJob *job)
{
    auto req = qobject_cast<EwsPoxAutodiscoverRequest *>(job);
    if (!req) {
        setErrorMsg(QStringLiteral("Invalid EwsPoxAutodiscoverRequest job object"));
        emitResult();
        return;
    }

    if (req->error()) {
        if (req->error() == 401 && !mUsedCreds && req->lastHttpUrl().scheme() != QLatin1String("http")) { // Don't try authentication over HTTP
            /* The 401 error may have come from an URL different to the original one (due to
             * redirections). When the original URL is retried with credentials KIO HTTP will issue
             * a warning that an authenticated request is made to a host that never asked for it.
             * To fix this restart the request using the last URL that resulted in the 401 code. */
            mUrlQueue.head() = req->lastHttpUrl().toString();
            sendNextRequest(true);
            return;
        } else {
            mUrlQueue.removeFirst();
        }

        if (mUrlQueue.isEmpty()) {
            setErrorText(job->errorText());
            setError(job->error());
            emitResult();
        } else {
            sendNextRequest(false);
        }
    } else {
        switch (req->action()) {
        case EwsPoxAutodiscoverRequest::Settings: {
            EwsPoxAutodiscoverRequest::Protocol proto = req->protocol(EwsPoxAutodiscoverRequest::ExchangeProto);
            if (!proto.isValid()) {
                setErrorMsg(i18n("Exchange protocol information not found"));
            } else {
                mEwsUrl = proto.ewsUrl();
                mOabUrl = proto.oabUrl();
            }
            emitResult();
            break;
        }
        case EwsPoxAutodiscoverRequest::RedirectAddr:
            qCDebug(EWSRES_LOG) << "Redirected to e-mail addr" << req->redirectAddr();
            mEmail = req->redirectAddr();
            mUrlQueue.clear();
            parseEmail();
            if (!mUrlQueue.isEmpty()) {
                sendNextRequest(false);
            }
            break;
        case EwsPoxAutodiscoverRequest::RedirectUrl:
            qCDebug(EWSRES_LOG) << "Redirected to URL" << req->redirectUrl();
            mUrlQueue.clear();
            mUrlQueue.enqueue(req->redirectUrl());
            sendNextRequest(false);
            break;
        }
    }
}
