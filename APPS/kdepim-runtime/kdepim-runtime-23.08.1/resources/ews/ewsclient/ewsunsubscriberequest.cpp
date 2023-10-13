/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsunsubscriberequest.h"

#include <QXmlStreamWriter>

#include "ewsclient_debug.h"

EwsUnsubscribeRequest::EwsUnsubscribeRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsUnsubscribeRequest::~EwsUnsubscribeRequest() = default;

void EwsUnsubscribeRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("Unsubscribe"));

    writer.writeTextElement(ewsMsgNsUri, QStringLiteral("SubscriptionId"), mSubscriptionId);

    writer.writeEndElement(); // Unsubscribe

    endSoapDocument(writer);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsUnsubscribeRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("Unsubscribe"), [this](QXmlStreamReader &reader) {
        return parseUnsubscribeResponse(reader);
    });
}

bool EwsUnsubscribeRequest::parseUnsubscribeResponse(QXmlStreamReader &reader)
{
    QSharedPointer<Response> resp(new Response(reader));
    if (resp->responseClass() == EwsResponseUnknown) {
        return false;
    }

    mResponse = resp;
    return true;
}

EwsUnsubscribeRequest::Response::Response(QXmlStreamReader &reader)
    : EwsRequest::Response(reader)
{
    if (mClass == EwsResponseParseError) {
        return;
    }

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsMsgNsUri && reader.namespaceUri() != ewsTypeNsUri) {
            setErrorMsg(QStringLiteral("Unexpected namespace in %1 element: %2").arg(QStringLiteral("ResponseMessage"), reader.namespaceUri().toString()));
            return;
        }

        if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}
