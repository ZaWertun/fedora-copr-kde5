/*
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "icalresource.h"

class NotesResource : public ICalResource
{
    Q_OBJECT

public:
    explicit NotesResource(const QString &id);
    ~NotesResource() override;

protected:
    /**
      Returns the Akonadi specific @c text/calendar sub MIME type of the given @p incidence.
    */
    QString mimeType(const KCalendarCore::IncidenceBase::Ptr &incidence) const override;

    /**
      Returns a list of all calendar component sub MIME types.
     */
    QStringList allMimeTypes() const override;
};
