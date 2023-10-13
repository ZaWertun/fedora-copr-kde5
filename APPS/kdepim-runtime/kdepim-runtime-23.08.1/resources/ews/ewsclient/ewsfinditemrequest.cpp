/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfinditemrequest.h"

#include <memory>

#include <QXmlStreamWriter>

#include "ewsclient_debug.h"

static const QString traversalTypeNames[] = {
    QStringLiteral("Shallow"),
    QStringLiteral("Deep"),
    QStringLiteral("SoftDeleted"),
    QStringLiteral("Associated"),
};

class EwsFindItemResponse : public EwsRequest::Response
{
public:
    EwsFindItemResponse(QXmlStreamReader &reader);
    bool parseRootFolder(QXmlStreamReader &reader);
    EwsItem *readItem(QXmlStreamReader &reader);

    QList<EwsItem> mItems;
    unsigned mTotalItems;
    int mNextOffset;
    int mNextNumerator;
    int mNextDenominator;
    bool mIncludesLastItem;
};

EwsFindItemRequest::EwsFindItemRequest(EwsClient &client, QObject *parent)
    : EwsRequest(client, parent)
    , mTraversal(EwsTraversalShallow)
    , mPagination(false)
    , mPageBasePoint(EwsBasePointBeginning)
    , mPageOffset(0)
    , mFractional(false)
    , mMaxItems(-1)
    , mFracNumerator(0)
    , mFracDenominator(0)
    , mTotalItems(0)
    , mNextOffset(-1)
    , mNextNumerator(-1)
    , mNextDenominator(-1)
    , mIncludesLastItem(false)
{
}

EwsFindItemRequest::~EwsFindItemRequest()
{
}

void EwsFindItemRequest::setFolderId(const EwsId &id)
{
    mFolderId = id;
}

void EwsFindItemRequest::setItemShape(const EwsItemShape &shape)
{
    mShape = shape;
}

void EwsFindItemRequest::start()
{
    QString reqString;
    QXmlStreamWriter writer(&reqString);

    startSoapDocument(writer);

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("FindItem"));
    writer.writeAttribute(QStringLiteral("Traversal"), traversalTypeNames[mTraversal]);

    mShape.write(writer);

    if (mPagination) {
        writer.writeStartElement(ewsMsgNsUri, QStringLiteral("IndexedPageItemView"));
        if (mMaxItems > 0) {
            writer.writeAttribute(QStringLiteral("MaxEntriesReturned"), QString::number(mMaxItems));
        }
        writer.writeAttribute(QStringLiteral("Offset"), QString::number(mPageOffset));
        writer.writeAttribute(QStringLiteral("BasePoint"), (mPageBasePoint == EwsBasePointEnd) ? QStringLiteral("End") : QStringLiteral("Beginning"));
        writer.writeEndElement();
    } else if (mFractional) {
        writer.writeStartElement(ewsMsgNsUri, QStringLiteral("FractionalPageItemView"));
        if (mMaxItems > 0) {
            writer.writeAttribute(QStringLiteral("MaxEntriesReturned"), QString::number(mMaxItems));
        }
        writer.writeAttribute(QStringLiteral("Numerator"), QString::number(mFracNumerator));
        writer.writeAttribute(QStringLiteral("Denominator"), QString::number(mFracDenominator));
        writer.writeEndElement();
    }

    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ParentFolderIds"));
    mFolderId.writeFolderIds(writer);
    writer.writeEndElement();

    writer.writeEndElement();

    endSoapDocument(writer);

    qCDebug(EWSCLI_PROTO_LOG) << reqString;

    qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Starting FindItems request (folder: ") << mFolderId << QStringLiteral(")");

    prepare(reqString);

    doSend();
}

bool EwsFindItemRequest::parseResult(QXmlStreamReader &reader)
{
    return parseResponseMessage(reader, QStringLiteral("FindItem"), [this](QXmlStreamReader &reader) {
        return parseItemsResponse(reader);
    });
}

bool EwsFindItemRequest::parseItemsResponse(QXmlStreamReader &reader)
{
    auto resp = new EwsFindItemResponse(reader);
    if (resp->responseClass() == EwsResponseUnknown) {
        return false;
    }

    mItems = resp->mItems;
    mTotalItems = resp->mTotalItems;
    mNextOffset = resp->mNextOffset;
    mNextNumerator = resp->mNextNumerator;
    mNextDenominator = resp->mNextDenominator;
    mIncludesLastItem = resp->mIncludesLastItem;

    if (EWSCLI_REQUEST_LOG().isDebugEnabled()) {
        if (resp->isSuccess()) {
            qCDebugNC(EWSCLI_REQUEST_LOG) << QStringLiteral("Got FindItems response (%1 items, last included: %2)")
                                                 .arg(mItems.size())
                                                 .arg(mIncludesLastItem ? QStringLiteral("true") : QStringLiteral("false"));
        } else {
            qCDebug(EWSCLI_REQUEST_LOG) << QStringLiteral("Got FindItems response - %1").arg(resp->responseMessage());
        }
    }

    return true;
}

