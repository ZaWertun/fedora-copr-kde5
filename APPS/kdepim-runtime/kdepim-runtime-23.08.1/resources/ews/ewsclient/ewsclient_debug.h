/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QTemporaryDir>
#include <ewscli_debug.h>
#include <ewscli_failedreq_debug.h>
#include <ewscli_proto_debug.h>
#include <ewscli_req_debug.h>

#define qCDebugNC(cat) qCDebug(cat).noquote()
#define qCInfoNC(cat) qCInfo(cat).noquote()
#define qCWarningNC(cat) qCWarning(cat).noquote()
#define qCCriticalNC(cat) qCCritical(cat).noquote()

#define qCDebugNS(cat) qCDebug(cat).nospace()
#define qCInfoNS(cat) qCInfo(cat).nospace()
#define qCWarningNS(cat) qCWarning(cat).nospace()
#define qCCriticalNS(cat) qCCritical(cat).nospace()

#define qCDebugNCS(cat) qCDebug(cat).noquote().nospace()
#define qCInfoNCS(cat) qCInfo(cat).noquote().nospace()
#define qCWarningNCS(cat) qCWarning(cat).noquote().nospace()
#define qCCriticalNCS(cat) qCCritical(cat).noquote().nospace()

inline QString ewsHash(const QString &val)
{
    return QString::number(qHash(val), 36);
}

extern QTemporaryDir ewsLogDir;
