/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmoveitemrequest.h"
#include "ewsclient_debug.h"

EwsMoveItemRequest::EwsMoveItemRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsMoveItemRequest::~EwsMoveItemRequest() = default;

void EwsMoveItemRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("MoveItem"));

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ToFolderId"));
    mDestFolderId.writeFolderIds(writer);
    writer.writeEndElement();

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ItemIds"));
    for (const EwsId &id : std::as_const(mIds)) {
        id.writeItemIds(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNCS(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting MoveItem request (") << mIds << "to" << mDestFolderId << ")";

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsMoveItemRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("MoveItem"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsMoveItemRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got MoveItem response - OK");
        } else {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got MoveItem response - %1").arg(resp.responseMessage());
        }
    }
    mResponses.append(resp);
    return true;
}

EwsMoveItemRequest::Response::Response(QXmlStreamReader &reader)
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

        if (reader.name() == QLatin1String("Items")) {
            if (reader.readNextStartElement()) {
                EwsItem item(reader);
                if (!item.isValid()) {
                    return;
                }
                mId = item[EwsItemFieldItemId].value<EwsId>();

                // Finish the Items element.
                reader.skipCurrentElement();
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}
