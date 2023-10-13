/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsattendee.h"

#include <QSharedData>
#include <QXmlStreamReader>

#include "ewsclient_debug.h"
#include "ewsmailbox.h"

class EwsAttendeePrivate : public QSharedData
{
public:
    EwsAttendeePrivate();
    virtual ~EwsAttendeePrivate();

    bool mValid;

    EwsMailbox mMailbox;
    EwsEventResponseType mResponse;
    QDateTime mResponseTime;
};

static const QString responseTypeNames[] = {QStringLiteral("Unknown"),
                                            QStringLiteral("Organizer"),
                                            QStringLiteral("Tentative"),
                                            QStringLiteral("Accept"),
                                            QStringLiteral("Decline"),
                                            QStringLiteral("NoResponseReceived")};
constexpr unsigned responseTypeNameCount = sizeof(responseTypeNames) / sizeof(responseTypeNames[0]);

EwsAttendeePrivate::EwsAttendeePrivate()
    : mValid(false)
    , mResponse(EwsEventResponseNotReceived)
{
}

EwsAttendee::EwsAttendee()
    : d(new EwsAttendeePrivate())
{
}

EwsAttendee::EwsAttendee(QXmlStreamReader &reader)
    : d(new EwsAttendeePrivate())
{
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in mailbox element:") << reader.namespaceUri();
            return;
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QStringRef readerName = reader.name();
#else
        const QStringView readerName = reader.name();
#endif
        if (readerName == QLatin1String("Mailbox")) {
            d->mMailbox = EwsMailbox(reader);
            if (!d->mMailbox.isValid()) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid attendee %1 element.").arg(readerName.toString());
                return;
            }
        } else if (readerName == QLatin1String("ResponseType")) {
            bool ok;
            d->mResponse = decodeEnumString<EwsEventResponseType>(reader.readElementText(), responseTypeNames, responseTypeNameCount, &ok);
            if (reader.error() != QXmlStreamReader::NoError || !ok) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid attendee %1 element.").arg(readerName.toString());
                return;
            }
        } else if (readerName == QLatin1String("LastResponseTime")) {
            // Unsupported - ignore
            // qCWarningNC(EWSCLIENT_LOG) << QStringLiteral("Unsupported mailbox element %1").arg(reader.name().toString());
            reader.skipCurrentElement();
        }
    }

    d->mValid = true;
}

EwsAttendeePrivate::~EwsAttendeePrivate()
{
}

EwsAttendee::EwsAttendee(const EwsAttendee &other)
    : d(other.d)
{
}

EwsAttendee::EwsAttendee(EwsAttendee &&other)
    : d(std::move(other.d))
{
}

EwsAttendee::~EwsAttendee()
{
}

EwsAttendee &EwsAttendee::operator=(const EwsAttendee &other)
{
    d = other.d;
    return *this;
}

EwsAttendee &EwsAttendee::operator=(EwsAttendee &&other)
{
    d = std::move(other.d);
    return *this;
}

bool EwsAttendee::isValid() const
{
    return d->mValid;
}

const EwsMailbox &EwsAttendee::mailbox() const
{
    return d->mMailbox;
}

EwsEventResponseType EwsAttendee::response() const
{
    return d->mResponse;
}

QDateTime EwsAttendee::responseTime() const
{
    return d->mResponseTime;
}
