/*
    SPDX-FileCopyrightText: 2006 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "icalresourcebase.h"

#include <KCalendarCore/IncidenceBase>

class ICalResource : public ICalResourceBase
{
    Q_OBJECT

public:
    explicit ICalResource(const QString &id);
    ~ICalResource() override;

protected:
    /**
     * Constructor for derived classes.
     * @param mimeTypes mimeTypes to be handled by the resource.
     * @param icon icon name to use.
     */
    ICalResource(const QString &id, const QStringList &mimeTypes, const QString &icon);

    bool doRetrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    void doRetrieveItems(const Akonadi::Collection &col) override;

    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

    /**
      Returns the Akonadi specific @c text/calendar sub MIME type of the given @p incidence.
    */
    virtual QString mimeType(const KCalendarCore::IncidenceBase::Ptr &incidence) const;

    /**
      Returns a list of all calendar component sub MIME types.
     */
    virtual QStringList allMimeTypes() const;
};
