/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fakeewsconnection.h"

#include <QBuffer>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QXmlNamePool>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlSerializer>

#include "fakeewsserver_debug.h"

static const QHash<uint, QString> responseCodes = {
    {200, QStringLiteral("OK")},
    {400, QStringLiteral("Bad Request")},
    {401, QStringLiteral("Unauthorized")},
    {403, QStringLiteral("Forbidden")},
    {404, QStringLiteral("Not Found")},
    {405, QStringLiteral("Method Not Allowed")},
    {500, QStringLiteral("Internal Server Error")},
};

static constexpr int streamingEventsHeartbeatIntervalSeconds = 5;

FakeEwsConnection::FakeEwsConnection(QTcpSocket *sock, FakeEwsServer *parent)
    : QObject(parent)
    , mSock(sock)
    , mContentLength(0)
    , mKeepAlive(false)
    , mState(Initial)
    , mAuthenticated(false)
{
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Got new EWS connection.");
    connect(mSock.data(), &QTcpSocket::disconnected, this, &FakeEwsConnection::disconnected);
    connect(mSock.data(), &QTcpSocket::readyRead, this, &FakeEwsConnection::dataAvailable);
    connect(&mDataTimer, &QTimer::timeout, this, &FakeEwsConnection::dataTimeout);
    connect(&mStreamingRequestHeartbeat, &QTimer::timeout, this, &FakeEwsConnection::streamingRequestHeartbeat);
    connect(&mStreamingRequestTimeout, &QTimer::timeout, this, &FakeEwsConnection::streamingRequestTimeout);
}

FakeEwsConnection::~FakeEwsConnection()
{
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Connection closed.");
}

void FakeEwsConnection::disconnected()
{
    deleteLater();
}

void FakeEwsConnection::dataAvailable()
{
    if (mState == Initial) {
        QByteArray line = mSock->readLine();
        QList<QByteArray> tokens = line.split(' ');
        mKeepAlive = false;

        if (tokens.size() < 3) {
            sendError(QStringLiteral("Invalid request header"));
            return;
        }
        if (tokens.at(0) != "POST") {
            sendError(QStringLiteral("Expected POST request"));
            return;
        }
        if (tokens.at(1) != "/EWS/Exchange.asmx") {
            sendError(QStringLiteral("Invalid EWS URL"));
            return;
        }
        mState = RequestReceived;
    }

    if (mState == RequestReceived) {
        QByteArray line;
        do {
            line = mSock->readLine();
            if (line.toLower().startsWith(QByteArray("content-length: "))) {
                bool ok;
                mContentLength = line.trimmed().mid(16).toUInt(&ok);
                if (!ok) {
                    sendError(QStringLiteral("Failed to parse content length."));
                    return;
                }
            } else if (line.toLower().startsWith(QByteArray("authorization: basic "))) {
                if (line.trimmed().mid(21) == "dGVzdDp0ZXN0") {
                    mAuthenticated = true;
                }
            } else if (line.toLower() == "connection: keep-alive\r\n") {
                mKeepAlive = true;
            }
        } while (!line.trimmed().isEmpty());

        if (line == "\r\n") {
            mState = HeadersReceived;
        }
    }

    if (mState == HeadersReceived) {
        if (mContentLength == 0) {
            sendError(QStringLiteral("Expected content"));
            return;
        }

        mContent += mSock->read(mContentLength - mContent.size());

        if (mContent.size() >= static_cast<int>(mContentLength)) {
            mDataTimer.stop();

            if (!mAuthenticated) {
                QString codeStr = responseCodes.value(401);
                QString response(QStringLiteral("HTTP/1.1 %1 %2\r\n"
                                                "WWW-Authenticate: Basic realm=\"Fake EWS Server\"\r\n"
                                                "Connection: close\r\n"
                                                "\r\n")
                                     .arg(401)
                                     .arg(codeStr));
                response += codeStr;
                mSock->write(response.toLatin1());
                mSock->disconnectFromHost();
                return;
            }

            FakeEwsServer::DialogEntry::HttpResponse resp = FakeEwsServer::EmptyResponse;

            const auto server = qobject_cast<FakeEwsServer *>(parent());
            const auto overrideReplyCallback = server->overrideReplyCallback();
            if (overrideReplyCallback) {
                QXmlResultItems ri;
                QXmlNamePool namePool;
                resp = overrideReplyCallback(QString::fromUtf8(mContent), ri, namePool);
            }

            if (resp == FakeEwsServer::EmptyResponse) {
                resp = parseRequest(QString::fromUtf8(mContent));
            }
            bool chunked = false;

            if (resp == FakeEwsServer::EmptyResponse) {
                resp = handleGetEventsRequest(QString::fromUtf8(mContent));
            }

            if (resp == FakeEwsServer::EmptyResponse) {
                resp = handleGetStreamingEventsRequest(QString::fromUtf8(mContent));
                if (resp.second > 1000) {
                    chunked = true;
                    resp.second %= 1000;
                }
            }

            auto defaultReplyCallback = server->defaultReplyCallback();
            if (defaultReplyCallback && (resp == FakeEwsServer::EmptyResponse)) {
                QXmlResultItems ri;
                QXmlNamePool namePool;
                resp = defaultReplyCallback(QString::fromUtf8(mContent), ri, namePool);
                qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning response from default callback ") << resp.second << QStringLiteral(": ") << resp.first;
            }

            if (resp == FakeEwsServer::EmptyResponse) {
                qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning default response 500.");
                resp = {QLatin1String(""), 500};
            }

            QByteArray buffer;
            QString codeStr = responseCodes.value(resp.second);
            QByteArray respContent = resp.first.toUtf8();
            buffer += QStringLiteral("HTTP/1.1 %1 %2\r\n").arg(resp.second).arg(codeStr).toLatin1();
            if (chunked) {
                buffer += "Transfer-Encoding: chunked\r\n";
                buffer += "\r\n";
                buffer += QByteArray::number(respContent.size(), 16) + "\r\n";
                buffer += respContent + "\r\n";
            } else {
                buffer += "Content-Length: " + QByteArray::number(respContent.size()) + "\r\n";
                buffer += mKeepAlive ? "Connection: Keep-Alive\n" : "Connection: Close\r\n";
                buffer += "\r\n";
                buffer += respContent;
            }
            mSock->write(buffer);

            if (!mKeepAlive && !chunked) {
                mSock->disconnectFromHost();
            }
            mContent.clear();
            mState = Initial;
        } else {
            mDataTimer.start(3000);
        }
    }
}

