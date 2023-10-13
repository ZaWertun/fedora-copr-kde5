/*
   SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "fakeserver.h"

// Qt
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

FakeServerThread::FakeServerThread(QObject *parent)
    : QThread(parent)
    , mServer(nullptr)
{
}

void FakeServerThread::run()
{
    mServer = new FakeServer();

    // Run forever, until someone from the outside calls quit() on us and quits the
    // event loop
    exec();

    delete mServer;
    mServer = nullptr;
}

FakeServer *FakeServerThread::server() const
{
    Q_ASSERT(mServer != nullptr);
    return mServer;
}

FakeServer::FakeServer(QObject *parent)
    : QObject(parent)
{
    mTcpServer = new QTcpServer();
    if (!mTcpServer->listen(QHostAddress(QHostAddress::LocalHost), 5989)) {
        qCritical() << "Unable to start the server";
    }

    connect(mTcpServer, &QTcpServer::newConnection, this, &FakeServer::newConnection);
}

FakeServer::~FakeServer()
{
    if (mConnections > 0) {
        disconnect(mTcpServerConnection, &QTcpSocket::readyRead, this, &FakeServer::dataAvailable);
    }

    delete mTcpServer;
    mTcpServer = nullptr;
}

QByteArray FakeServer::parseDeleteMark(const QByteArray &expectedData, const QByteArray &dataReceived)
{
    // Only called from parseResponse(), which is already thread-safe

    const QByteArray deleteMark = QStringLiteral("%DELE%").toUtf8();
    if (expectedData.contains(deleteMark)) {
        Q_ASSERT(!mAllowedDeletions.isEmpty());
        for (int i = 0; i < mAllowedDeletions.size(); i++) {
            QByteArray substituted = expectedData;
            substituted.replace(deleteMark, mAllowedDeletions[i]);
            if (substituted == dataReceived) {
                mAllowedDeletions.removeAt(i);
                return substituted;
            }
        }
        qWarning() << "Received:" << dataReceived.data() << "\nExpected:" << expectedData.data();
        Q_ASSERT_X(false, "FakeServer::parseDeleteMark", "Unable to substitute data!");
        return {};
    } else {
        return expectedData;
    }
}

QByteArray FakeServer::parseRetrMark(const QByteArray &expectedData, const QByteArray &dataReceived)
{
    // Only called from parseResponse(), which is already thread-safe

    const QByteArray retrMark = QStringLiteral("%RETR%").toUtf8();
    if (expectedData.contains(retrMark)) {
        Q_ASSERT(!mAllowedRetrieves.isEmpty());
        for (int i = 0; i < mAllowedRetrieves.size(); i++) {
            QByteArray substituted = expectedData;
            substituted.replace(retrMark, mAllowedRetrieves[i]);
            if (substituted == dataReceived) {
                mAllowedRetrieves.removeAt(i);
                return substituted;
            }
        }
        qWarning() << "Received:" << dataReceived.data() << "\nExpected:" << expectedData.data();
        Q_ASSERT_X(false, "FakeServer::parseRetrMark", "Unable to substitute data!");
        return {};
    } else {
        return expectedData;
    }
}

QByteArray FakeServer::parseResponse(const QByteArray &expectedData, const QByteArray &dataReceived)
{
    // Only called from dataAvailable, which is already thread-safe

    const QByteArray result = parseDeleteMark(expectedData, dataReceived);
    if (result != expectedData) {
        return result;
    } else {
        return parseRetrMark(expectedData, dataReceived);
    }
}

static QByteArray removeCRLF(const QByteArray &ba)
{
    QByteArray returnArray = ba;
    return returnArray.replace(QByteArrayLiteral("\r\n"), QByteArray());
}

void FakeServer::dataAvailable()
{
    QMutexLocker locker(&mMutex);
    mProgress++;

    // We got data, so we better expect it and have an answer!
    Q_ASSERT(!mReadData.isEmpty());
    Q_ASSERT(!mWriteData.isEmpty());

    const QByteArray data = mTcpServerConnection->readAll();
    const QByteArray expected(mReadData.takeFirst());
    const QByteArray reallyExpected = parseResponse(expected, data);
    if (data != reallyExpected) {
        qDebug() << "Got data:" << removeCRLF(data);
        qDebug() << "Expected data:" << removeCRLF(expected);
        qDebug() << "Really expected:" << removeCRLF(reallyExpected);
    }

    Q_ASSERT(data == reallyExpected);

    QByteArray toWrite = mWriteData.takeFirst();
    // qDebug() << "Going to write data:" << removeCRLF( toWrite );
    const bool allWritten = mTcpServerConnection->write(toWrite) == toWrite.size();
    Q_ASSERT(allWritten);
    Q_UNUSED(allWritten)
    const bool flushed = mTcpServerConnection->flush();
    Q_ASSERT(flushed);
    Q_UNUSED(flushed)
}

void FakeServer::newConnection()
{
    QMutexLocker locker(&mMutex);
    Q_ASSERT(mConnections == 0);
    mConnections++;
    mGotDisconnected = false;

    mTcpServerConnection = mTcpServer->nextPendingConnection();
    mTcpServerConnection->write(QByteArray("+OK Initech POP3 server ready.\r\n"));
    connect(mTcpServerConnection, &QTcpSocket::readyRead, this, &FakeServer::dataAvailable);
    connect(mTcpServerConnection, &QTcpSocket::disconnected, this, &FakeServer::slotDisconnected);
}

void FakeServer::slotDisconnected()
{
    QMutexLocker locker(&mMutex);
    mConnections--;
    mGotDisconnected = true;
    Q_ASSERT(mConnections == 0);
    Q_ASSERT(mAllowedDeletions.isEmpty());
    Q_ASSERT(mAllowedRetrieves.isEmpty());
    Q_ASSERT(mReadData.isEmpty());
    Q_ASSERT(mWriteData.isEmpty());
    Q_EMIT disconnected();
}

void FakeServer::setAllowedDeletions(const QString &deleteIds)
{
    QMutexLocker locker(&mMutex);
    mAllowedDeletions.clear();
    const QStringList ids = deleteIds.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &id : ids) {
        mAllowedDeletions.append(id.toUtf8());
    }
}

void FakeServer::setAllowedRetrieves(const QString &retrieveIds)
{
    QMutexLocker locker(&mMutex);
    mAllowedRetrieves.clear();
    const QStringList ids = retrieveIds.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &id : ids) {
        mAllowedRetrieves.append(id.toUtf8());
    }
}

void FakeServer::setMails(const QList<QByteArray> &mails)
{
    QMutexLocker locker(&mMutex);
    mMails = mails;
}

void FakeServer::setNextConversation(const QString &conversation, const QList<int> &exceptions)
{
    QMutexLocker locker(&mMutex);

    Q_ASSERT(mReadData.isEmpty());
    Q_ASSERT(mWriteData.isEmpty());
    Q_ASSERT(!conversation.isEmpty());

    mGotDisconnected = false;
    const QStringList lines = conversation.split(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
    Q_ASSERT(lines.first().startsWith(QLatin1String("C:")));

    enum Mode {
        Client,
        Server,
    };
    Mode mode = Client;

    const QByteArray mailSizeMarker = QStringLiteral("%MAILSIZE%").toLatin1();
    const QByteArray mailMarker = QStringLiteral("%MAIL%").toLatin1();
    int sizeIndex = 0;
    int mailIndex = 0;

    for (const QString &line : lines) {
        QByteArray lineData(line.toUtf8());

        if (lineData.contains(mailSizeMarker)) {
            Q_ASSERT(mMails.size() > sizeIndex);
            lineData.replace(mailSizeMarker, QString::number(mMails[sizeIndex++].size()).toLatin1());
        }
        if (lineData.contains(mailMarker)) {
            while (exceptions.contains(mailIndex + 1)) {
                mailIndex++;
            }
            Q_ASSERT(mMails.size() > mailIndex);
            lineData.replace(mailMarker, mMails[mailIndex++]);
        }

        if (lineData.startsWith("S: ")) {
            mWriteData.append(lineData.mid(3) + "\r\n");
            mode = Server;
        } else if (line.startsWith(QLatin1String("C: "))) {
            mReadData.append(lineData.mid(3) + "\r\n");
            mode = Client;
        } else {
            switch (mode) {
            case Server:
                mWriteData.last() += (lineData + "\r\n");
                break;
            case Client:
                mReadData.last() += (lineData + "\r\n");
                break;
            }
        }
    }
}

int FakeServer::progress() const
{
    QMutexLocker locker(&mMutex);
    return mProgress;
}

bool FakeServer::gotDisconnected() const
{
    QMutexLocker locker(&mMutex);
    return mGotDisconnected;
}
