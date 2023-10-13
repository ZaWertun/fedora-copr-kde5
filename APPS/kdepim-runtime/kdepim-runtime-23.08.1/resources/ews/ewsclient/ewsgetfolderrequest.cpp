/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsgetfolderrequest.h"

#include <QXmlStreamWriter>

#include "ewsclient_debug.h"

EwsGetFolderRequest::EwsGetFolderRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsGetFolderRequest::~EwsGetFolderRequest() = default;

void EwsGetFolderRequest::setFolderIds(const EwsId::List &ids)
{
    mIds = ids;
}

void EwsGetFolderRequest::setFolderShape(const EwsFolderShape &shape)
{
    mShape = shape;
}

void EwsGetFolderRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("GetFolder"));

    mShape.write(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("FolderIds"));
    for (const EwsId &id : std::as_const(mIds)) {
        id.writeFolderIds(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    qCDebugNCS(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting GetFolder request (") << mIds << ")";

    prepare(reqString);

    doSend();
}

bool EwsGetFolderRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("GetFolder"), [this](QXmlStreamReader &reader) {
        return parseFoldersResponse(reader);
    });
}

bool EwsGetFolderRequest::parseFoldersResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    mResponses.append(resp);
    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            const EwsFolder &folder = resp.folder();
            const EwsId &id = folder[EwsFolderFieldFolderId].value<EwsId>();
            qCDebugNC(EWSCLI_REQUEST_LOG)
                << QStringLiteral("Got GetFolder response (id: %1, name: %2)").arg(ewsHash(id.id()), folder[EwsFolderFieldDisplayName].toString());
        } else {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got GetFolder response - %1").arg(resp.responseMessage());
        }
    }

    QVariant dn = resp.folder()[EwsFolderFieldDisplayName];
    if (!dn.isNull()) {
        EwsClient::folderHash[resp.folder()[EwsFolderFieldFolderId].value<EwsId>().id()] = dn.toString();
    }

    return true;
}

EwsGetFolderRequest::Response::Response(QXmlStreamReader &reader)
    : EwsRequest::Response(reader)
{
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsMsgNsUri && reader.namespaceUri() != ewsTypeNsUri) {
            setErrorMsg(QStringLiteral("Unexpected namespace in %1 element: %2").arg(QStringLiteral("ResponseMessage"), reader.namespaceUri().toString()));
            return;
        }

        if (reader.name() == QLatin1String("Folders")) {
            if (responseClass() == EwsResponseError) {
                // Skip empty folders element
                reader.skipCurrentElement();
            } else if (!parseFolders(reader)) {
                return;
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}

bool EwsGetFolderRequest::Response::parseFolders(QXmlStreamReader &reader)
{
    if (reader.namespaceUri() != ewsMsgNsUri || reader.name() != QLatin1String("Folders")) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected Folders element (got %1).").arg(reader.qualifiedName().toString()));
    }

    if (!reader.readNextStartElement()) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected a child element in Folders element."));
    }

    if (reader.namespaceUri() != ewsTypeNsUri) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected child element from types namespace."));
    }

    EwsFolder folder(reader);
    if (!folder.isValid()) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - invalid Folder element."));
    }
    mFolder = folder;

    // Finish the Folders element
    reader.skipCurrentElement();

    return true;
}
