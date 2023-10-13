/*
    SPDX-FileCopyrightText: 2017-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fakeewsserverthread.h"

#include <QEventLoop>

#include "fakeewsserver_debug.h"

FakeEwsServerThread::FakeEwsServerThread(QObject *parent)
    : QThread(parent)
    , mPortNumber(0)
    , mIsRunning(0)
{
}

FakeEwsServerThread::~FakeEwsServerThread() = default;

void FakeEwsServerThread::run()
{
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Starting fake server thread");
    mMutex.lock();
    mServer.reset(new FakeEwsServer(nullptr));
    bool ok = mServer->start();
    mMutex.unlock();

    if (ok) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Fake server thread started.");
        mPortNumber = mServer->portNumber();
        Q_EMIT serverStarted(ok);
        mIsRunning = 1;
        exec();
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Fake server thread terminating.");
    } else {
        Q_EMIT serverStarted(ok);
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Fake server thread start failed.");
    }

    mMutex.lock();
    mServer.reset();
    mMutex.unlock();
}

void FakeEwsServerThread::setDialog(const FakeEwsServer::DialogEntry::List &dialog)
{
    QMutexLocker lock(&mMutex);

    if (mServer) {
        mServer->setDialog(dialog);
    }
}

void FakeEwsServerThread::setDefaultReplyCallback(const FakeEwsServer::DialogEntry::ReplyCallback &defaultReplyCallback)
{
    QMutexLocker lock(&mMutex);

    if (mServer) {
        mServer->setDefaultReplyCallback(defaultReplyCallback);
    }
}

void FakeEwsServerThread::setOverrideReplyCallback(const FakeEwsServer::DialogEntry::ReplyCallback &overrideReplyCallback)
{
    QMutexLocker lock(&mMutex);

    if (mServer) {
        mServer->setOverrideReplyCallback(overrideReplyCallback);
    }
}

void FakeEwsServerThread::queueEventsXml(const QStringList &events)
{
    QMutexLocker lock(&mMutex);

    if (mServer) {
        metaObject()->invokeMethod(this, "doQueueEventsXml", Q_ARG(QStringList, events));
    }
}

void FakeEwsServerThread::doQueueEventsXml(const QStringList &events)
{
    mServer->queueEventsXml(events);
}

bool FakeEwsServerThread::waitServerStarted() const
{
    QEventLoop loop;
    {
        QMutexLocker lock(&mMutex);
        if (isFinished()) {
            return false;
        }
        if (mIsRunning) {
            return true;
        }
        connect(this, &FakeEwsServerThread::serverStarted, this, [&loop](bool ok) {
            loop.exit(ok ? 1 : 0);
        });
    }
    return loop.exec();
}
