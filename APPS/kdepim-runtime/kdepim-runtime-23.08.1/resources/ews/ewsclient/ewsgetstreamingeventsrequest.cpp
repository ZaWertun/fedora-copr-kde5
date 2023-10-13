/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsgetstreamingeventsrequest.h"

#include <QTemporaryFile>

#include "ewsclient_debug.h"

static constexpr uint respChunkTimeout = 250; /* ms */

EwsGetStreamingEventsRequest::EwsGetStreamingEventsRequest(EwsClient &client, QObject *parent)
    : EwsEventRequestBase(client, QStringLiteral("GetStreamingEvents"), parent)
    , mTimeout(30)
    , mRespTimer(this)
{
    mRespTimer.setInterval(respChunkTimeout);
    connect(&mRespTimer, &QTimer::timeout, this, &EwsGetStreamingEventsRequest::requestDataTimeout);
}

EwsGetStreamingEventsRequest::~EwsGetStreamingEventsRequest() = default;

void EwsGetStreamingEventsRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    if (!serverVersion().supports(EwsServerVersion::StreamingSubscription)) {
        setServerVersion(EwsServerVersion::minSupporting(EwsServerVersion::StreamingSubscription));
    }

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("GetStreamingEvents"));

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("SubscriptionIds"));
    writer.writeTextElement(ewsTypeNsUri, QStringLiteral("SubscriptionId"), mSubscriptionId);
    writer.writeEndElement();

    writer.writeTextElement(ewsMsgNsUri, QStringLiteral("ConnectionTimeout"), QString::number(mTimeout));

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting GetStreamingEvents request (subId: %1, timeout: %2)").arg(ewsHash(mSubscriptionId)).arg(mTimeout);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

void EwsGetStreamingEventsRequest::requestData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    mRespTimer.stop();
    qCDebug(EWSCLI_PROTO_LOG) << "data" << job << data;
    mResponseData += QString::fromUtf8(data);
    mRespTimer.start();
}

void EwsGetStreamingEventsRequest::requestDataTimeout()
{
    if (mResponseData.isEmpty()) {
        return;
    }
    if (EWSCLI_PROTO_LOG().isDebugEnabled()) {
        ewsLogDir.setAutoRemove(false);
        if (ewsLogDir.isValid()) {
            QTemporaryFile dumpFile(ewsLogDir.path() + QStringLiteral("/ews_xmldump_XXXXXXX.xml"));
            dumpFile.open();
            dumpFile.setAutoRemove(false);
            dumpFile.write(mResponseData.toUtf8());
            qCDebug(EWSCLI_PROTO_LOG) << "response dumped to" << dumpFile.fileName();
            dumpFile.close();
        }
    }

    QXmlStreamReader reader(mResponseData);
    if (!readResponse(reader)) {
        const auto jobs{subjobs()};
        for (KJob *job : jobs) {
            removeSubjob(job);
            job->kill();
        }
        emitResult();
    } else {
        Q_EMIT eventsReceived(this);
    }

    mResponseData.clear();
}

void EwsGetStreamingEventsRequest::eventsProcessed(const Response &resp)
{
    mResponses.removeOne(resp);
}
