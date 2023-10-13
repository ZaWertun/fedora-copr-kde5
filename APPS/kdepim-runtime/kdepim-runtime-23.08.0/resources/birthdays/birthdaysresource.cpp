/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "birthdaysresource.h"
#include "settings.h"
#include "settingsadaptor.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MimeTypeChecker>
#include <Akonadi/Monitor>
#include <Akonadi/VectorHelper>

#include <KContacts/Addressee>

#include <KEmailAddress>

#include "birthdays_debug.h"
#include <KLocalizedString>

#include <Akonadi/TagCreateJob>

using namespace Akonadi;
using namespace KContacts;
using namespace KCalendarCore;

BirthdaysResource::BirthdaysResource(const QString &id)
    : ResourceBase(id)
{
    Settings::instance(KSharedConfig::openConfig());
    new SettingsAdaptor(Settings::self());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), Settings::self(), QDBusConnection::ExportAdaptors);

    setName(i18n("Birthdays & Anniversaries"));
    auto monitor = new Monitor(this);
    monitor->setMimeTypeMonitored(Addressee::mimeType());
    monitor->itemFetchScope().fetchFullPayload();
    connect(monitor, &Monitor::itemAdded, this, &BirthdaysResource::contactChanged);
    connect(monitor, &Monitor::itemChanged, this, &BirthdaysResource::contactChanged);
    connect(monitor, &Monitor::itemRemoved, this, &BirthdaysResource::contactRemoved);

    connect(this, &BirthdaysResource::reloadConfiguration, this, &BirthdaysResource::slotReloadConfig);
}

BirthdaysResource::~BirthdaysResource() = default;

void BirthdaysResource::slotReloadConfig()
{
    doFullSearch();
    synchronizeCollectionTree();
}

void BirthdaysResource::retrieveCollections()
{
    Collection c;
    c.setParentCollection(Collection::root());
    c.setRemoteId(QStringLiteral("akonadi_birthdays_resource"));
    c.setName(name());
    c.setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
    c.setRights(Collection::ReadOnly);

    auto attribute = c.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attribute->setIconName(QStringLiteral("view-calendar-birthday"));

    Collection::List list;
    list << c;
    collectionsRetrieved(list);
}

void BirthdaysResource::retrieveItems(const Akonadi::Collection &collection)
{
    Q_UNUSED(collection)
    itemsRetrievedIncremental(Akonadi::valuesToVector(mPendingItems), Akonadi::valuesToVector(mDeletedItems));
    mPendingItems.clear();
    mDeletedItems.clear();
}

bool BirthdaysResource::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)

    // collect contacts, same contact might be used multiple times, for birthday & anniversary
    QSet<Akonadi::Item::Id> contactIds;
    contactIds.reserve(items.count());
    for (const auto &item : items) {
        const Akonadi::Item::Id contactId = item.remoteId().mid(1).toLongLong();
        contactIds << contactId;
    }

    // query contacts
    Akonadi::Item::List contactItems;
    contactItems.reserve(contactIds.size());
    for (Akonadi::Item::Id contactId : std::as_const(contactIds)) {
        contactItems.append(Item(contactId));
    }
    auto job = new ItemFetchJob(contactItems, this);
    job->fetchScope().fetchFullPayload();
    connect(job, &ItemFetchJob::result, this, &BirthdaysResource::contactsRetrieved);

    return true;
}

bool BirthdaysResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    Q_UNUSED(parts)
    qint64 contactId = item.remoteId().mid(1).toLongLong();
    auto job = new ItemFetchJob(Item(contactId), this);
    job->fetchScope().fetchFullPayload();
    connect(job, &ItemFetchJob::result, this, &BirthdaysResource::contactRetrieved);
    return true;
}

void BirthdaysResource::contactRetrieved(KJob *job)
{
    auto fj = static_cast<ItemFetchJob *>(job);
    if (job->error()) {
        Q_EMIT error(job->errorText());
        cancelTask();
    } else if (fj->items().count() != 1) {
        cancelTask();
    } else {
        const auto contactItem = fj->items().at(0);
        if (!contactItem.hasPayload<KContacts::Addressee>()) {
            cancelTask();
            return;
        }
        auto contact = contactItem.payload<KContacts::Addressee>();
        KCalendarCore::Incidence::Ptr ev;
        if (currentItems().at(0).remoteId().startsWith(QLatin1Char('b'))) {
            ev = createBirthday(contact, contactItem.id());
        } else if (currentItems().at(0).remoteId().startsWith(QLatin1Char('a'))) {
            ev = createAnniversary(contact, contactItem.id());
        }
        if (!ev) {
            cancelTask();
        } else {
            Item i(currentItems().at(0));
            i.setPayload<Incidence::Ptr>(ev);
            itemRetrieved(i);
        }
    }
}

