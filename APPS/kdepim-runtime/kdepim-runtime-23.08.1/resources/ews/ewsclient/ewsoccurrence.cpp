/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsoccurrence.h"

#include <QSharedData>
#include <QXmlStreamReader>

#include "ewsclient_debug.h"
#include "ewsid.h"

class EwsOccurrencePrivate : public QSharedData
{
public:
    EwsOccurrencePrivate();
    virtual ~EwsOccurrencePrivate();

    bool mValid;

    EwsId mItemId;
    QDateTime mStart;
    QDateTime mEnd;
    QDateTime mOriginalStart;
};

EwsOccurrencePrivate::EwsOccurrencePrivate()
    : mValid(false)
{
}

EwsOccurrence::EwsOccurrence()
    : d(new EwsOccurrencePrivate())
{
}

EwsOccurrence::EwsOccurrence(QXmlStreamReader &reader)
    : d(new EwsOccurrencePrivate())
{
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in mailbox element:") << reader.namespaceUri();
            return;
        }

        if (reader.name() == QLatin1String("ItemId")) {
            d->mItemId = EwsId(reader);
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("Start")) {
            d->mStart = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
            if (reader.error() != QXmlStreamReader::NoError || !d->mStart.isValid()) {
                qCWarning(EWSCLI_LOG)
                    << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(QStringLiteral("Occurrence"), QStringLiteral("Start"));
                return;
            }
        } else if (reader.name() == QLatin1String("End")) {
            d->mEnd = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
            if (reader.error() != QXmlStreamReader::NoError || !d->mStart.isValid()) {
                qCWarning(EWSCLI_LOG)
                    << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(QStringLiteral("Occurrence"), QStringLiteral("End"));
                return;
            }
        } else if (reader.name() == QLatin1String("OriginalStart")) {
            d->mStart = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
            if (reader.error() != QXmlStreamReader::NoError || !d->mStart.isValid()) {
                qCWarning(EWSCLI_LOG)
                    << QStringLiteral("Failed to read %1 element - invalid %2 element.").arg(QStringLiteral("Occurrence"), QStringLiteral("OriginalStart"));
                return;
            }
        } else {
            qCWarning(EWSCLI_LOG)
                << QStringLiteral("Failed to read %1 element - unknown element: %2.").arg(QStringLiteral("Occurrence"), reader.name().toString());
            return;
        }
    }

    d->mValid = true;
}

EwsOccurrencePrivate::~EwsOccurrencePrivate()
{
}

EwsOccurrence::EwsOccurrence(const EwsOccurrence &other)
    : d(other.d)
{
}

EwsOccurrence::EwsOccurrence(EwsOccurrence &&other)
    : d(std::move(other.d))
{
}

EwsOccurrence::~EwsOccurrence()
{
}

EwsOccurrence &EwsOccurrence::operator=(const EwsOccurrence &other)
{
    d = other.d;
    return *this;
}

EwsOccurrence &EwsOccurrence::operator=(EwsOccurrence &&other)
{
    d = std::move(other.d);
    return *this;
}

bool EwsOccurrence::isValid() const
{
    return d->mValid;
}

const EwsId &EwsOccurrence::itemId() const
{
    return d->mItemId;
}

QDateTime EwsOccurrence::start() const
{
    return d->mStart;
}

QDateTime EwsOccurrence::end() const
{
    return d->mEnd;
}

QDateTime EwsOccurrence::originalStart() const
{
    return d->mOriginalStart;
}
