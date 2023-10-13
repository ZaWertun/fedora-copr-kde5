/*
    SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "calendarhandler.h"
#include "defaultreminderattribute.h"
#include "googlecalendar_debug.h"
#include "googleresource.h"
#include "googlesettings.h"

#include <Akonadi/BlockAlarmsAttribute>
#include <Akonadi/CollectionColorAttribute>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemModifyJob>

#include <KGAPI/Account>
#include <KGAPI/Calendar/Calendar>
#include <KGAPI/Calendar/CalendarCreateJob>
#include <KGAPI/Calendar/CalendarDeleteJob>
#include <KGAPI/Calendar/CalendarFetchJob>
#include <KGAPI/Calendar/CalendarModifyJob>
#include <KGAPI/Calendar/Event>
#include <KGAPI/Calendar/EventCreateJob>
#include <KGAPI/Calendar/EventDeleteJob>
#include <KGAPI/Calendar/EventFetchJob>
#include <KGAPI/Calendar/EventModifyJob>
#include <KGAPI/Calendar/EventMoveJob>
#include <KGAPI/Calendar/FreeBusyQueryJob>

#include <KCalendarCore/Calendar>
#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/ICalFormat>

using namespace KGAPI2;
using namespace Akonadi;

QString CalendarHandler::mimeType()
{
    return KCalendarCore::Event::eventMimeType();
}

bool CalendarHandler::canPerformTask(const Item &item)
{
    return GenericHandler::canPerformTask<KCalendarCore::Event::Ptr>(item);
}

bool CalendarHandler::canPerformTask(const Item::List &items)
{
    return GenericHandler::canPerformTask<KCalendarCore::Event::Ptr>(items);
}

void CalendarHandler::setupCollection(Collection &collection, const CalendarPtr &calendar)
{
    collection.setContentMimeTypes({mimeType()});
    collection.setName(calendar->uid());
    collection.setRemoteId(calendar->uid());
    if (calendar->editable()) {
        collection.setRights(Collection::CanChangeCollection | Collection::CanDeleteCollection | Collection::CanCreateItem | Collection::CanChangeItem
                             | Collection::CanDeleteItem);
    } else {
        collection.setRights(Collection::ReadOnly);
    }
    // TODO: for some reason, KOrganizer creates virtual collections
    // newCollection.setVirtual(false);
    // Setting icon
    auto attr = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(calendar->title());
    attr->setIconName(QStringLiteral("view-calendar"));
    // Setting color
    if (calendar->backgroundColor().isValid()) {
        auto colorAttr = collection.attribute<CollectionColorAttribute>(Collection::AddIfMissing);
        colorAttr->setColor(calendar->backgroundColor());
    }
    // Setting default reminders
    auto reminderAttr = collection.attribute<DefaultReminderAttribute>(Collection::AddIfMissing);
    reminderAttr->setReminders(calendar->defaultReminders());
    // Block email reminders, since Google sends them for us
    auto blockAlarms = collection.attribute<BlockAlarmsAttribute>(Collection::AddIfMissing);
    blockAlarms->blockAlarmType(KCalendarCore::Alarm::Audio, false);
    blockAlarms->blockAlarmType(KCalendarCore::Alarm::Display, false);
    blockAlarms->blockAlarmType(KCalendarCore::Alarm::Procedure, false);
}

void CalendarHandler::retrieveCollections(const Collection &rootCollection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Retrieving calendars"));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Retrieving calendars...";
    auto job = new CalendarFetchJob(m_settings->accountPtr(), this);
    connect(job, &CalendarFetchJob::finished, this, [this, rootCollection](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        qCDebug(GOOGLE_CALENDAR_LOG) << "Calendars retrieved";

        const ObjectsList calendars = qobject_cast<CalendarFetchJob *>(job)->items();
        Collection::List collections;
        collections.reserve(calendars.count());
        const QStringList activeCalendars = m_settings->calendars();
        for (const auto &object : calendars) {
            const CalendarPtr &calendar = object.dynamicCast<Calendar>();
            qCDebug(GOOGLE_CALENDAR_LOG) << " -" << calendar->title() << "(" << calendar->uid() << ")";
            if (!activeCalendars.contains(calendar->uid())) {
                qCDebug(GOOGLE_CALENDAR_LOG) << "Skipping, not subscribed";
                continue;
            }
            Collection collection;
            setupCollection(collection, calendar);
            collection.setParentCollection(rootCollection);
            collections << collection;
        }

        m_iface->collectionsRetrievedFromHandler(collections);
    });
}

void CalendarHandler::retrieveItems(const Collection &collection)
{
    qCDebug(GOOGLE_CALENDAR_LOG) << "Retrieving events for calendar" << collection.remoteId();
    const QString syncToken = collection.remoteRevision();
    auto job = new EventFetchJob(collection.remoteId(), m_settings->accountPtr(), this);
    if (!syncToken.isEmpty()) {
        qCDebug(GOOGLE_CALENDAR_LOG) << "Using sync token" << syncToken;
        job->setSyncToken(syncToken);
        job->setFetchDeleted(true);
    } else {
        // No need to fetch deleted items for non-incremental update
        job->setFetchDeleted(false);
        if (!m_settings->eventsSince().isEmpty()) {
            const QDate date = QDate::fromString(m_settings->eventsSince(), Qt::ISODate);
            job->setTimeMin(QDateTime(date.startOfDay()).toSecsSinceEpoch());
        }
    }

    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &EventFetchJob::finished, this, &CalendarHandler::slotItemsRetrieved);

    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Retrieving events for calendar '%1'", collection.displayName()));
}

void CalendarHandler::slotItemsRetrieved(KGAPI2::Job *job)
{
    if (!m_iface->handleError(job)) {
        return;
    }
    Item::List changedItems, removedItems;
    auto collection = job->property(COLLECTION_PROPERTY).value<Collection>();
    auto attr = collection.attribute<DefaultReminderAttribute>();

    auto fetchJob = qobject_cast<EventFetchJob *>(job);
    const ObjectsList objects = fetchJob->items();
    bool isIncremental = !fetchJob->syncToken().isEmpty();
    qCDebug(GOOGLE_CALENDAR_LOG) << "Retrieved" << objects.count() << "events for calendar" << collection.remoteId();
    changedItems.reserve(objects.count());
    for (const ObjectPtr &object : objects) {
        const EventPtr event = object.dynamicCast<Event>();
        if (event->useDefaultReminders() && attr) {
            const KCalendarCore::Alarm::List alarms = attr->alarms(event.data());
            for (const KCalendarCore::Alarm::Ptr &alarm : alarms) {
                event->addAlarm(alarm);
            }
        }

        Item item;
        item.setMimeType(mimeType());
        item.setParentCollection(collection);
        item.setRemoteId(event->id());
        item.setRemoteRevision(event->etag());
        item.setPayload<KCalendarCore::Event::Ptr>(event.dynamicCast<KCalendarCore::Event>());

        if (event->deleted()) {
            qCDebug(GOOGLE_CALENDAR_LOG) << " - removed" << event->uid();
            removedItems << item;
        } else {
            qCDebug(GOOGLE_CALENDAR_LOG) << " - changed" << event->uid();
            changedItems << item;
        }
    }

    if (!isIncremental) {
        m_iface->itemsRetrieved(changedItems);
    } else {
        m_iface->itemsRetrievedIncremental(changedItems, removedItems);
    }
    qCDebug(GOOGLE_CALENDAR_LOG) << "Next sync token:" << fetchJob->syncToken();
    collection.setRemoteRevision(fetchJob->syncToken());
    new CollectionModifyJob(collection, this);

    emitReadyStatus();
}

void CalendarHandler::itemAdded(const Item &item, const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Adding event to calendar '%1'", collection.name()));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Event added to calendar" << collection.remoteId();
    EventPtr event(new Event(*item.payload<KCalendarCore::Event::Ptr>()));
    auto job = new EventCreateJob(event, collection.remoteId(), m_settings->accountPtr(), this);
    job->setSendUpdates(SendUpdatesPolicy::None);
    connect(job, &EventCreateJob::finished, this, [this, item](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        Item newItem(item);
        const EventPtr event = qobject_cast<EventCreateJob *>(job)->items().first().dynamicCast<Event>();
        qCDebug(GOOGLE_CALENDAR_LOG) << "Event added";
        newItem.setRemoteId(event->id());
        newItem.setRemoteRevision(event->etag());
        newItem.setGid(event->uid());
        m_iface->itemChangeCommitted(newItem);
        newItem.setPayload<KCalendarCore::Event::Ptr>(event.dynamicCast<KCalendarCore::Event>());
        new ItemModifyJob(newItem, this);
        emitReadyStatus();
    });
}

void CalendarHandler::itemChanged(const Item &item, const QSet<QByteArray> & /*partIdentifiers*/)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Changing event in calendar '%1'", item.parentCollection().displayName()));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Changing event" << item.remoteId();
    EventPtr event(new Event(*item.payload<KCalendarCore::Event::Ptr>()));
    auto job = new EventModifyJob(event, item.parentCollection().remoteId(), m_settings->accountPtr(), this);
    job->setSendUpdates(SendUpdatesPolicy::None);
    job->setProperty(ITEM_PROPERTY, QVariant::fromValue(item));
    connect(job, &EventModifyJob::finished, this, &CalendarHandler::slotGenericJobFinished);
}

