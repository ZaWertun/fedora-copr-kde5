/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include "ewsclient.h"
#include "ewsclient_debug.h"
#include "ewsres_agentif_debug.h"
#include "ewsres_debug.h"

inline QDebug operator<<(QDebug debug, const Akonadi::Item::List &items)
{
    QDebugStateSaver saver(debug);
    QStringList itemStrs;
    itemStrs.reserve(items.count());
    for (const Akonadi::Item &item : items) {
        itemStrs.append(ewsHash(item.remoteId()));
    }
    debug.nospace().noquote() << "Akonadi::Item::List(" << itemStrs.join(QChar::fromLatin1(',')) << ")";
    return debug.maybeSpace();
}

inline QDebug operator<<(QDebug debug, const Akonadi::Item &item)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "Akonadi::Item(" << ewsHash(item.remoteId()) << ")";
    return debug.maybeSpace();
}

inline QDebug operator<<(QDebug debug, const Akonadi::Collection::List &cols)
{
    QDebugStateSaver saver(debug);
    QStringList itemStrs;
    itemStrs.reserve(cols.count());
    for (const Akonadi::Collection &col : cols) {
        itemStrs.append(EwsClient::folderHash.value(col.remoteId(), ewsHash(col.remoteId())));
    }
    debug.nospace().noquote() << "Akonadi::Collection::List(" << itemStrs.join(QLatin1Char(',')) << ")";
    return debug.maybeSpace();
}

inline QDebug operator<<(QDebug debug, const Akonadi::Collection &col)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "Akonadi::Collection(" << EwsClient::folderHash.value(col.remoteId(), ewsHash(col.remoteId())) << ")";
    return debug.maybeSpace();
}

inline QDebug operator<<(QDebug debug, const QSet<QByteArray> &items)
{
    QDebugStateSaver saver(debug);
    QStringList itemStrs;
    itemStrs.reserve(items.count());
    for (const QByteArray &item : items) {
        itemStrs.append(QString::fromLatin1(item));
    }
    debug.nospace().noquote() << "QSet<QByteArray>(" << itemStrs.join(QLatin1Char(',')) << ")";
    return debug.maybeSpace();
}