void BirthdaysResource::contactsRetrieved(KJob *job)
{
    if (job->error()) {
        Q_EMIT error(job->errorText());
        cancelTask();
        return;
    }

    // prepare contacts look-up table
    auto fetchJob = static_cast<ItemFetchJob *>(job);
    const auto contactItems = fetchJob->items();

    QHash<Akonadi::Item::Id, KContacts::Addressee> contacts;
    contacts.reserve(contactItems.size());
    for (auto &contactItem : contactItems) {
        if (!contactItem.hasPayload<KContacts::Addressee>()) {
            cancelTask();
            return;
        }
        auto contact = contactItem.payload<KContacts::Addressee>();
        contacts.insert(contactItem.id(), contact);
    }

    // for all queried items now generate payload from the available contacts
    const Akonadi::Item::List queriedItems = currentItems();

    Akonadi::Item::List resultItems;
    resultItems.reserve(queriedItems.size());

    for (auto &item : queriedItems) {
        const QString remoteId = item.remoteId();
        const Akonadi::Item::Id contactId = remoteId.mid(1).toLongLong();
        auto it = contacts.constFind(contactId);
        if (it == contacts.constEnd()) {
            cancelTask();
            return;
        }
        auto &contact = *it;

        KCalendarCore::Incidence::Ptr ev;
        if (remoteId.startsWith(QLatin1Char('b'))) {
            ev = createBirthday(contact, contactId);
        } else if (remoteId.startsWith(QLatin1Char('a'))) {
            ev = createAnniversary(contact, contactId);
        }
        if (!ev) {
            cancelTask();
            return;
        }

        Item i(item);
        i.setPayload<Incidence::Ptr>(ev);
        resultItems.append(i);
    }

    itemsRetrieved(resultItems);
}

void BirthdaysResource::contactChanged(const Akonadi::Item &item)
{
    if (!item.hasPayload<KContacts::Addressee>()) {
        return;
    }

    auto contact = item.payload<KContacts::Addressee>();

    if (Settings::self()->filterOnCategories()) {
        bool hasCategory = false;
        const QStringList categories = contact.categories();
        const QStringList lst = Settings::self()->filterCategories();
        for (const QString &cat : lst) {
            if (categories.contains(cat)) {
                hasCategory = true;
                break;
            }
        }

        if (!hasCategory) {
            return;
        }
    }

    const Akonadi::Item::Id itemId = item.id();
    Event::Ptr event = createBirthday(contact, itemId);
    if (event) {
        addPendingEvent(event, QStringLiteral("b%1").arg(itemId));
    } else {
        Item i(KCalendarCore::Event::eventMimeType());
        i.setRemoteId(QStringLiteral("b%1").arg(itemId));
        mDeletedItems[i.remoteId()] = i;
    }

    event = createAnniversary(contact, itemId);
    if (event) {
        addPendingEvent(event, QStringLiteral("a%1").arg(itemId));
    } else {
        Item i(KCalendarCore::Event::eventMimeType());
        i.setRemoteId(QStringLiteral("a%1").arg(itemId));
        mDeletedItems[i.remoteId()] = i;
    }
    synchronize();
}

void BirthdaysResource::addPendingEvent(const KCalendarCore::Event::Ptr &event, const QString &remoteId)
{
    KCalendarCore::Incidence::Ptr evptr(event);
    Item i(KCalendarCore::Event::eventMimeType());
    i.setRemoteId(remoteId);
    i.setPayload(evptr);
    mPendingItems[remoteId] = i;
}

void BirthdaysResource::contactRemoved(const Akonadi::Item &item)
{
    Item i(KCalendarCore::Event::eventMimeType());
    i.setRemoteId(QStringLiteral("b%1").arg(item.id()));
    mDeletedItems[i.remoteId()] = i;
    i.setRemoteId(QStringLiteral("a%1").arg(item.id()));
    mDeletedItems[i.remoteId()] = i;
    synchronize();
}

void BirthdaysResource::doFullSearch()
{
    auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
    connect(job, &CollectionFetchJob::collectionsReceived, this, &BirthdaysResource::listContacts);
}

void BirthdaysResource::listContacts(const Akonadi::Collection::List &cols)
{
    MimeTypeChecker contactFilter;
    contactFilter.addWantedMimeType(Addressee::mimeType());
    for (const Collection &col : cols) {
        if (!contactFilter.isWantedCollection(col)) {
            continue;
        }
        auto job = new ItemFetchJob(col, this);
        job->fetchScope().fetchFullPayload();
        connect(job, &ItemFetchJob::itemsReceived, this, &BirthdaysResource::createEvents);
    }
}

void BirthdaysResource::createEvents(const Akonadi::Item::List &items)
{
    for (const Item &item : items) {
        contactChanged(item);
    }
}