void CalendarHandler::itemsRemoved(const Item::List &items)
{
    m_iface->emitStatus(AgentBase::Running, i18ncp("@info:status", "Removing %1 event", "Removing %1 events", items.count()));
    QStringList eventIds;
    eventIds.reserve(items.count());
    std::transform(items.cbegin(), items.cend(), std::back_inserter(eventIds), [](const Item &item) {
        return item.remoteId();
    });
    qCDebug(GOOGLE_CALENDAR_LOG) << "Removing events:" << eventIds;
    auto job = new EventDeleteJob(eventIds, items.first().parentCollection().remoteId(), m_settings->accountPtr(), this);
    job->setProperty(ITEMS_PROPERTY, QVariant::fromValue(items));
    connect(job, &EventDeleteJob::finished, this, &CalendarHandler::slotGenericJobFinished);
}

void CalendarHandler::itemsMoved(const Item::List &items, const Collection &collectionSource, const Collection &collectionDestination)
{
    m_iface->emitStatus(AgentBase::Running,
                        i18ncp("@info:status",
                               "Moving %1 event from calendar '%2' to calendar '%3'",
                               "Moving %1 events from calendar '%2' to calendar '%3'",
                               items.count(),
                               collectionSource.displayName(),
                               collectionDestination.displayName()));
    QStringList eventIds;
    eventIds.reserve(items.count());
    std::transform(items.cbegin(), items.cend(), std::back_inserter(eventIds), [](const Item &item) {
        return item.remoteId();
    });
    qCDebug(GOOGLE_CALENDAR_LOG) << "Moving events" << eventIds << "from" << collectionSource.remoteId() << "to" << collectionDestination.remoteId();
    auto job = new EventMoveJob(eventIds, collectionSource.remoteId(), collectionDestination.remoteId(), m_settings->accountPtr(), this);
    job->setProperty(ITEMS_PROPERTY, QVariant::fromValue(items));
    connect(job, &EventMoveJob::finished, this, &CalendarHandler::slotGenericJobFinished);
}

