/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmovefolderrequest.h"
#include "ewsclient_debug.h"

EwsMoveFolderRequest::EwsMoveFolderRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsMoveFolderRequest::~EwsMoveFolderRequest() = default;

void EwsMoveFolderRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("MoveFolder"));

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ToFolderId"));
    mDestFolderId.writeFolderIds(writer);
    writer.writeEndElement();

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("FolderIds"));
    for (const EwsId &id : std::as_const(mIds)) {
        id.writeFolderIds(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting MoveFolder request (%1 folders, to %2)").arg(mIds.size()).arg(mDestFolderId.id());

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsMoveFolderRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("MoveFolder"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsMoveFolderRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            qCDebug(EWSCLI_REQUEST_LOG) << QStringLiteral("Got MoveFolder response - OK");
        } else {
            qCDebug(EWSCLI_REQUEST_LOG) << QStringLiteral("Got MoveFolder response - %1").arg(resp.responseMessage());
        }
    }
    mResponses.append(resp);
    return true;
}

EwsMoveFolderRequest::Response::Response(QXmlStreamReader &reader)
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

        if (reader.name() == QLatin1String("Folders")) {
            if (reader.readNextStartElement()) {
                EwsFolder folder(reader);
                if (!folder.isValid()) {
                    return;
                }
                mId = folder[EwsFolderFieldFolderId].value<EwsId>();

                // Finish the Folders element.
                reader.skipCurrentElement();
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}
