/*
    SPDX-FileCopyrightText: 2006 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icalresource.h"

#include <KCalendarCore/FreeBusy>

#include <KLocalizedString>
#include <QDebug>

using namespace Akonadi;
using namespace KCalendarCore;

ICalResource::ICalResource(const QString &id)
    : ICalResourceBase(id)
{
    QStringList mimeTypes;
    mimeTypes << QStringLiteral("text/calendar");
    mimeTypes += allMimeTypes();
    initialise(mimeTypes, QStringLiteral("office-calendar"));
}

ICalResource::ICalResource(const QString &id, const QStringList &mimeTypes, const QString &icon)
    : ICalResourceBase(id)
{
    initialise(mimeTypes, icon);
}

ICalResource::~ICalResource() = default;

bool ICalResource::doRetrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    Akonadi::Item::List resultItems;
    resultItems.reserve(items.size());

    for (const Akonadi::Item &item : items) {
        const QString rid = item.remoteId();
        Incidence::Ptr incidence = calendar()->instance(rid);
        if (!incidence) {
            qCritical() << "akonadi_ical_resource: Can't find incidence with uid " << rid << "; item.id() = " << item.id();
            Q_EMIT error(i18n("Incidence with uid '%1' not found.", rid));
            return false;
        }

        Incidence::Ptr incidencePtr(incidence->clone());

        Item i = item;
        i.setMimeType(incidencePtr->mimeType());
        i.setPayload<Incidence::Ptr>(incidencePtr);
        resultItems.append(i);
    }

    itemsRetrieved(resultItems);

    return true;
}

void ICalResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &)
{
    if (!checkItemAddedChanged<Incidence::Ptr>(item, CheckForAdded)) {
        return;
    }

    auto i = item.payload<Incidence::Ptr>();
    if (!calendar()->addIncidence(Incidence::Ptr(i->clone()))) {
        // qCritical() << "akonadi_ical_resource: Error adding incidence with uid "
        //         << i->uid() << "; item.id() " << item.id() << i->recurrenceId();
        cancelTask();
        return;
    }

    Item it(item);
    it.setRemoteId(i->instanceIdentifier());
    scheduleWrite();
    changeCommitted(it);
}

void ICalResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    if (!checkItemAddedChanged<Incidence::Ptr>(item, CheckForChanged)) {
        return;
    }

    auto payload = item.payload<Incidence::Ptr>();
    Incidence::Ptr incidence = calendar()->instance(item.remoteId());
    if (!incidence) {
        // not in the calendar yet, should not happen -> add it
        calendar()->addIncidence(Incidence::Ptr(payload->clone()));
    } else {
        // make sure any observer the resource might have installed gets properly notified
        incidence->startUpdates();

        if (incidence->type() == payload->type()) {
            // IncidenceBase::operator= calls virtual method assign, so it's safe.
            *incidence.staticCast<IncidenceBase>().data() = *payload.data();
            incidence->updated();
            incidence->endUpdates();
        } else {
            incidence->endUpdates();
            qWarning() << "akonadi_ical_resource: Item changed incidence type. Replacing it.";

            calendar()->deleteIncidence(incidence);
            calendar()->addIncidence(Incidence::Ptr(payload->clone()));
        }
    }
    scheduleWrite();
    changeCommitted(item);
}

void ICalResource::doRetrieveItems(const Akonadi::Collection &col)
{
    Q_UNUSED(col)
    const Incidence::List incidences = calendar()->incidences();
    Item::List items;
    items.reserve(incidences.count());
    for (const Incidence::Ptr &incidence : incidences) {
        Item item(incidence->mimeType());
        item.setRemoteId(incidence->instanceIdentifier());
        item.setPayload(Incidence::Ptr(incidence->clone()));
        items << item;
    }
    itemsRetrieved(items);
}

QStringList ICalResource::allMimeTypes() const
{
    return QStringList() << KCalendarCore::Event::eventMimeType() << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType()
                         << KCalendarCore::FreeBusy::freeBusyMimeType();
}

QString ICalResource::mimeType(const IncidenceBase::Ptr &incidence) const
{
    return incidence->mimeType();
}
