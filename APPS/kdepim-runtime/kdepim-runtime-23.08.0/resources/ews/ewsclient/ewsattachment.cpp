/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsattachment.h"

#include <QBitArray>

#include "ewsclient_debug.h"
#include "ewsxml.h"

class EwsAttachmentPrivate : public QSharedData
{
public:
    EwsAttachmentPrivate();
    ~EwsAttachmentPrivate();

    enum Field {
        Id = 0,
        Name,
        ContentType,
        ContentId,
        ContentLocation,
        Size,
        LastModifiedTime,
        IsInline,
        IsContactPhoto,
        Content,
        Item,
        NumFields,
    };

    EwsAttachment::Type mType;
    QString mId;
    QString mName;
    QString mContentType;
    QString mContentId;
    QString mContentLocation;
    long mSize = 0;
    QDateTime mLastModifiedTime;
    bool mIsInline = false;
    bool mIsContactPhoto = false;
    QByteArray mContent;
    EwsItem mItem;
    bool mValid = false;
    QBitArray mValidFields;
};

EwsAttachmentPrivate::EwsAttachmentPrivate()
    : mType(EwsAttachment::UnknownAttachment)
    , mValidFields(NumFields)
{
}

EwsAttachmentPrivate::~EwsAttachmentPrivate()
{
}

EwsAttachment::EwsAttachment()
    : d(new EwsAttachmentPrivate())
{
}

EwsAttachment::~EwsAttachment()
{
}

EwsAttachment::EwsAttachment(QXmlStreamReader &reader)
    : d(new EwsAttachmentPrivate())
{
    bool ok = true;

    if (reader.namespaceUri() != ewsTypeNsUri) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in Attachment element:") << reader.namespaceUri();
        reader.skipCurrentElement();
        return;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QStringRef readerName = reader.name();
#else
    const QStringView readerName = reader.name();
#endif
    if (readerName == QLatin1String("ItemAttachment")) {
        d->mType = ItemAttachment;
    } else if (readerName == QLatin1String("FileAttachment")) {
        d->mType = FileAttachment;
    } else if (readerName == QLatin1String("ReferenceAttachment")) {
        d->mType = ReferenceAttachment;
    } else {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unknown attachment type %1").arg(readerName.toString());
        ok = false;
    }

    // Skip this attachment type as it's not clearly documented.
    if (d->mType == ReferenceAttachment) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Attachment type ReferenceAttachment not fully supported");
        reader.skipCurrentElement();
        d->mValid = true;
        return;
    }

    while (ok && reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in Attachment element:") << reader.namespaceUri();
            reader.skipCurrentElement();
            ok = false;
            break;
        }

        const QString elmName = reader.name().toString();
        if (elmName == QLatin1String("AttachmentId")) {
            QXmlStreamAttributes attrs = reader.attributes();
            if (!attrs.hasAttribute(QStringLiteral("Id"))) {
                qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - missing Id in AttachmentId element.").arg(QStringLiteral("Attachment"));
                reader.skipCurrentElement();
                ok = false;
            } else {
                d->mId = attrs.value(QStringLiteral("Id")).toString();
                d->mValidFields.setBit(EwsAttachmentPrivate::Id);
            }
            reader.skipCurrentElement();
        } else if (elmName == QLatin1String("Name")) {
            d->mName = readXmlElementValue<QString>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::Name, ok);
        } else if (elmName == QLatin1String("ContentType")) {
            d->mContentType = readXmlElementValue<QString>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::ContentType, ok);
        } else if (elmName == QLatin1String("ContentId")) {
            d->mContentId = readXmlElementValue<QString>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::ContentId, ok);
        } else if (elmName == QLatin1String("ContentLocation")) {
            d->mContentLocation = readXmlElementValue<QString>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::ContentLocation, ok);
        } else if (elmName == QLatin1String("AttachmentOriginalUrl")) {
            // Ignore
            reader.skipCurrentElement();
        } else if (elmName == QLatin1String("Size")) {
            d->mSize = readXmlElementValue<long>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::Size, ok);
        } else if (elmName == QLatin1String("LastModifiedTime")) {
            d->mLastModifiedTime = readXmlElementValue<QDateTime>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::LastModifiedTime, ok);
        } else if (elmName == QLatin1String("IsInline")) {
            d->mIsInline = readXmlElementValue<bool>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::IsInline, ok);
        } else if (d->mType == FileAttachment && elmName == QLatin1String("IsContactPhoto")) {
            d->mIsContactPhoto = readXmlElementValue<bool>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::IsContactPhoto, ok);
        } else if (d->mType == FileAttachment && elmName == QLatin1String("Content")) {
            d->mContent = readXmlElementValue<QByteArray>(reader, ok, QStringLiteral("Attachment"));
            d->mValidFields.setBit(EwsAttachmentPrivate::Content, ok);
        } else if (d->mType == ItemAttachment
                   && (elmName == QLatin1String("Item") || elmName == QLatin1String("Message") || elmName == QLatin1String("CalendarItem")
                       || elmName == QLatin1String("Contact") || elmName == QLatin1String("MeetingMessage") || elmName == QLatin1String("MeetingRequest")
                       || elmName == QLatin1String("MeetingResponse") || elmName == QLatin1String("MeetingCancellation") || elmName == QLatin1String("Task"))) {
            d->mItem = EwsItem(reader);
            if (!d->mItem.isValid()) {
                qCWarningNC(EWSCLI_LOG)
                    << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(QStringLiteral("Attachment"), QStringLiteral("Item"));
                reader.skipCurrentElement();
                ok = false;
            } else {
                d->mValidFields.setBit(EwsAttachmentPrivate::Item);
            }
        } else {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - unknown %2 element.").arg(QStringLiteral("Attachment"), elmName);
            reader.skipCurrentElement();
            ok = false;
        }
    }

    if (!ok) {
        reader.skipCurrentElement();
    }
    d->mValid = ok;
}

