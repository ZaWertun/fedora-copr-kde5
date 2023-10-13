/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewssubscriberequest.h"

#include <QXmlStreamWriter>

#include "ewsclient_debug.h"

static const QVector<QString> subscribeTypeNames = {
    QStringLiteral("PullSubscriptionRequest"),
    QStringLiteral("PushSubscriptionRequest"),
    QStringLiteral("StreamingSubscriptionRequest"),
};

static const QVector<QString> eventTypeNames = {
    QStringLiteral("CopiedEvent"),
    QStringLiteral("CreatedEvent"),
    QStringLiteral("DeletedEvent"),
    QStringLiteral("ModifiedEvent"),
    QStringLiteral("MovedEvent"),
    QStringLiteral("NewMailEvent"),
    QStringLiteral("FreeBusyChangedEvent"),
};

EwsSubscribeRequest::EwsSubscribeRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
    , mType(PullSubscription)
    , mAllFolders(false)
    , mTimeout(30)
{
}

EwsSubscribeRequest::~EwsSubscribeRequest() = default;

void EwsSubscribeRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    if (mType == StreamingSubscription && !serverVersion().supports(EwsServerVersion::StreamingSubscription)) {
        setServerVersion(EwsServerVersion::minSupporting(EwsServerVersion::StreamingSubscription));
    }
    if (mEventTypes.contains(EwsFreeBusyChangedEvent) && !serverVersion().supports(EwsServerVersion::FreeBusyChangedEvent)) {
        setServerVersion(EwsServerVersion::minSupporting(EwsServerVersion::FreeBusyChangedEvent));
    }

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("Subscribe"));

    writer.writeStartElement(ewsMsgNsUri, subscribeTypeNames[mType]);

    if (mAllFolders) {
        writer.writeAttribute(QStringLiteral("SubscribeToAllFolders"), QStringLiteral("true"));
    }

    writer.writeStartElement(ewsTypeNsUri, QStringLiteral("FolderIds"));
    for (const EwsId &id : std::as_const(mFolderIds)) {
        id.writeFolderIds(writer);
    }
    writer.writeEndElement();

    writer.writeStartElement(ewsTypeNsUri, QStringLiteral("EventTypes"));
    for (const EwsEventType type : std::as_const(mEventTypes)) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("EventType"), eventTypeNames[type]);
    }
    writer.writeEndElement(); // EventTypes

    if (mType == PullSubscription) {
        if (!mWatermark.isNull()) {
            writer.writeTextElement(ewsTypeNsUri, QStringLiteral("Watermark"), mWatermark);
        }
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("Timeout"), QString::number(mTimeout));
    }

    writer.writeEndElement(); // XxxSubscriptionRequest

    writer.writeEndElement(); // Subscribe

    endSoapDocument(writer);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsSubscribeRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("Subscribe"), [this](QXmlStreamReader &reader) {
        return parseSubscribeResponse(reader);
    });
}

bool EwsSubscribeRequest::parseSubscribeResponse(QXmlStreamReader &reader)
{
    QSharedPointer<Response> resp(new Response(reader));
    if (resp->responseClass() == EwsResponseUnknown) {
        return false;
    }

    mResponse = resp;
    return true;
}

EwsSubscribeRequest::Response::Response(QXmlStreamReader &reader)
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

        if (reader.name() == QLatin1String("SubscriptionId")) {
            mId = reader.readElementText();

            if (reader.error() != QXmlStreamReader::NoError) {
                setErrorMsg(QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("SubscriptionId")));
                return;
            }
        } else if (reader.name() == QLatin1String("Watermark")) {
            mWatermark = reader.readElementText();

            if (reader.error() != QXmlStreamReader::NoError) {
                setErrorMsg(QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("Watermark")));
                return;
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}
