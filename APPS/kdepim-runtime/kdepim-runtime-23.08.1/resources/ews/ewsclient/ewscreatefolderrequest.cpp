/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscreatefolderrequest.h"

#include "ewsclient_debug.h"

EwsCreateFolderRequest::EwsCreateFolderRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsCreateFolderRequest::~EwsCreateFolderRequest()
{
}

void EwsCreateFolderRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("CreateFolder"));

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ParentFolderId"));
    mParentFolderId.writeFolderIds(writer);
    writer.writeEndElement();

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("Folders"));
    for (const EwsFolder &folder : std::as_const(mFolders)) {
        folder.write(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting CreateFolder request (%1 folders, parent %2)").arg(mFolders.size()).arg(mParentFolderId.id());

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsCreateFolderRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("CreateFolder"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsCreateFolderRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            qCDebug(EWSCLI_REQUEST_LOG) << QStringLiteral("Got CreateFolder response - OK");
        } else {
            qCDebug(EWSCLI_REQUEST_LOG) << QStringLiteral("Got CreateFolder response - %1").arg(resp.responseMessage());
        }
    }
    mResponses.append(resp);
    return true;
}

EwsCreateFolderRequest::Response::Response(QXmlStreamReader &reader)
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