void FakeEwsConnection::sendError(const QString &msg, ushort code)
{
    qCWarningNC(EWSFAKE_LOG) << msg;
    QString codeStr = responseCodes.value(code);
    QByteArray response(QStringLiteral("HTTP/1.1 %1 %2\nConnection: close\n\n").arg(code).arg(codeStr).toLatin1());
    response += msg.toLatin1();
    mSock->write(response);
    mSock->disconnectFromHost();
}

void FakeEwsConnection::dataTimeout()
{
    qCWarning(EWSFAKE_LOG) << QLatin1String("Timeout waiting for content.");
    sendError(QStringLiteral("Timeout waiting for content."));
}

FakeEwsServer::DialogEntry::HttpResponse FakeEwsConnection::parseRequest(const QString &content)
{
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Got request: ") << content;

    auto server = qobject_cast<FakeEwsServer *>(parent());
    FakeEwsServer::DialogEntry::HttpResponse resp = FakeEwsServer::EmptyResponse;
    const auto dialogs{server->dialog()};
    for (const FakeEwsServer::DialogEntry &de : dialogs) {
        QXmlResultItems ri;
        QByteArray resultBytes;
        QString result;
        QBuffer resultBuffer(&resultBytes);
        resultBuffer.open(QIODevice::WriteOnly);
        QXmlQuery query;
        QXmlSerializer xser(query, &resultBuffer);
        if (!de.xQuery.isNull()) {
            query.setFocus(content);
            query.setQuery(de.xQuery);
            query.evaluateTo(&xser);
            query.evaluateTo(&ri);
            if (ri.hasError()) {
                qCDebugNC(EWSFAKE_LOG) << QStringLiteral("XQuery failed due to errors - skipping");
                continue;
            }
            result = QString::fromUtf8(resultBytes);
        }

        if (!result.trimmed().isEmpty()) {
            qCDebugNC(EWSFAKE_LOG) << QStringLiteral("Got match for \"") << de.description << QStringLiteral("\"");
            if (de.replyCallback) {
                resp = de.replyCallback(content, ri, query.namePool());
                qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning response from callback ") << resp.second << QStringLiteral(": ") << resp.first;
            } else {
                resp = {result.trimmed(), 200};
                qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning response from XQuery ") << resp.second << QStringLiteral(": ") << resp.first;
            }
            break;
        }
    }

    if (resp == FakeEwsServer::EmptyResponse) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning empty response.");
        qCInfoNC(EWSFAKE_LOG) << content;
    }

    return resp;
}

