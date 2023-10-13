/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(EWSFAKE_LOG)

#define qCDebugNC(cat) qCDebug(cat).noquote()
#define qCInfoNC(cat) qCInfo(cat).noquote()
#define qCWarningNC(cat) qCWarning(cat).noquote()
#define qCCriticalNC(cat) qCCritical(cat).noquote()
