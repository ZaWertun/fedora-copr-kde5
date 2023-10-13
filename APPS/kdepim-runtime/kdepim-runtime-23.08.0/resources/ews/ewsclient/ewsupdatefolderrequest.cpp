/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsupdatefolderrequest.h"
#include "ewsclient_debug.h"

static const QVector<QString> updateTypeElementNames = {
    QStringLiteral("AppendToFolderField"),
    QStringLiteral("SetFolderField"),
    QStringLiteral("DeleteFolderField"),
};

static const QVector<QString> folderTypeNames = {
    QStringLiteral("Folder"),
    QStringLiteral("CalendarFolder"),
    QStringLiteral("ContactsFolder"),
    QStringLiteral("SearchFolder"),
    QStringLiteral("TasksFolder"),
};

EwsUpdateFolderRequest::EwsUpdateFolderRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
{
}

EwsUpdateFolderRequest::~EwsUpdateFolderRequest() = default;

void EwsUpdateFolderRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("UpdateFolder"));

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("FolderChanges"));
    for (const FolderChange &ch : std::as_const(mChanges)) {
        ch.write(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting UpdateFolder request (%1 changes)").arg(mChanges.size());

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsUpdateFolderRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("UpdateFolder"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsUpdateFolderRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got UpdateFolder response - OK");
        } else {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got UpdateFolder response - %1").arg(resp.responseMessage());
        }
    }

    mResponses.append(resp);
    return true;
}

EwsUpdateFolderRequest::Response::Response(QXmlStreamReader &reader)
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
            }

            // Finish the Folders element.
            reader.skipCurrentElement();
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}

bool EwsUpdateFolderRequest::Update::write(QXmlStreamWriter &writer, EwsFolderType folderType) const
{
    bool retVal = true;

    writer.writeStartElement(ewsTypeNsUri, updateTypeElementNames[mType]);

    mField.write(writer);

    if (mType != Delete) {
        writer.writeStartElement(ewsTypeNsUri, folderTypeNames[folderType]);
        retVal = mField.writeWithValue(writer, mValue);
        writer.writeEndElement();
    }

    writer.writeEndElement();

    return retVal;
}

bool EwsUpdateFolderRequest::FolderChange::write(QXmlStreamWriter &writer) const
{
    bool retVal = true;

    writer.writeStartElement(ewsTypeNsUri, QStringLiteral("FolderChange"));

    mId.writeFolderIds(writer);

    writer.writeStartElement(ewsTypeNsUri, QStringLiteral("Updates"));

    for (const QSharedPointer<const Update> &upd : std::as_const(mUpdates)) {
        if (!upd->write(writer, mType)) {
            retVal = false;
            break;
        }
    }

    writer.writeEndElement();

    writer.writeEndElement();

    return retVal;
}
