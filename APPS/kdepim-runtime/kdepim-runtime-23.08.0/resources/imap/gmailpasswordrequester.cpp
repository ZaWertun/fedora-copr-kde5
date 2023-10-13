/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gmailpasswordrequester.h"
#include "imapresourcebase.h"
#include "settings.h"

#include <KGAPI/Account>
#include <KGAPI/AccountManager>
#include <KGAPI/AuthJob>

#define GOOGLE_API_KEY QStringLiteral("554041944266.apps.googleusercontent.com")
#define GOOGLE_API_SECRET QStringLiteral("mdT1DjzohxN3npUUzkENT0gO")

GmailPasswordRequester::GmailPasswordRequester(ImapResourceBase *resource, QObject *parent)
    : PasswordRequesterInterface(parent)
    , mResource(resource)
{
}

GmailPasswordRequester::~GmailPasswordRequester() = default;

void GmailPasswordRequester::requestPassword(RequestType request, const QString &serverError)
{
    Q_UNUSED(serverError) // we don't get anything useful from XOAUTH2 SASL

    if (request == WrongPasswordRequest) {
        auto promise = KGAPI2::AccountManager::instance()->findAccount(GOOGLE_API_KEY, mResource->settings()->userName());
        connect(promise, &KGAPI2::AccountPromise::finished, this, [this](KGAPI2::AccountPromise *promise) {
            if (promise->account()) {
                promise = KGAPI2::AccountManager::instance()->refreshTokens(GOOGLE_API_KEY, GOOGLE_API_SECRET, mResource->settings()->userName());
            } else {
                promise = KGAPI2::AccountManager::instance()->getAccount(GOOGLE_API_KEY,
                                                                         GOOGLE_API_SECRET,
                                                                         mResource->settings()->userName(),
                                                                         {KGAPI2::Account::mailScopeUrl()});
            }
            connect(promise, &KGAPI2::AccountPromise::finished, this, &GmailPasswordRequester::onTokenRequestFinished);
            mPendingPromise = promise;
        });
        mPendingPromise = promise;
    } else {
        auto promise = KGAPI2::AccountManager::instance()->getAccount(GOOGLE_API_KEY,
                                                                      GOOGLE_API_SECRET,
                                                                      mResource->settings()->userName(),
                                                                      {KGAPI2::Account::mailScopeUrl()});
        connect(promise, &KGAPI2::AccountPromise::finished, this, &GmailPasswordRequester::onTokenRequestFinished);
        mPendingPromise = promise;
    }
}

void GmailPasswordRequester::cancelPasswordRequests()
{
    if (mPendingPromise) {
        mPendingPromise->disconnect(this);
    }
}

void GmailPasswordRequester::onTokenRequestFinished(KGAPI2::AccountPromise *promise)
{
    mPendingPromise = nullptr;
    if (promise->hasError()) {
        Q_EMIT done(UserRejected, promise->errorText());
    } else {
        Q_EMIT done(PasswordRetrieved, promise->account()->accessToken());
    }
}
