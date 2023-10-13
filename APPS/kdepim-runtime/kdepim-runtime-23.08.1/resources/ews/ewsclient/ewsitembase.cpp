/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitembase.h"
#include "ewsitembase_p.h"

#include "ewsclient_debug.h"

EwsItemBasePrivate::EwsItemBasePrivate()
    : mValid(false)
{
    qRegisterMetaType<EwsItemBasePrivate::PropertyHash>();
}

EwsItemBasePrivate::~EwsItemBasePrivate() = default;

EwsItemBase::EwsItemBase(const QSharedDataPointer<EwsItemBasePrivate> &priv)
    : d(priv)
{
}

EwsItemBase::EwsItemBase(const EwsItemBase &other)
    : d(other.d)
{
}

EwsItemBase::EwsItemBase(EwsItemBase &&other)
    : d(std::move(other.d))
{
}

EwsItemBase::~EwsItemBase()
{
}

bool EwsItemBase::isValid() const
{
    return d->mValid;
}

bool EwsItemBasePrivate::extendedPropertyReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsPropertyField prop;
    QVariant value;
    PropertyHash propHash = val.value<PropertyHash>();
    QString elmName = reader.name().toString();

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid namespace.").arg(elmName);
            reader.skipCurrentElement();
            return false;
        }

        if (reader.name() == QLatin1String("FieldURI") || reader.name() == QLatin1String("IndexedFieldURI")
            || reader.name() == QLatin1String("ExtendedFieldURI")) {
            if (!prop.read(reader)) {
                reader.skipCurrentElement();
                return false;
            }
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("Value")) {
            value = reader.readElementText();
        } else if (reader.name() == QLatin1String("Values")) {
            QStringList values;
            while (reader.readNextStartElement()) {
                if (reader.namespaceUri() != ewsTypeNsUri) {
                    qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid namespace.").arg(elmName);
                    reader.skipCurrentElement();
                    reader.skipCurrentElement();
                    return false;
                }

                if (reader.name() == QLatin1String("Value")) {
                    values.append(reader.readElementText());
                }
            }
            value = values;
        } else {
            qCWarningNC(EWSCLI_LOG)
                << QStringLiteral("Failed to read %1 element - unexpected child element %2").arg(elmName, reader.qualifiedName().toString());
            reader.skipCurrentElement();
            return false;
        }
    }
    propHash.insert(prop, value);

    val = QVariant::fromValue<PropertyHash>(propHash);

    return true;
}

bool EwsItemBasePrivate::extendedPropertyWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    PropertyHash propHash = val.value<PropertyHash>();
    PropertyHash::const_iterator it;
    for (it = propHash.cbegin(); it != propHash.cend();) {
        /* EwsXml will already start the ExtendedProperty element expecting a single value.
         * This is not exactly true for extended properties as there may be many. Work around this
         * by avoiding writing the first element start tag and the last element end tag. */
        if (it != propHash.cbegin()) {
            writer.writeStartElement(ewsTypeNsUri, QStringLiteral("ExtendedProperty"));
        }
        it.key().write(writer);
        it.key().writeExtendedValue(writer, it.value());
        it++;
        if (it != propHash.cend()) {
            writer.writeEndElement();
        }
    }

    return true;
}

bool EwsItemBasePrivate::operator==(const EwsItemBasePrivate &other) const
{
    if (mValid != other.mValid) {
        return false;
    }

    return mFields == other.mFields;
}

QVariant EwsItemBase::operator[](const EwsPropertyField &prop) const
{
    EwsItemBasePrivate::PropertyHash propHash = d->mFields[EwsItemFieldExtendedProperties].value<EwsItemBasePrivate::PropertyHash>();
    EwsItemBasePrivate::PropertyHash::iterator it = propHash.find(prop);
    if (it != propHash.end()) {
        return it.value();
    } else {
        return QVariant();
    }
}

bool EwsItemBase::hasField(EwsItemFields f) const
{
    return d->mFields.contains(f);
}

QVariant EwsItemBase::operator[](EwsItemFields f) const
{
    if (hasField(f)) {
        return d->mFields[f];
    } else {
        return QVariant();
    }
}

void EwsItemBase::setField(EwsItemFields f, const QVariant &value)
{
    d->mFields[f] = value;
}

void EwsItemBase::setProperty(const EwsPropertyField &prop, const QVariant &value)
{
    EwsItemBasePrivate::PropertyHash propHash = d->mFields[EwsItemFieldExtendedProperties].value<EwsItemBasePrivate::PropertyHash>();
    propHash[prop] = value;
    d->mFields[EwsItemFieldExtendedProperties] = QVariant::fromValue<EwsItemBasePrivate::PropertyHash>(propHash);
}
