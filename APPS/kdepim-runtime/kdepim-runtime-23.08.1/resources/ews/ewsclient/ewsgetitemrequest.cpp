/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsgetitemrequest.h"
#include "ewsclient_debug.h"

EwsGetItemRequest::EwsGetItemRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsGetItemRequest::~EwsGetItemRequest() = default;

void EwsGetItemRequest::setItemIds(const EwsId::List &ids)
{
    mIds = ids;
}

void EwsGetItemRequest::setItemShape(const EwsItemShape &shape)
{
    mShape = shape;
}

void EwsGetItemRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("GetItem"));

    mShape.write(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ItemIds"));
    for (const EwsId &id : std::as_const(mIds)) {
        id.writeItemIds(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    qCDebugNCS(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting GetItem request (") << mIds << ")";

    prepare(reqString);

    doSend();
}

bool EwsGetItemRequest::parseResult(QXmlStreamReader &reader)
{
    mResponses.reserve(mIds.size());
    return parseResponseMessage(reader, QStringLiteral("GetItem"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsGetItemRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            const EwsItem &item = resp.item();
            const EwsId &id = item[EwsItemFieldItemId].value<EwsId>();
            qCDebugNC(EWSCLI_REQUEST_LOG)
                << QStringLiteral("Got GetItem response (id: %1, subject: %2)").arg(ewsHash(id.id()), item[EwsItemFieldSubject].toString());
        } else {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got GetItem response - %1").arg(resp.responseMessage());
        }
    }

    mResponses.append(resp);

    return true;
}

EwsGetItemRequest::Response::Response(QXmlStreamReader &reader)
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
            if (!parseItems(reader)) {
                return;
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}

bool EwsGetItemRequest::Response::parseItems(QXmlStreamReader &reader)
{
    if (reader.namespaceUri() != ewsMsgNsUri || reader.name() != QLatin1String("Items")) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected Items element (got %1).").arg(reader.qualifiedName().toString()));
    }

    if (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            return setErrorMsg(QStringLiteral("Failed to read EWS request - expected child element from types namespace."));
        }

        EwsItem item(reader);
        if (!item.isValid()) {
            return setErrorMsg(QStringLiteral("Failed to read EWS request - invalid Item element."));
        }
        mItem = item;

        // Finish the Items element.
        reader.skipCurrentElement();
    }
    return true;
}
