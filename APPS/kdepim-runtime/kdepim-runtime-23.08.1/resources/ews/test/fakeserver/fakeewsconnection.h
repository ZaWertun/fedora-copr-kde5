/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "fakeewsserver.h"

class QTcpSocket;

class FakeEwsConnection : public QObject
{
    Q_OBJECT
public:
    FakeEwsConnection(QTcpSocket *sock, FakeEwsServer *parent);
    ~FakeEwsConnection() override;
    void sendEvents(const QStringList &events);
private Q_SLOTS:
    void disconnected();
    void dataAvailable();
    void dataTimeout();
    void streamingRequestTimeout();
    void streamingRequestHeartbeat();
Q_SIGNALS:
    void streamingRequestStarted(FakeEwsConnection *conn);

private:
    void sendError(const QString &msg, ushort code = 500);
    FakeEwsServer::DialogEntry::HttpResponse parseRequest(const QString &content);
    FakeEwsServer::DialogEntry::HttpResponse handleGetEventsRequest(const QString &content);
    FakeEwsServer::DialogEntry::HttpResponse handleGetStreamingEventsRequest(const QString &content);
    QString prepareEventsResponse(const QStringList &events);

    enum HttpConnectionState {
        Initial,
        RequestReceived,
        HeadersReceived,
    };

    QPointer<QTcpSocket> mSock;
    uint mContentLength;
    QByteArray mContent;
    QTimer mDataTimer;
    bool mKeepAlive;
    QTimer mStreamingRequestHeartbeat;
    QTimer mStreamingRequestTimeout;
    QString mStreamingSubId;
    HttpConnectionState mState;
    bool mAuthenticated;
};
