/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QMap>

#include "ewstypes.h"

const QString soapEnvNsUri = QStringLiteral("http://schemas.xmlsoap.org/soap/envelope/");
const QString ewsMsgNsUri = QStringLiteral("http://schemas.microsoft.com/exchange/services/2006/messages");
const QString ewsTypeNsUri = QStringLiteral("http://schemas.microsoft.com/exchange/services/2006/types");

const QVector<QString> ewsItemTypeNames = {
    QStringLiteral("Item"),
    QStringLiteral("Message"),
    QStringLiteral("CalendarItem"),
    QStringLiteral("Contact"),
    QStringLiteral("DistributionList"),
    QStringLiteral("MeetingMessage"),
    QStringLiteral("MeetingRequest"),
    QStringLiteral("MeetingResponse"),
    QStringLiteral("MeetingCancellation"),
    QStringLiteral("Task"),
};

static const QMap<QString, EwsResponseCode> ewsResponseCodeMapping = {{QLatin1String("NoError"), EwsResponseCodeNoError},
                                                                      {QLatin1String("ErrorServerBusy"), EwsResponseCodeErrorServerBusy}};

EwsResponseCode decodeEwsResponseCode(const QString &code)
{
    if (ewsResponseCodeMapping.contains(code)) {
        return ewsResponseCodeMapping[code];
    } else {
        return EwsResponseCodeUnknown;
    }
}

bool isEwsResponseCodeTemporaryError(EwsResponseCode code)
{
    switch (code) {
    case EwsResponseCodeErrorServerBusy:
        /* fall through */
    case EwsResponseCodeUnauthorized:
        return true;
    default:
        return false;
    }
    return false;
}
