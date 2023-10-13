/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2012 Marco Nelles <marco.nelles@credativ.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "oxerrors.h"

using namespace OXA;

OXErrors::EditErrorID OXErrors::getEditErrorID(const QString &errorText)
{
    const int b1Pos = errorText.indexOf(QLatin1Char('['));
    const int b2Pos = errorText.indexOf(QLatin1Char(']'));
    const QString errorID = errorText.mid(b1Pos + 1, b2Pos - b1Pos - 1);

    bool ok;
    int eid = errorID.toInt(&ok);
    if (!ok) {
        return OXErrors::EditErrorUndefined;
    }

    switch (eid) {
    case 1000:
        return OXErrors::ConcurrentModification;
    case 1001:
        return OXErrors::ObjectNotFound;
    case 1002:
        return OXErrors::NoPermissionForThisAction;
    case 1003:
        return OXErrors::ConflictsDetected;
    case 1004:
        return OXErrors::MissingMandatoryFields;
    case 1006:
        return OXErrors::AppointmentConflicts;
    case 1500:
        return OXErrors::InternalServerError;
    default:;
    }

    return OXErrors::EditErrorUndefined;
}