EwsAttachment::EwsAttachment(const EwsAttachment &other)
    : d(other.d)
{
}

EwsAttachment::EwsAttachment(EwsAttachment &&other)
    : d(std::move(other.d))
{
}

EwsAttachment &EwsAttachment::operator=(EwsAttachment &&other)
{
    d = std::move(other.d);
    return *this;
}

EwsAttachment &EwsAttachment::operator=(const EwsAttachment &other)
{
    d = other.d;
    return *this;
}

void EwsAttachment::write(QXmlStreamWriter &writer) const
{
    QString elmName;
    switch (d->mType) {
    case ItemAttachment:
        elmName = QStringLiteral("ItemAttachment");
        break;
    case FileAttachment:
        elmName = QStringLiteral("FileAttachment");
        break;
    case ReferenceAttachment:
        elmName = QStringLiteral("ReferenceAttachment");
        break;
    default:
        qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to write Attachment element - invalid attachment type.");
        return;
    }
    writer.writeStartElement(ewsTypeNsUri, elmName);

    if (d->mType == ReferenceAttachment) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Attachment type ReferenceAttachment not fully supported");
        writer.writeEndElement();
        return;
    }

    if (d->mValidFields[EwsAttachmentPrivate::Id]) {
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("AttachmentId"));
        writer.writeAttribute(QStringLiteral("Id"), d->mId);
        writer.writeEndElement();
    }
    if (d->mValidFields[EwsAttachmentPrivate::Name]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("Name"), d->mName);
    }
    if (d->mValidFields[EwsAttachmentPrivate::ContentType]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("ContentType"), d->mContentType);
    }
    if (d->mValidFields[EwsAttachmentPrivate::ContentId]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("ContentId"), d->mContentId);
    }
    if (d->mValidFields[EwsAttachmentPrivate::ContentLocation]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("ContentLocation"), d->mContentLocation);
    }
    if (d->mValidFields[EwsAttachmentPrivate::Size]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("Size"), QString::number(d->mSize));
    }
    if (d->mValidFields[EwsAttachmentPrivate::LastModifiedTime]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("LastModifiedTime"), d->mLastModifiedTime.toString(Qt::ISODate));
    }
    if (d->mValidFields[EwsAttachmentPrivate::IsInline]) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("IsInline"), d->mIsInline ? QStringLiteral("true") : QStringLiteral("false"));
    }
    if (d->mType == FileAttachment) {
        if (d->mValidFields[EwsAttachmentPrivate::IsContactPhoto]) {
            writer.writeTextElement(ewsTypeNsUri, QStringLiteral("IsContactPhoto"), d->mIsContactPhoto ? QStringLiteral("true") : QStringLiteral("false"));
        }
        if (d->mValidFields[EwsAttachmentPrivate::Content]) {
            writer.writeTextElement(ewsTypeNsUri, QStringLiteral("Content"), QString::fromLatin1(d->mContent.toBase64()));
        }
    } else if (d->mType == ItemAttachment) {
        if (d->mValidFields[EwsAttachmentPrivate::Item]) {
            d->mItem.write(writer);
        }
    }

    writer.writeEndElement();
}

bool EwsAttachment::isValid() const
{
    return d->mValid;
}

EwsAttachment::Type EwsAttachment::type() const
{
    return d->mType;
}

void EwsAttachment::setType(Type type)
{
    d->mType = type;
}

QString EwsAttachment::id() const
{
    return d->mId;
}

void EwsAttachment::setId(const QString &id)
{
    d->mId = id;
    d->mValidFields.setBit(EwsAttachmentPrivate::Id);
}

void EwsAttachment::resetId()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::Id);
}

