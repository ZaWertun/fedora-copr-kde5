/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsupdateitemrequest.h"
#include "ewsclient_debug.h"

static const QVector<QString> conflictResolutionNames = {
    QStringLiteral("NeverOverwrite"),
    QStringLiteral("AutoResolve"),
    QStringLiteral("AlwaysOverwrite"),
};

static const QVector<QString> messageDispositionNames = {
    QStringLiteral("SaveOnly"),
    QStringLiteral("SendOnly"),
    QStringLiteral("SendAndSaveCopy"),
};

static const QVector<QString> meetingDispositionNames = {
    QStringLiteral("SendToNone"),
    QStringLiteral("SendOnlyToAll"),
    QStringLiteral("SendOnlyToChanged"),
    QStringLiteral("SendToAllAndSaveCopy"),
    QStringLiteral("SendToChangedAndSaveCopy"),
};

static const QVector<QString> updateTypeElementNames = {
    QStringLiteral("AppendToItemField"),
    QStringLiteral("SetItemField"),
    QStringLiteral("DeleteItemField"),
};

EwsUpdateItemRequest::EwsUpdateItemRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
    , mMessageDisp(EwsDispSaveOnly)
    , mConflictResol(EwsResolAlwaysOverwrite)
    , mMeetingDisp(EwsMeetingDispUnspecified)
{
}

EwsUpdateItemRequest::~EwsUpdateItemRequest() = default;

void EwsUpdateItemRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("UpdateItem"));

    writer.writeAttribute(QStringLiteral("ConflictResolution"), conflictResolutionNames[mConflictResol]);

    writer.writeAttribute(QStringLiteral("MessageDisposition"), messageDispositionNames[mMessageDisp]);

    if (mMeetingDisp != EwsMeetingDispUnspecified) {
        writer.writeAttribute(QStringLiteral("SendMeetingInvitationsOrCancellations"), meetingDispositionNames[mMeetingDisp]);
    }

    if (mSavedFolderId.type() != EwsId::Unspecified) {
        writer.writeStartElement(ewsMsgNsUri, QStringLiteral("SavedItemFolderId"));
        mSavedFolderId.writeFolderIds(writer);
        writer.writeEndElement();
    }

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ItemChanges"));
    for (const ItemChange &ch : std::as_const(mChanges)) {
        ch.write(writer);
    }
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting UpdateItem request (%1 changes)").arg(mChanges.size());

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    prepare(reqString);

    doSend();
}

bool EwsUpdateItemRequest::parseResult(QXmlStreamReader &reader)
{
    mResponses.reserve(mChanges.size());
    return parseResponseMessage(reader, QStringLiteral("UpdateItem"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsUpdateItemRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    Response resp(reader);
    if (resp.responseClass() == EwsResponseUnknown) {
        return false;
    }

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp.isSuccess()) {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got UpdateItem response - OK");
        } else {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got UpdateItem response - %1").arg(resp.responseMessage());
        }
    }

    mResponses.append(resp);
    return true;
}

EwsUpdateItemRequest::Response::Response(QXmlStreamReader &reader)
    : EwsRequest::Response(reader)
    , mConflictCount(0)
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
            }

            // Finish the Items element.
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("ConflictResults")) {
            if (!reader.readNextStartElement()) {
                setErrorMsg(QStringLiteral("Failed to read EWS request - expected a %1 element inside %2 element.")
                                .arg(QStringLiteral("Value"), QStringLiteral("ConflictResults")));
                return;
            }

            if (reader.name() != QLatin1String("Count")) {
                setErrorMsg(QStringLiteral("Failed to read EWS request - expected a %1 element inside %2 element.")
                                .arg(QStringLiteral("Count"), QStringLiteral("ConflictResults")));
                return;
            }

            bool ok;
            mConflictCount = reader.readElementText().toUInt(&ok);

            if (!ok) {
                setErrorMsg(QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("ConflictResults/Value")));
            }
            // Finish the Value element.
            reader.skipCurrentElement();
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element %1.").arg(reader.name().toString()));
            return;
        }
    }
}

bool EwsUpdateItemRequest::Update::write(QXmlStreamWriter &writer, EwsItemType itemType) const
{
    bool retVal = true;

    writer.writeStartElement(ewsTypeNsUri, updateTypeElementNames[mType]);

    mField.write(writer);

    if (mType != Delete) {
        writer.writeStartElement(ewsTypeNsUri, ewsItemTypeNames[itemType]);
        retVal = mField.writeWithValue(writer, mValue);
        writer.writeEndElement();
    }

    writer.writeEndElement();

    return retVal;
}

bool EwsUpdateItemRequest::ItemChange::write(QXmlStreamWriter &writer) const
{
    bool retVal = true;

    writer.writeStartElement(ewsTypeNsUri, QStringLiteral("ItemChange"));

    mId.writeItemIds(writer);

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