EwsFindItemResponse::EwsFindItemResponse(QXmlStreamReader &reader)
    : EwsRequest::Response(reader)
{
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsMsgNsUri && reader.namespaceUri() != ewsTypeNsUri) {
            setErrorMsg(QStringLiteral("Unexpected namespace in %1 element: %2").arg(QStringLiteral("ResponseMessage"), reader.namespaceUri().toString()));
            return;
        }

        if (reader.name() == QLatin1String("RootFolder")) {
            if (!parseRootFolder(reader)) {
                return;
            }
        } else if (!readResponseElement(reader)) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid response element."));
            return;
        }
    }
}

bool EwsFindItemResponse::parseRootFolder(QXmlStreamReader &reader)
{
    if (reader.namespaceUri() != ewsMsgNsUri || reader.name() != QLatin1String("RootFolder")) {
        return setErrorMsg(
            QStringLiteral("Failed to read EWS request - expected %1 element (got %2).").arg(QStringLiteral("RootFolder"), reader.qualifiedName().toString()));
    }

    if (!reader.attributes().hasAttribute(QStringLiteral("TotalItemsInView")) || !reader.attributes().hasAttribute(QStringLiteral("TotalItemsInView"))) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - missing attributes of %1 element.").arg(QStringLiteral("RootFolder")));
    }
    bool ok;
    QXmlStreamAttributes attrs = reader.attributes();
    mTotalItems = attrs.value(QStringLiteral("TotalItemsInView")).toUInt(&ok);
    if (!ok) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - failed to read %1 attribute.").arg(QStringLiteral("TotalItemsInView")));
    }
    mIncludesLastItem = attrs.value(QStringLiteral("IncludesLastItemInRange")) == QLatin1String("true");

    if (attrs.hasAttribute(QStringLiteral("IndexedPagingOffset"))) {
        mNextOffset = attrs.value(QStringLiteral("IndexedPagingOffset")).toInt(&ok);
        if (!ok) {
            return setErrorMsg(QStringLiteral("Failed to read EWS request - failed to read %1 attribute.").arg(QStringLiteral("IndexedPagingOffset")));
        }
    }

    if (attrs.hasAttribute(QStringLiteral("NumeratorOffset"))) {
        mNextNumerator = attrs.value(QStringLiteral("NumeratorOffset")).toInt(&ok);
        if (!ok) {
            return setErrorMsg(QStringLiteral("Failed to read EWS request - failed to read %1 attribute.").arg(QStringLiteral("NumeratorOffset")));
        }
    }

    if (attrs.hasAttribute(QStringLiteral("AbsoluteDenominator"))) {
        mNextDenominator = attrs.value(QStringLiteral("AbsoluteDenominator")).toInt(&ok);
        if (!ok) {
            return setErrorMsg(QStringLiteral("Failed to read EWS request - failed to read %1 attribute.").arg(QStringLiteral("AbsoluteDenominator")));
        }
    }

    if (!reader.readNextStartElement()) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected a child element in %1 element.").arg(QStringLiteral("RootFolder")));
    }

    if (reader.namespaceUri() != ewsTypeNsUri || reader.name() != QLatin1String("Items")) {
        return setErrorMsg(
            QStringLiteral("Failed to read EWS request - expected %1 element (got %2).").arg(QStringLiteral("Items"), reader.qualifiedName().toString()));
    }

    if (!reader.readNextStartElement()) {
        // An empty Items element means no items.
        reader.skipCurrentElement();
        return true;
    }

    if (reader.namespaceUri() != ewsTypeNsUri) {
        return setErrorMsg(QStringLiteral("Failed to read EWS request - expected child element from types namespace."));
    }

    do {
        EwsItem *item = readItem(reader);
        if (item) {
            mItems.append(*item);
        }
    } while (reader.readNextStartElement());

    // Finish the Items element
    reader.skipCurrentElement();

    // Finish the RootFolder element
    reader.skipCurrentElement();

    return true;
}

EwsItem *EwsFindItemResponse::readItem(QXmlStreamReader &reader)
{
    EwsItem *item = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QStringRef readerName = reader.name();
#else
    const QStringView readerName = reader.name();
#endif
    if (readerName == QLatin1String("Item") || readerName == QLatin1String("Message") || readerName == QLatin1String("CalendarItem")
        || readerName == QLatin1String("Contact") || readerName == QLatin1String("DistributionList") || readerName == QLatin1String("MeetingMessage")
        || readerName == QLatin1String("MeetingRequest") || readerName == QLatin1String("MeetingResponse") || readerName == QLatin1String("MeetingCancellation")
        || readerName == QLatin1String("Task")) {
        qCDebug(EWSCLI_LOG).noquote() << QStringLiteral("Processing %1").arg(readerName.toString());
        item = new EwsItem(reader);
        if (!item->isValid()) {
            setErrorMsg(QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(readerName.toString()));
            delete item;
            return nullptr;
        }
    } else {
        qCWarning(EWSCLI_LOG).noquote() << QStringLiteral("Unsupported folder type %1").arg(readerName.toString());
        reader.skipCurrentElement();
    }

    return item;
}
