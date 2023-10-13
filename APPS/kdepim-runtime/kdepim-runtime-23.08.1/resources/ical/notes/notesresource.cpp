/*
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notesresource.h"

#include <KCalendarCore/Incidence>

#include <KConfigSkeleton>
#include <QStandardPaths>

using namespace Akonadi;
using namespace KCalendarCore;

static const QLatin1String sNotesType("application/x-vnd.kde.notes");

NotesResource::NotesResource(const QString &id)
    : ICalResource(id, allMimeTypes(), QStringLiteral("knotes"))
{
    KConfigSkeleton::ItemPath *item = static_cast<KConfigSkeleton::ItemPath *>(mSettings->findItem(QStringLiteral("Path")));
    if (item) {
        item->setDefaultValue(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("knotes/"));
    }
}

NotesResource::~NotesResource() = default;

QStringList NotesResource::allMimeTypes() const
{
    return {sNotesType};
}

QString NotesResource::mimeType(const KCalendarCore::IncidenceBase::Ptr &) const
{
    return sNotesType;
}

AKONADI_RESOURCE_MAIN(NotesResource)