void CalendarHandler::collectionAdded(const Collection &collection, const Collection & /*parent*/)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Creating calendar '%1'", collection.displayName()));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Adding calendar" << collection.displayName();
    CalendarPtr calendar(new Calendar());
    calendar->setTitle(collection.displayName());
    calendar->setEditable(true);

    auto job = new CalendarCreateJob(calendar, m_settings->accountPtr(), this);
    connect(job, &CalendarCreateJob::finished, this, [this, collection](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        CalendarPtr calendar = qobject_cast<CalendarCreateJob *>(job)->items().first().dynamicCast<Calendar>();
        qCDebug(GOOGLE_CALENDAR_LOG) << "Created calendar" << calendar->uid();
        // Enable newly added calendar in settings, otherwise user won't see it
        m_settings->addCalendar(calendar->uid());
        // TODO: the calendar returned by google is almost empty, i.e. it's not "editable",
        // does not contain the color, etc
        calendar->setEditable(true);
        // Populate remoteId & other stuff
        Collection newCollection(collection);
        setupCollection(newCollection, calendar);
        m_iface->collectionChangeCommitted(newCollection);
        emitReadyStatus();
    });
}

void CalendarHandler::collectionChanged(const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Changing calendar '%1'", collection.displayName()));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Changing calendar" << collection.remoteId();
    CalendarPtr calendar(new Calendar());
    calendar->setUid(collection.remoteId());
    calendar->setTitle(collection.displayName());
    calendar->setEditable(true);
    auto job = new CalendarModifyJob(calendar, m_settings->accountPtr(), this);
    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &CalendarModifyJob::finished, this, &CalendarHandler::slotGenericJobFinished);
}