FakeEwsServer::DialogEntry::HttpResponse FakeEwsConnection::handleGetEventsRequest(const QString &content)
{
    const QRegularExpression re(QStringLiteral(
        "<?xml .*<\\w*:?GetEvents[ "
        ">].*<\\w*:?SubscriptionId>(?<subid>[^<]*)</\\w*:?SubscriptionId><\\w*:?Watermark>(?<watermark>[^<]*)</\\w*:?Watermark></\\w*:?GetEvents>.*"));

    QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch() || match.hasPartialMatch()) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Not a valid GetEvents request.");
        return FakeEwsServer::EmptyResponse;
    }

    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Got valid GetEvents request.");

    QString resp = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
        "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
        "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
        "<soap:Header>"
        "<t:ServerVersionInfo MajorVersion=\"8\" MinorVersion=\"0\" MajorBuildNumber=\"628\" MinorBuildNumber=\"0\" />"
        "</soap:Header>"
        "<soap:Body>"
        "<m:GetEventsResponse xmlns=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
        "<m:ResponseMessages>"
        "<m:GetEventsResponseMessage ResponseClass=\"Success\">"
        "<m:ResponseCode>NoError</m:ResponseCode>"
        "<m:Notification>");

    if (match.captured(QStringLiteral("subid")).isEmpty() || match.captured(QStringLiteral("watermark")).isEmpty()) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Missing subscription id or watermark.");
        const QString errorResp = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
            "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
            "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
            "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
            "<soap:Header>"
            "<t:ServerVersionInfo MajorVersion=\"8\" MinorVersion=\"0\" MajorBuildNumber=\"628\" MinorBuildNumber=\"0\" />"
            "</soap:Header>"
            "<soap:Body>"
            "<m:GetEventsResponse>"
            "<m:ResponseMessages>"
            "<m:GetEventsResponseMessage ResponseClass=\"Error\">"
            "<m:MessageText>Missing subscription id or watermark.</m:MessageText>"
            "<m:ResponseCode>ErrorInvalidPullSubscriptionId</m:ResponseCode>"
            "<m:DescriptiveLinkKey>0</m:DescriptiveLinkKey>"
            "</m:GetEventsResponseMessage>"
            "</m:ResponseMessages>"
            "</m:GetEventsResponse>"
            "</soap:Body>"
            "</soap:Envelope>");
        return {errorResp, 200};
    }

    resp += QLatin1String("<SubscriptionId>") + match.captured(QStringLiteral("subid")) + QLatin1String("<SubscriptionId>");
    resp += QLatin1String("<PreviousWatermark>") + match.captured(QStringLiteral("watermark")) + QLatin1String("<PreviousWatermark>");
    resp += QStringLiteral("<MoreEvents>false<MoreEvents>");

    auto server = qobject_cast<FakeEwsServer *>(parent());
    const QStringList events = server->retrieveEventsXml();
    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning %1 events.").arg(events.size());
    for (const QString &eventXml : events) {
        resp += eventXml;
    }

    resp += QStringLiteral(
        "</m:Notification></m:GetEventsResponseMessage></m:ResponseMessages>"
        "</m:GetEventsResponse></soap:Body></soap:Envelope>");

    return {resp, 200};
}