KCalendarCore::Event::Ptr BirthdaysResource::createBirthday(const KContacts::Addressee &contact, Akonadi::Item::Id itemId)
{
    const QString name = contact.realName().isEmpty() ? contact.nickName() : contact.realName();
    if (name.isEmpty()) {
        qCDebug(BIRTHDAYS_LOG) << "contact " << contact.uid() << itemId << " has no name, skipping.";
        return {};
    }

    const QDate birthdate = contact.birthday().date();
    if (birthdate.isValid()) {
        const QString summary = i18n("%1's birthday", name);

        Event::Ptr ev = createEvent(birthdate);
        ev->setUid(contact.uid() + QStringLiteral("_KABC_Birthday"));

        ev->setCustomProperty("KABC", "BIRTHDAY", QStringLiteral("YES"));
        ev->setCustomProperty("KABC", "UID-1", contact.uid());
        ev->setCustomProperty("KABC", "NAME-1", name);
        ev->setCustomProperty("KABC", "EMAIL-1", contact.preferredEmail());
        ev->setSummary(summary);

        checkForUnknownCategories(i18n("Birthday"), ev);
        return ev;
    }
    return {};
}

KCalendarCore::Event::Ptr BirthdaysResource::createAnniversary(const KContacts::Addressee &contact, Akonadi::Item::Id itemId)
{
    const QString name = contact.realName().isEmpty() ? contact.nickName() : contact.realName();
    if (name.isEmpty()) {
        qCDebug(BIRTHDAYS_LOG) << "contact " << contact.uid() << itemId << " has no name, skipping.";
        return {};
    }

    const QString anniversary_string = contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"));
    if (anniversary_string.isEmpty()) {
        return {};
    }
    const QDate anniversary = QDate::fromString(anniversary_string, Qt::ISODate);
    if (anniversary.isValid()) {
        const QString spouseName = contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"));

        QString summary;
        if (!spouseName.isEmpty()) {
            QString tname, temail;
            KEmailAddress::extractEmailAddressAndName(spouseName, temail, tname);
            tname = KEmailAddress::quoteNameIfNecessary(tname);
            if ((tname[0] == QLatin1Char('"')) && (tname[tname.length() - 1] == QLatin1Char('"'))) {
                tname.remove(0, 1);
                tname.chop(1);
            }
            tname.remove(QLatin1Char('\\')); // remove escape chars
            KContacts::Addressee spouse;
            spouse.setNameFromString(tname);
            QString name_2 = spouse.nickName();
            if (name_2.isEmpty()) {
                name_2 = spouse.realName();
            }
            summary = i18nc("insert names of both spouses", "%1's & %2's anniversary", name, name_2);
        } else {
            summary = i18nc("only one spouse in addressbook, insert the name", "%1's anniversary", name);
        }

        Event::Ptr event = createEvent(anniversary);
        event->setUid(contact.uid() + QStringLiteral("_KABC_Anniversary"));
        event->setSummary(summary);

        event->setCustomProperty("KABC", "UID-1", contact.uid());
        event->setCustomProperty("KABC", "NAME-1", name);
        event->setCustomProperty("KABC", "EMAIL-1", contact.fullEmail());
        event->setCustomProperty("KABC", "ANNIVERSARY", QStringLiteral("YES"));
        // insert category
        checkForUnknownCategories(i18n("Anniversary"), event);
        return event;
    }
    return {};
}

KCalendarCore::Event::Ptr BirthdaysResource::createEvent(QDate date)
{
    Event::Ptr event(new Event());
    event->setDtStart(QDateTime(date, {}));
    event->setDtEnd(QDateTime(date, {}));
    event->setAllDay(true);
    event->setTransparency(Event::Transparent);

    // Set the recurrence
    Recurrence *recurrence = event->recurrence();
    recurrence->setStartDateTime(QDateTime(date, {}), true);
    recurrence->setYearly(1);
    if (date.month() == 2 && date.day() == 29) {
        recurrence->addYearlyDay(60);
    }

    // Set the alarm
    event->clearAlarms();
    if (Settings::self()->enableAlarm()) {
        Alarm::Ptr alarm = event->newAlarm();
        alarm->setType(Alarm::Display);
        alarm->setText(event->summary());
        alarm->setTime(QDateTime(date, {}));
        // N days before
        alarm->setStartOffset(Duration(-Settings::self()->alarmDays(), Duration::Days));
        alarm->setEnabled(true);
    }

    return event;
}

void BirthdaysResource::checkForUnknownCategories(const QString &categoryToCheck, Event::Ptr &event)
{
    auto tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(categoryToCheck), this);
    tagCreateJob->setMergeIfExisting(true);
    event->setCategories(categoryToCheck);
}

AKONADI_RESOURCE_MAIN(BirthdaysResource)
