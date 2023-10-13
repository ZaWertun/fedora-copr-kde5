/*

    SPDX-FileCopyrightText: 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once
#include <KCalendarCore/IncidenceBase>
#include <QByteArray>

QByteArray mailAttendees(const KCalendarCore::IncidenceBase::Ptr &incidence, bool bccMe, const QString &attachment);

QByteArray mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &incidence, const QString &from, bool bccMe, const QString &attachment, const QString &sub);
