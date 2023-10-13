// SPDX-FileCopyrightText: 2011 Daniel Vrátil <dvratil@redhat.com>
// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "googlescopes.h"
#include <KGAPI/Account>

using namespace KGAPI2;

QList<QUrl> googleScopes()
{
    // TODO: determine it based on what user wants?
    return {
        Account::accountInfoScopeUrl(),
        Account::calendarScopeUrl(),
        Account::peopleScopeUrl(),
        Account::tasksScopeUrl(),
    };
}