QString FakeEwsConnection::prepareEventsResponse(const QStringList &events)
{
    QString resp = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
        "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
        "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
        "<soap:Header>"
        "<t:ServerVersionInfo MajorVersion=\"8\" MinorVersion=\"0\" MajorBuildNumber=\"628\" MinorBuildNumber=\"0\" />"
        "</soap:Header>"
        "<soap:Body>"
        "<m:GetStreamingEventsResponse>"
        "<m:ResponseMessages>"
        "<m:GetStreamingEventsResponseMessage ResponseClass=\"Success\">"
        "<m:ResponseCode>NoError</m:ResponseCode>"
        "<m:ConnectionStatus>OK</m:ConnectionStatus>");

    if (!events.isEmpty()) {
        resp += QLatin1String("<m:Notifications><m:Notification><SubscriptionId>") + mStreamingSubId + QLatin1String("<SubscriptionId>");

        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Returning %1 events.").arg(events.size());
        for (const QString &eventXml : std::as_const(events)) {
            resp += eventXml;
        }

        resp += QStringLiteral("</m:Notification></m:Notifications>");
    }
    resp += QStringLiteral(
        "</m:GetStreamingEventsResponseMessage></m:ResponseMessages>"
        "</m:GetStreamingEventsResponse></soap:Body></soap:Envelope>");

    return resp;
}

FakeEwsServer::DialogEntry::HttpResponse FakeEwsConnection::handleGetStreamingEventsRequest(const QString &content)
{
    const QRegularExpression re(
        QStringLiteral("<?xml .*<\\w*:?GetStreamingEvents[ "
                       ">].*<\\w*:?SubscriptionIds><\\w*:?SubscriptionId>(?<subid>[^<]*)</\\w*:?SubscriptionId></"
                       "\\w*:?SubscriptionIds>.*<\\w*:?ConnectionTimeout>(?<timeout>[^<]*)</\\w*:?ConnectionTimeout></\\w*:?GetStreamingEvents>.*"));

    QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch() || match.hasPartialMatch()) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Not a valid GetStreamingEvents request.");
        return FakeEwsServer::EmptyResponse;
    }

    qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Got valid GetStreamingEvents request.");

    if (match.captured(QStringLiteral("subid")).isEmpty() || match.captured(QStringLiteral("timeout")).isEmpty()) {
        qCInfoNC(EWSFAKE_LOG) << QStringLiteral("Missing subscription id or timeout.");
        const QString errorResp = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
            "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
            "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
            "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
            "<soap:Header>"
            "<t:ServerVersionInfo MajorVersion=\"8\" MinorVersion=\"0\" MajorBuildNumber=\"628\" MinorBuildNumber=\"0\" />"
            "</soap:Header>"
            "<soap:Body>"
            "<m:GetStreamingEventsResponse>"
            "<m:ResponseMessages>"
            "<m:GetStreamingEventsResponseMessage ResponseClass=\"Error\">"
            "<m:MessageText>Missing subscription id or timeout.</m:MessageText>"
            "<m:ResponseCode>ErrorInvalidSubscription</m:ResponseCode>"
            "<m:DescriptiveLinkKey>0</m:DescriptiveLinkKey>"
            "</m:GetEventsResponseMessage>"
            "</m:ResponseMessages>"
            "</m:GetEventsResponse>"
            "</soap:Body>"
            "</soap:Envelope>");
        return {errorResp, 200};
    }

    mStreamingSubId = match.captured(QStringLiteral("subid"));

    auto server = qobject_cast<FakeEwsServer *>(parent());
    const QStringList events = server->retrieveEventsXml();

    QString resp = prepareEventsResponse(events);

    mStreamingRequestTimeout.start(match.captured(QStringLiteral("timeout")).toInt() * 1000 * 60);
    mStreamingRequestHeartbeat.setSingleShot(false);
    mStreamingRequestHeartbeat.start(streamingEventsHeartbeatIntervalSeconds * 1000);

    Q_EMIT streamingRequestStarted(this);

    return {resp, 1200};
}

void FakeEwsConnection::streamingRequestHeartbeat()
{
    sendEvents(QStringList());
}

void FakeEwsConnection::streamingRequestTimeout()
{
    mStreamingRequestTimeout.stop();
    mStreamingRequestHeartbeat.stop();
    mSock->write("0\r\n\r\n");
    mSock->disconnectFromHost();
}

void FakeEwsConnection::sendEvents(const QStringList &events)
{
    QByteArray resp = prepareEventsResponse(events).toUtf8();

    mSock->write(QByteArray::number(resp.size(), 16) + "\r\n" + resp + "\r\n");
}
