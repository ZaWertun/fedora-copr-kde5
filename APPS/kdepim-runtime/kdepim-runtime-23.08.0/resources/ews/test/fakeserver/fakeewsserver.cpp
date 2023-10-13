/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fakeewsserver.h"

#include "fakeewsconnection.h"
#include "fakeewsserver_debug.h"
#include <QRandomGenerator>
#include <QThread>

const FakeEwsServer::DialogEntry::HttpResponse FakeEwsServer::EmptyResponse = {QString(), 0};

FakeEwsServer::FakeEwsServer(QObject *parent)
    : QTcpServer(parent)
    , mPortNumber(0)
{
}

FakeEwsServer::~FakeEwsServer()
{
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Stopping fake EWS server.");
}

bool FakeEwsServer::start()
{
    QMutexLocker lock(&mMutex);

    int retries = 3;
    bool ok;
    auto generator = QRandomGenerator::global();
    do {
        mPortNumber = (generator->bounded(10000)) + 10000;
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Starting fake EWS server at 127.0.0.1:%1").arg(mPortNumber);
        ok = listen(QHostAddress::LocalHost, mPortNumber);
        if (!ok) {
            qCWarningNC(EWSFAKE_LOG) << QStringLiteral("Failed to start server");
        }
    } while (!ok && --retries);

    if (ok) {
        connect(this, &QTcpServer::newConnection, this, &FakeEwsServer::newConnectionReceived);
    }

    return ok;
}

void FakeEwsServer::setDialog(const DialogEntry::List &dialog)
{
    QMutexLocker lock(&mMutex);

    mDialog = dialog;
}

void FakeEwsServer::setDefaultReplyCallback(const DialogEntry::ReplyCallback &defaultReplyCallback)
{
    QMutexLocker lock(&mMutex);

    mDefaultReplyCallback = defaultReplyCallback;
}

void FakeEwsServer::setOverrideReplyCallback(const DialogEntry::ReplyCallback &overrideReplyCallback)
{
    QMutexLocker lock(&mMutex);

    mOverrideReplyCallback = overrideReplyCallback;
}

void FakeEwsServer::queueEventsXml(const QStringList &events)
{
    if (QThread::currentThread() != thread()) {
        qCWarningNC(EWSFAKE_LOG) << QStringLiteral(
            "queueEventsXml called from wrong thread "
            "(called from ") << QThread::currentThread()
                                 << QStringLiteral(", should be ") << thread() << QStringLiteral(")");
        return;
    }
    mEventQueue += events;

    if (mStreamingEventsConnection) {
        mStreamingEventsConnection->sendEvents(mEventQueue);
        mEventQueue.clear();
    }
}

QStringList FakeEwsServer::retrieveEventsXml()
{
    QStringList events = mEventQueue;
    mEventQueue.clear();
    return events;
}

void FakeEwsServer::newConnectionReceived()
{
    QTcpSocket *sock = nextPendingConnection();

    auto conn = new FakeEwsConnection(sock, this);
    connect(conn, &FakeEwsConnection::streamingRequestStarted, this, &FakeEwsServer::streamingConnectionStarted);
}

const FakeEwsServer::DialogEntry::List FakeEwsServer::dialog() const
{
    QMutexLocker lock(&mMutex);

    return mDialog;
}

const FakeEwsServer::DialogEntry::ReplyCallback FakeEwsServer::defaultReplyCallback() const
{
    QMutexLocker lock(&mMutex);

    return mDefaultReplyCallback;
}

const FakeEwsServer::DialogEntry::ReplyCallback FakeEwsServer::overrideReplyCallback() const
{
    QMutexLocker lock(&mMutex);

    return mOverrideReplyCallback;
}

void FakeEwsServer::streamingConnectionStarted(FakeEwsConnection *conn)
{
    if (mStreamingEventsConnection) {
        qCWarningNC(EWSFAKE_LOG) << QStringLiteral("Got new streaming connection while existing one is active - terminating existing one");
        mStreamingEventsConnection->deleteLater();
    }

    mStreamingEventsConnection = conn;
}

ushort FakeEwsServer::portNumber() const
{
    QMutexLocker lock(&mMutex);

    return mPortNumber;
}
