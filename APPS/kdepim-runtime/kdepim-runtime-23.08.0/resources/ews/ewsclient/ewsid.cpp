/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsid.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "ewsclient.h"
#include "ewsclient_debug.h"

static const QString distinguishedIdNames[] = {
    QStringLiteral("calendar"),
    QStringLiteral("contacts"),
    QStringLiteral("deleteditems"),
    QStringLiteral("drafts"),
    QStringLiteral("inbox"),
    QStringLiteral("journal"),
    QStringLiteral("notes"),
    QStringLiteral("outbox"),
    QStringLiteral("sentitems"),
    QStringLiteral("tasks"),
    QStringLiteral("msgfolderroot"),
    QStringLiteral("root"),
    QStringLiteral("junkemail"),
    QStringLiteral("searchfolders"),
    QStringLiteral("voicemail"),
    QStringLiteral("recoverableitemsroot"),
    QStringLiteral("recoverableitemsdeletions"),
    QStringLiteral("recoverableitemsversions"),
    QStringLiteral("recoverableitemspurges"),
    QStringLiteral("archiveroot"),
    QStringLiteral("archivemsgfolderroot"),
    QStringLiteral("archivedeleteditems"),
    QStringLiteral("archiverecoverableitemsroot"),
    QStringLiteral("archiverecoverableitemsdeletions"),
    QStringLiteral("archiverecoverableitemsversions"),
    QStringLiteral("archiverecoverableitemspurges"),
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class EwsIdComparatorRegistrar
{
public:
    EwsIdComparatorRegistrar()
    {
        QMetaType::registerComparators<EwsId>();
    }
};
const EwsIdComparatorRegistrar ewsIdComparatorRegistrar;
#endif
EwsId::EwsId(QXmlStreamReader &reader)
    : mDid(EwsDIdCalendar)
{
    // Don't check for this element's name as a folder id may be contained in several elements
    // such as "FolderId" or "ParentFolderId".
    const QXmlStreamAttributes &attrs = reader.attributes();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStringRef idRef = attrs.value(QStringLiteral("Id"));
    QStringRef changeKeyRef = attrs.value(QStringLiteral("ChangeKey"));
#else
    QStringView idRef = attrs.value(QStringLiteral("Id"));
    QStringView changeKeyRef = attrs.value(QStringLiteral("ChangeKey"));
#endif

    if (idRef.isNull()) {
        return;
    }

    mId = idRef.toString();
    if (!changeKeyRef.isNull()) {
        mChangeKey = changeKeyRef.toString();
    }
    mType = Real;
}

EwsId::EwsId(const QString &id, const QString &changeKey)
    : mType(Real)
    , mId(id)
    , mChangeKey(changeKey)
    , mDid(EwsDIdCalendar)
{
}

EwsId &EwsId::operator=(const EwsId &other)
{
    mType = other.mType;
    if (mType == Distinguished) {
        mDid = other.mDid;
    } else if (mType == Real) {
        mId = other.mId;
        mChangeKey = other.mChangeKey;
    }
    return *this;
}

EwsId &EwsId::operator=(EwsId &&other)
{
    mType = other.mType;
    if (mType == Distinguished) {
        mDid = other.mDid;
    } else if (mType == Real) {
        mId = std::move(other.mId);
        mChangeKey = std::move(other.mChangeKey);
    }
    return *this;
}

bool EwsId::operator==(const EwsId &other) const
{
    if (mType != other.mType) {
        return false;
    }

    if (mType == Distinguished) {
        return mDid == other.mDid;
    } else if (mType == Real) {
        return mId == other.mId && mChangeKey == other.mChangeKey;
    }
    return true;
}

bool EwsId::operator<(const EwsId &other) const
{
    if (mType != other.mType) {
        return mType < other.mType;
    }

    if (mType == Distinguished) {
        return mDid < other.mDid;
    } else if (mType == Real) {
        return mId < other.mId && mChangeKey < other.mChangeKey;
    }
    return false;
}

void EwsId::writeFolderIds(QXmlStreamWriter &writer) const
{
    if (mType == Distinguished) {
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("DistinguishedFolderId"));
        writer.writeAttribute(QStringLiteral("Id"), distinguishedIdNames[mDid]);
        writer.writeEndElement();
    } else if (mType == Real) {
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("FolderId"));
        writer.writeAttribute(QStringLiteral("Id"), mId);
        if (!mChangeKey.isEmpty()) {
            writer.writeAttribute(QStringLiteral("ChangeKey"), mChangeKey);
        }
        writer.writeEndElement();
    }
}

void EwsId::writeItemIds(QXmlStreamWriter &writer) const
{
    if (mType == Real) {
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("ItemId"));
        writer.writeAttribute(QStringLiteral("Id"), mId);
        if (!mChangeKey.isEmpty()) {
            writer.writeAttribute(QStringLiteral("ChangeKey"), mChangeKey);
        }
        writer.writeEndElement();
    }
}

void EwsId::writeAttributes(QXmlStreamWriter &writer) const
{
    if (mType == Real) {
        writer.writeAttribute(QStringLiteral("Id"), mId);
        if (!mChangeKey.isEmpty()) {
            writer.writeAttribute(QStringLiteral("ChangeKey"), mChangeKey);
        }
    }
}

QDebug operator<<(QDebug debug, const EwsId &id)
{
    QDebugStateSaver saver(debug);
    QDebug d = debug.nospace().noquote();
    d << QStringLiteral("EwsId(");

    switch (id.mType) {
    case EwsId::Distinguished:
        d << QStringLiteral("Distinguished: ") << distinguishedIdNames[id.mDid];
        break;
    case EwsId::Real: {
        QString name = EwsClient::folderHash.value(id.mId, ewsHash(id.mId));
        d << name << QStringLiteral(", ") << ewsHash(id.mChangeKey);
        break;
    }
    default:
        break;
    }
    d << ')';
    return debug;
}

uint qHash(const EwsId &id, uint seed)
{
    return qHash(id.id(), seed) ^ qHash(id.changeKey(), seed) ^ static_cast<uint>(id.type());
}
