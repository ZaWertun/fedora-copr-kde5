/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonotesresource.h"

AkonotesResource::AkonotesResource(const QString &id)
    : MaildirResource(id)
{
}

AkonotesResource::~AkonotesResource() = default;

QString AkonotesResource::itemMimeType() const
{
    return QStringLiteral("text/x-vnd.akonadi.note");
}

QString AkonotesResource::defaultResourceType()
{
    return QStringLiteral("notes");
}

AKONADI_RESOURCE_MAIN(AkonotesResource)
