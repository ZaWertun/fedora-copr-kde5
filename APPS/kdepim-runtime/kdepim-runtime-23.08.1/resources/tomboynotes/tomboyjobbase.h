/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "o1tomboy.h"
#include "o2/o1requestor.h"
#include <KCompositeJob>
#include <QNetworkAccessManager>
#include <QString>

enum TomboyJobError {
    NoError,
    TemporaryError,
    PermanentError,
};

class TomboyJobBase : public KCompositeJob
{
    Q_OBJECT
public:
    explicit TomboyJobBase(QNetworkAccessManager *manager, QObject *parent = nullptr);

    void setServerURL(const QString &apiurl, const QString &contenturl);
    void setAuthentication(const QString &token, const QString &secret);

protected:
    QNetworkAccessManager *const mManager;
    O1Requestor *mRequestor = nullptr;
    O1Tomboy *const mO1;
    QNetworkReply *mReply = nullptr;

    QString mApiURL;
    QString mContentURL;

    void checkReplyError();
};
