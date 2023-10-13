/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMutex>
#include <QThread>

#include "fakeewsserver.h"
#include "fakeewsserver_export.h"

class FAKEEWSSERVER_EXPORT FakeEwsServerThread : public QThread
{
    Q_OBJECT
public:
    explicit FakeEwsServerThread(QObject *parent = nullptr);
    ~FakeEwsServerThread() override;

    //    FakeEwsServer *server() const;
    ushort portNumber() const
    {
        return mPortNumber;
    }

    bool isRunning() const
    {
        return mIsRunning == 1;
    }

    void setDialog(const FakeEwsServer::DialogEntry::List &dialog);
    void setDefaultReplyCallback(const FakeEwsServer::DialogEntry::ReplyCallback &defaultReplyCallback);
    void setOverrideReplyCallback(const FakeEwsServer::DialogEntry::ReplyCallback &overrideReplyCallback);
    void queueEventsXml(const QStringList &events);
    bool waitServerStarted() const;
Q_SIGNALS:
    void serverStarted(bool ok);

protected:
    void run() override;
private Q_SLOTS:
    void doQueueEventsXml(const QStringList &events);

private:
    QScopedPointer<FakeEwsServer> mServer;
    ushort mPortNumber;
    QAtomicInt mIsRunning;
    mutable QMutex mMutex;
};
