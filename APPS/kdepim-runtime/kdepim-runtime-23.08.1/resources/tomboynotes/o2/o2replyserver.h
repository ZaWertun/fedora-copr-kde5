/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QTcpServer>

/// HTTP server to process authentication response.
class O2ReplyServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit O2ReplyServer(QObject *parent = nullptr);

    /// Page content on local host after successful oauth - in case you do not want to close the browser, but display something
    Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent)
    QByteArray replyContent() const;
    void setReplyContent(const QByteArray &value);

Q_SIGNALS:
    void verificationReceived(const QMultiMap<QString, QString> &);

public Q_SLOTS:
    void onIncomingConnection();
    void onBytesReady();
    QMultiMap<QString, QString> parseQueryParams(QByteArray *data);

protected:
    QByteArray replyContent_;
};
