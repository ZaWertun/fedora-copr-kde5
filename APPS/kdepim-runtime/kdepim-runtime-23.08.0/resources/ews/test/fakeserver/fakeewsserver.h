/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include "fakeewsserver_export.h"

#include <QMutex>
#include <QPointer>
#include <QTcpServer>
class QTcpSocket;

class FakeEwsConnection;
class QXmlResultItems;
class QXmlNamePool;

class FAKEEWSSERVER_EXPORT FakeEwsServer : public QTcpServer
{
    Q_OBJECT
public:
    class FAKEEWSSERVER_EXPORT DialogEntry
    {
    public:
        using HttpResponse = QPair<QString, ushort>;
        using ReplyCallback = std::function<HttpResponse(const QString &, QXmlResultItems &, const QXmlNamePool &)>;
        QString xQuery;
        ReplyCallback replyCallback;
        QString description;

        using List = QVector<DialogEntry>;
    };

    static const DialogEntry::HttpResponse EmptyResponse;

    explicit FakeEwsServer(QObject *parent);
    ~FakeEwsServer() override;
    bool start();
    void setDefaultReplyCallback(const DialogEntry::ReplyCallback &defaultReplyCallback);
    void setOverrideReplyCallback(const DialogEntry::ReplyCallback &overrideReplyCallback);
    void queueEventsXml(const QStringList &events);
    void setDialog(const DialogEntry::List &dialog);
    ushort portNumber() const;
private Q_SLOTS:
    void newConnectionReceived();
    void streamingConnectionStarted(FakeEwsConnection *conn);

private:
    void dataAvailable(QTcpSocket *sock);
    void sendError(QTcpSocket *sock, const QString &msg, ushort code = 500);
    const DialogEntry::List dialog() const;
    const DialogEntry::ReplyCallback defaultReplyCallback() const;
    const DialogEntry::ReplyCallback overrideReplyCallback() const;
    QStringList retrieveEventsXml();

    DialogEntry::List mDialog;
    DialogEntry::ReplyCallback mDefaultReplyCallback;
    DialogEntry::ReplyCallback mOverrideReplyCallback;
    QStringList mEventQueue;
    QPointer<FakeEwsConnection> mStreamingEventsConnection;
    ushort mPortNumber;
    mutable QMutex mMutex;

    friend class FakeEwsConnection;
};
