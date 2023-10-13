/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#include "debug.h"
#include <QPair>
#include <QTcpSocket>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2/o2replyserver.h"

O2ReplyServer::O2ReplyServer(QObject *parent)
    : QTcpServer(parent)
{
    connect(this, &QTcpServer::newConnection, this, &O2ReplyServer::onIncomingConnection);
    replyContent_ = QByteArrayLiteral("<HTML></HTML>");
}

void O2ReplyServer::onIncomingConnection()
{
    QTcpSocket *socket = nextPendingConnection();
    connect(socket, &QIODevice::readyRead, this, &O2ReplyServer::onBytesReady, Qt::UniqueConnection);
    connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);
}

void O2ReplyServer::onBytesReady()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2ReplyServer::onBytesReady";
    auto socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }
    QByteArray reply;
    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
    reply.append(QByteArray("Content-Length: ") + QByteArray::number(replyContent_.size()) + "\r\n\r\n");
    reply.append(replyContent_);
    socket->write(reply);

    QByteArray data = socket->readAll();
    QMultiMap<QString, QString> queryParams = parseQueryParams(&data);
    socket->disconnectFromHost();
    close();
    Q_EMIT verificationReceived(queryParams);
}

QMultiMap<QString, QString> O2ReplyServer::parseQueryParams(QByteArray *data)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2ReplyServer::parseQueryParams";

    QString splitGetLine = QString::fromLatin1(*data).split(QStringLiteral("\r\n")).first();
    splitGetLine.remove(QStringLiteral("GET "));
    splitGetLine.remove(QStringLiteral("HTTP/1.1"));
    splitGetLine.remove(QStringLiteral("\r\n"));
    splitGetLine.prepend(QStringLiteral("http://localhost"));
    QUrl getTokenUrl(splitGetLine);

    QList<QPair<QString, QString>> tokens;
#if QT_VERSION < 0x050000
    tokens = getTokenUrl.queryItems();
#else
    QUrlQuery query(getTokenUrl);
    tokens = query.queryItems();
#endif
    QMultiMap<QString, QString> queryParams;
    for (const auto &tokenPair : std::as_const(tokens)) {
        // FIXME: We are decoding key and value again. This helps with Google OAuth, but is it mandated by the standard?
        const QString key = QUrl::fromPercentEncoding(tokenPair.first.trimmed().toLatin1());
        const QString value = QUrl::fromPercentEncoding(tokenPair.second.trimmed().toLatin1());
        queryParams.insert(key, value);
    }
    return queryParams;
}

QByteArray O2ReplyServer::replyContent() const
{
    return replyContent_;
}

void O2ReplyServer::setReplyContent(const QByteArray &value)
{
    replyContent_ = value;
}
