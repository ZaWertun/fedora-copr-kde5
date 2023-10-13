/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "tomboyjobbase.h"
#include <QString>
#include <QWebEngineView>

class TomboyServerAuthenticateJob : public TomboyJobBase
{
    Q_OBJECT
public:
    explicit TomboyServerAuthenticateJob(QNetworkAccessManager *manager, QObject *parent = nullptr);

    ~TomboyServerAuthenticateJob() override;

    QString getRequestToken() const;
    QString getRequestTokenSecret() const;
    QString getContentUrl() const;
    QString getUserURL() const;

    void start() override;

private:
    void onLinkingFailed();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);

    void onApiRequestFinished();
    void onUserRequestFinished();
    QString mUserURL;

    QWebEngineView *const mWebView;
};
