/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tomboyjobbase.h"

TomboyJobBase::TomboyJobBase(QNetworkAccessManager *manager, QObject *parent)
    : KCompositeJob(parent)
    , mManager(manager)
    , mO1(new O1Tomboy(this))
{
    mRequestor = new O1Requestor(mManager, mO1, this);
}

void TomboyJobBase::setServerURL(const QString &apiurl, const QString &contenturl)
{
    mO1->setBaseURL(apiurl);
    mApiURL = apiurl + QStringLiteral("/api/1.0");
    mContentURL = contenturl;
}

void TomboyJobBase::setAuthentication(const QString &token, const QString &secret)
{
    mO1->restoreAuthData(token, secret);
}

void TomboyJobBase::checkReplyError()
{
    switch (mReply->error()) {
    case QNetworkReply::NoError:
        setError(TomboyJobError::NoError);
        break;
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
        setError(TomboyJobError::TemporaryError);
        break;
    default:
        setError(TomboyJobError::PermanentError);
        break;
    }
}
