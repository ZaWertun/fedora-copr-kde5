/*
   SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMutex>
#include <QThread>

class QTcpServer;
class QTcpSocket;
class FakeServer : public QObject
{
    Q_OBJECT

public:
    explicit FakeServer(QObject *parent = nullptr);
    ~FakeServer() override;

    void setNextConversation(const QString &conversation, const QList<int> &exceptions = QList<int>());
    void setAllowedDeletions(const QString &deleteIds);
    void setAllowedRetrieves(const QString &retrieveIds);
    void setMails(const QList<QByteArray> &mails);

    // This is kind of a hack: The POP3 test needs to know when the POP3 client
    // disconnects from the server. Normally, we could just use a QSignalSpy on the
    // disconnected() signal, but that is not thread-safe. Therefore this hack with the
    // state variable mGotDisconnected
    bool gotDisconnected() const;

    // Returns an integer that is incremented each time the POP3 server receives some
    // data
    int progress() const;

Q_SIGNALS:
    void disconnected();

private Q_SLOTS:

    void newConnection();
    void dataAvailable();
    void slotDisconnected();

private:
    QByteArray parseDeleteMark(const QByteArray &expectedData, const QByteArray &dataReceived);
    QByteArray parseRetrMark(const QByteArray &expectedData, const QByteArray &dataReceived);
    QByteArray parseResponse(const QByteArray &expectedData, const QByteArray &dataReceived);

    QList<QByteArray> mReadData;
    QList<QByteArray> mWriteData;
    QList<QByteArray> mAllowedDeletions;
    QList<QByteArray> mAllowedRetrieves;
    QList<QByteArray> mMails;
    QTcpServer *mTcpServer = nullptr;
    QTcpSocket *mTcpServerConnection = nullptr;
    int mConnections = 0;
    int mProgress = 0;
    bool mGotDisconnected = false;

    // We use one big mutex to protect everything
    // There shouldn't be deadlocks, as there are only 2 places where the functions
    // are called: From the KTcpSocket (or QSslSocket with KIO >= 5.65) signals, which
    // are triggered by the POP3 ioslave, and from the actual test.
    mutable QMutex mMutex;
};

class FakeServerThread : public QThread
{
    Q_OBJECT

public:
    explicit FakeServerThread(QObject *parent);
    void run() override;

    // Returns the FakeServer use. Be careful when using this and make sure
    // the methods you use are actually thread-safe!!
    // This should however be the case because the FakeServer uses one big mutex
    // to protect everything.
    FakeServer *server() const;

private:
    FakeServer *mServer = nullptr;
};
