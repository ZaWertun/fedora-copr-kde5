/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QVector>
#include <QXmlStreamReader>

#include "ewsclient_debug.h"

template<typename T>
class EwsXml
{
public:
    typedef std::function<bool(QXmlStreamReader &, QVariant &)> ReadFunction;
    typedef std::function<bool(QXmlStreamWriter &, const QVariant &)> WriteFunction;
    typedef std::function<bool(QXmlStreamReader &, const QString &)> UnknownElementFunction;

    typedef QHash<T, QVariant> ValueHash;

    static constexpr T Ignore = static_cast<T>(-1);

    struct Item {
        Item()
            : key(Ignore)
        {
        }

        Item(T k, const QString &n, const ReadFunction &rfn = ReadFunction(), const WriteFunction &wfn = WriteFunction())
            : key(k)
            , elmName(n)
            , readFn(rfn)
            , writeFn(wfn)
        {
        }

        T key;
        QString elmName;
        ReadFunction readFn;
        WriteFunction writeFn;
    };

    EwsXml()
    {
    }

    EwsXml(const QVector<Item> &items)
        : mItems(items)
    {
        rebuildItemHash();
    }

    EwsXml(const EwsXml &other)
        : mItems(other.mItems)
        , mValues(other.mValues)
        , mItemHash(other.mItemHash)
    {
    }

    void setItems(const QVector<Item> &items)
    {
        mItems = items;
        rebuildItemHash();
    }

    bool readItem(QXmlStreamReader &reader, const QString &parentElm, const QString &nsUri, UnknownElementFunction unknownElmFn = &defaultUnknownElmFunction)
    {
        typename QHash<QString, Item>::iterator it = mItemHash.find(reader.name().toString());
        if (it != mItemHash.end() && nsUri == reader.namespaceUri()) {
            if (it->key == Ignore) {
                qCInfoNC(EWSCLI_LOG) << QStringLiteral("Unsupported %1 child element %2 - ignoring.").arg(parentElm).arg(reader.name().toString());
                reader.skipCurrentElement();
                return true;
            } else if (!it->readFn) {
                qCWarning(EWSCLI_LOG)
                    << QStringLiteral("Failed to read %1 element - no read support for %2 element.").arg(parentElm).arg(reader.name().toString());
                return false;
            } else {
                QVariant val = mValues[it->key];
                if (it->readFn(reader, val)) {
                    mValues[it->key] = val;
                    return true;
                }
                return false;
            }
        }
        return unknownElmFn(reader, parentElm);
    }

    bool readItems(QXmlStreamReader &reader, const QString &nsUri, const UnknownElementFunction &unknownElmFn = &defaultUnknownElmFunction)
    {
        QString elmName(reader.name().toString());
        while (reader.readNextStartElement()) {
            if (!readItem(reader, elmName, nsUri, unknownElmFn)) {
                return false;
            }
        }
        return true;
    }

    bool writeItems(QXmlStreamWriter &writer,
                    const QString &parentElm,
                    const QString &nsUri,
                    const ValueHash &values,
                    const QList<T> &keysToWrite = QList<T>()) const
    {
        bool hasKeysToWrite = !keysToWrite.isEmpty();
        for (const Item &item : std::as_const(mItems)) {
            if (!hasKeysToWrite || keysToWrite.contains(item.key)) {
                typename ValueHash::const_iterator it = values.find(item.key);
                if (it != values.end()) {
                    if (!item.writeFn) {
                        qCWarning(EWSCLI_LOG)
                            << QStringLiteral("Failed to write %1 element - no write support for %2 element.").arg(parentElm).arg(item.elmName);
                        return false;
                    }
                    writer.writeStartElement(nsUri, item.elmName);
                    bool status = item.writeFn(writer, *it);
                    writer.writeEndElement();
                    if (!status) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    ValueHash values() const
    {
        return mValues;
    }

private:
    static bool defaultUnknownElmFunction(QXmlStreamReader &reader, const QString &parentElm)
    {
        qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(parentElm).arg(reader.name().toString());
        return false;
    }

    const QVector<Item> mItems;
    ValueHash mValues;
    QHash<QString, Item> mItemHash;

    void rebuildItemHash()
    {
        for (const Item &item : std::as_const(mItems)) {
            mItemHash.insert(item.elmName, item);
        }
    }
};

template<typename T>
T readXmlElementValue(QXmlStreamReader &reader, bool &ok, const QString &parentElement);

extern bool ewsXmlBoolReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlBoolWriter(QXmlStreamWriter &writer, const QVariant &val);
extern bool ewsXmlBase64Reader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlBase64Writer(QXmlStreamWriter &writer, const QVariant &val);
extern bool ewsXmlIdReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlIdWriter(QXmlStreamWriter &writer, const QVariant &val);
extern bool ewsXmlTextReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlTextWriter(QXmlStreamWriter &writer, const QVariant &val);
extern bool ewsXmlUIntReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlUIntWriter(QXmlStreamWriter &writer, const QVariant &val);
extern bool ewsXmlDateTimeReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlItemReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlFolderReader(QXmlStreamReader &reader, QVariant &val);

extern bool ewsXmlEnumReader(QXmlStreamReader &reader, QVariant &val, const QVector<QString> &items);
extern bool ewsXmlSensitivityReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlImportanceReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlCalendarItemTypeReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlLegacyFreeBusyStatusReader(QXmlStreamReader &reader, QVariant &val);
extern bool ewsXmlResponseTypeReader(QXmlStreamReader &reader, QVariant &val);
