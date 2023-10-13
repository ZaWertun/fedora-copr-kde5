/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsgeteventsrequest.h"
#include "ewsclient_debug.h"

EwsGetEventsRequest::EwsGetEventsRequest(EwsClient &client, QObject *parent)
    : EwsEventRequestBase(client, QStringLiteral("GetEvents"), parent)
{
}

EwsGetEventsRequest::~EwsGetEventsRequest() = default;

void EwsGetEventsRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("GetEvents"));

    writer.writeTextElement(ewsMsgNsUri, QStringLiteral("SubscriptionId"), mSubscriptionId);

    writer.writeTextElement(ewsMsgNsUri, QStringLiteral("Watermark"), mWatermark);

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting GetEvents request (subId: %1, wmark: %2)").arg(mSubscriptionId, mWatermark);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}