bool EwsAttachment::hasId() const
{
    return d->mValidFields[EwsAttachmentPrivate::Id];
}

QString EwsAttachment::name() const
{
    return d->mName;
}

void EwsAttachment::setName(const QString &name)
{
    d->mName = name;
    d->mValidFields.setBit(EwsAttachmentPrivate::Name);
}

void EwsAttachment::resetName()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::Name);
}

bool EwsAttachment::hasName() const
{
    return d->mValidFields[EwsAttachmentPrivate::Name];
}

QString EwsAttachment::contentType() const
{
    return d->mContentType;
}

void EwsAttachment::setContentType(const QString &contentType)
{
    d->mContentType = contentType;
    d->mValidFields.setBit(EwsAttachmentPrivate::ContentType);
}

void EwsAttachment::resetContentType()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::ContentType);
}

bool EwsAttachment::hasContentType() const
{
    return d->mValidFields[EwsAttachmentPrivate::ContentType];
}

QString EwsAttachment::contentId() const
{
    return d->mContentId;
}

void EwsAttachment::setContentId(const QString &contentId)
{
    d->mContentId = contentId;
    d->mValidFields.setBit(EwsAttachmentPrivate::ContentId);
}

void EwsAttachment::resetContentId()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::ContentId);
}

bool EwsAttachment::hasContentId() const
{
    return d->mValidFields[EwsAttachmentPrivate::ContentId];
}

QString EwsAttachment::contentLocation() const
{
    return d->mContentLocation;
}

void EwsAttachment::setContentLocation(const QString &contentLocation)
{
    d->mContentLocation = contentLocation;
    d->mValidFields.setBit(EwsAttachmentPrivate::ContentLocation);
}

void EwsAttachment::resetContentLocation()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::ContentLocation);
}

bool EwsAttachment::hasContentLocation() const
{
    return d->mValidFields[EwsAttachmentPrivate::ContentLocation];
}

long EwsAttachment::size() const
{
    return d->mSize;
}

void EwsAttachment::setSize(long size)
{
    d->mSize = size;
    d->mValidFields.setBit(EwsAttachmentPrivate::Size);
}

void EwsAttachment::resetSize()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::Size);
}

bool EwsAttachment::hasSize() const
{
    return d->mValidFields[EwsAttachmentPrivate::Size];
}

QDateTime EwsAttachment::lastModifiedTime() const
{
    return d->mLastModifiedTime;
}

void EwsAttachment::setLastModifiedTime(const QDateTime &time)
{
    d->mLastModifiedTime = time;
    d->mValidFields.setBit(EwsAttachmentPrivate::LastModifiedTime);
}

void EwsAttachment::resetLastModifiedTime()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::LastModifiedTime);
}

bool EwsAttachment::hasLastModifiedTime() const
{
    return d->mValidFields[EwsAttachmentPrivate::LastModifiedTime];
}

bool EwsAttachment::isInline() const
{
    return d->mIsInline;
}

void EwsAttachment::setIsInline(bool isInline)
{
    d->mIsInline = isInline;
    d->mValidFields.setBit(EwsAttachmentPrivate::IsInline);
}

void EwsAttachment::resetIsInline()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::IsInline);
}

bool EwsAttachment::hasIsInline() const
{
    return d->mValidFields[EwsAttachmentPrivate::IsInline];
}

bool EwsAttachment::isContactPhoto() const
{
    return d->mIsContactPhoto;
}

void EwsAttachment::setIsContactPhoto(bool isContactPhoto)
{
    d->mIsContactPhoto = isContactPhoto;
    d->mValidFields.setBit(EwsAttachmentPrivate::IsContactPhoto);
}

void EwsAttachment::resetIsContactPhoto()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::IsContactPhoto);
}

bool EwsAttachment::hasIsContactPhoto() const
{
    return d->mValidFields[EwsAttachmentPrivate::IsContactPhoto];
}

QByteArray EwsAttachment::content() const
{
    return d->mContent;
}

void EwsAttachment::setContent(const QByteArray &content)
{
    d->mContent = content;
    d->mValidFields.setBit(EwsAttachmentPrivate::Content);
}

void EwsAttachment::resetContent()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::Content);
}

bool EwsAttachment::hasContent() const
{
    return d->mValidFields[EwsAttachmentPrivate::Content];
}

const EwsItem &EwsAttachment::item() const
{
    return d->mItem;
}

void EwsAttachment::setItem(const EwsItem &item)
{
    d->mItem = item;
    d->mValidFields.setBit(EwsAttachmentPrivate::Item);
}

void EwsAttachment::resetItem()
{
    d->mValidFields.clearBit(EwsAttachmentPrivate::Item);
}

bool EwsAttachment::hasItem() const
{
    return d->mValidFields[EwsAttachmentPrivate::Item];
}