void CalendarHandler::collectionRemoved(const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Removing calendar '%1'", collection.displayName()));
    qCDebug(GOOGLE_CALENDAR_LOG) << "Removing calendar" << collection.remoteId();
    auto job = new CalendarDeleteJob(collection.remoteId(), m_settings->accountPtr(), this);
    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &CalendarDeleteJob::finished, this, &CalendarHandler::slotGenericJobFinished);
}

/**
 * FreeBusy
 */
FreeBusyHandler::FreeBusyHandler(GoogleResourceStateInterface *iface, GoogleSettings *settings)
    : m_iface(iface)
    , m_settings(settings)
{
}

QDateTime FreeBusyHandler::lastCacheUpdate() const
{
    return {};
}

void FreeBusyHandler::canHandleFreeBusy(const QString &email)
{
    if (m_iface->canPerformTask()) {
        m_iface->handlesFreeBusy(email, false);
        return;
    }

    auto job = new FreeBusyQueryJob(email, QDateTime::currentDateTimeUtc(), QDateTime::currentDateTimeUtc().addSecs(3600), m_settings->accountPtr(), this);
    connect(job, &FreeBusyQueryJob::finished, this, [this](KGAPI2::Job *job) {
        auto queryJob = qobject_cast<FreeBusyQueryJob *>(job);
        if (!m_iface->handleError(job, false)) {
            m_iface->handlesFreeBusy(queryJob->id(), false);
            return;
        }
        m_iface->handlesFreeBusy(queryJob->id(), true);
    });
}

void FreeBusyHandler::retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end)
{
    if (m_iface->canPerformTask()) {
        m_iface->freeBusyRetrieved(email, QString(), false, QString());
        return;
    }

    auto job = new FreeBusyQueryJob(email, start, end, m_settings->accountPtr(), this);
    connect(job, &FreeBusyQueryJob::finished, this, [this](KGAPI2::Job *job) {
        auto queryJob = qobject_cast<FreeBusyQueryJob *>(job);

        if (!m_iface->handleError(job, false)) {
            m_iface->freeBusyRetrieved(queryJob->id(), QString(), false, QString());
            return;
        }

        KCalendarCore::FreeBusy::Ptr fb(new KCalendarCore::FreeBusy);
        fb->setUid(QStringLiteral("%1%2@google.com").arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMddTHHmmssZ"))));
        fb->setOrganizer(job->account()->accountName());
        fb->addAttendee(KCalendarCore::Attendee(QString(), queryJob->id()));
        // FIXME: is it really sort?
        fb->setDateTime(QDateTime::currentDateTimeUtc(), KCalendarCore::IncidenceBase::RoleSort);
        const auto ranges = queryJob->busy();
        for (const auto &range : ranges) {
            fb->addPeriod(range.busyStart, range.busyEnd);
        }

        KCalendarCore::ICalFormat format;
        const QString fbStr = format.createScheduleMessage(fb, KCalendarCore::iTIPRequest);

        m_iface->freeBusyRetrieved(queryJob->id(), fbStr, true, QString());
    });
}
