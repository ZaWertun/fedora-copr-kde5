/*
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2008 Bertjan Broeksema <broeksema@kde.org>
    SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icaldirresource.h"

#include "settingsadaptor.h"

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KCalendarCore/FileStorage>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>
#include <KLocalizedString>
#include <QDebug>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTimeZone>

using namespace Akonadi;
using namespace KCalendarCore;

static Incidence::Ptr readFromFile(const QString &fileName, const QString &expectedIdentifier)
{
    MemoryCalendar::Ptr calendar = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    FileStorage::Ptr fileStorage = FileStorage::Ptr(new FileStorage(calendar, fileName, new ICalFormat()));

    Incidence::Ptr incidence;
    if (fileStorage->load()) {
        Incidence::List incidences = calendar->incidences();
        if (incidences.count() == 1 && incidences.first()->instanceIdentifier() == expectedIdentifier) {
            incidence = incidences.first();
        }
    } else {
        qCritical() << "Error loading file " << fileName;
    }

    return incidence;
}

static bool writeToFile(const QString &fileName, Incidence::Ptr &incidence)
{
    if (!incidence) {
        qCritical() << "incidence is 0!";
        return false;
    }

    MemoryCalendar::Ptr calendar = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    FileStorage::Ptr fileStorage = FileStorage::Ptr(new FileStorage(calendar, fileName, new ICalFormat()));
    calendar->addIncidence(incidence);
    Q_ASSERT(calendar->incidences().count() == 1);

    const bool success = fileStorage->save();
    if (!success) {
        qCritical() << QStringLiteral("Failed to save calendar to file ") + fileName;
    }

    return success;
}

ICalDirResource::ICalDirResource(const QString &id)
    : ResourceBase(id)
{
    IcalDirResourceSettings::instance(KSharedConfig::openConfig());
    // setup the resource
    new SettingsAdaptor(IcalDirResourceSettings::self());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), IcalDirResourceSettings::self(), QDBusConnection::ExportAdaptors);

    changeRecorder()->itemFetchScope().fetchFullPayload();
    connect(this, &ICalDirResource::reloadConfiguration, this, &ICalDirResource::slotReloadConfig);
}

ICalDirResource::~ICalDirResource() = default;

void ICalDirResource::slotReloadConfig()
{
    IcalDirResourceSettings::self()->load();

    initializeICalDirectory();
    loadIncidences();

    synchronize();
}

void ICalDirResource::aboutToQuit()
{
    IcalDirResourceSettings::self()->save();
}

bool ICalDirResource::loadIncidences()
{
    mIncidences.clear();

    QDirIterator it(iCalDirectoryName());
    while (it.hasNext()) {
        it.next();
        if (it.fileName() != QLatin1String(".") && it.fileName() != QLatin1String("..") && it.fileName() != QLatin1String("WARNING_README.txt")) {
            const KCalendarCore::Incidence::Ptr incidence = readFromFile(it.filePath(), it.fileName());
            if (incidence) {
                mIncidences.insert(incidence->instanceIdentifier(), incidence);
            }
        }
    }

    Q_EMIT status(Idle);
    return true;
}

bool ICalDirResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    const QString remoteId = item.remoteId();
    if (!mIncidences.contains(remoteId)) {
        Q_EMIT error(i18n("Incidence with uid '%1' not found.", remoteId));
        return false;
    }

    Item newItem(item);
    newItem.setPayload<KCalendarCore::Incidence::Ptr>(mIncidences.value(remoteId));
    itemRetrieved(newItem);

    return true;
}

void ICalDirResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &)
{
    if (IcalDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", iCalDirectoryName()));
        cancelTask();
        return;
    }

    KCalendarCore::Incidence::Ptr incidence;
    if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    }

    if (incidence) {
        // add it to the cache...
        mIncidences.insert(incidence->instanceIdentifier(), incidence);

        // ... and write it through to the file system
        const bool success = writeToFile(iCalDirectoryFileName(incidence->instanceIdentifier()), incidence);

        if (success) {
            // report everything ok
            Item newItem(item);
            newItem.setRemoteId(incidence->instanceIdentifier());
            changeCommitted(newItem);
        } else {
            cancelTask();
        }
    } else {
        changeProcessed();
    }
}

void ICalDirResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    if (IcalDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", iCalDirectoryName()));
        cancelTask();
        return;
    }

    KCalendarCore::Incidence::Ptr incidence;
    if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    }

    if (incidence) {
        // change it in the cache...
        mIncidences.insert(incidence->instanceIdentifier(), incidence);

        // ... and write it through to the file system
        const bool success = writeToFile(iCalDirectoryFileName(incidence->instanceIdentifier()), incidence);

        if (success) {
            Item newItem(item);
            newItem.setRemoteId(incidence->instanceIdentifier());
            changeCommitted(newItem);
        } else {
            cancelTask();
        }
    } else {
        changeProcessed();
    }
}

void ICalDirResource::itemRemoved(const Akonadi::Item &item)
{
    if (IcalDirResourceSettings::self()->readOnly()) {
        Q_EMIT error(i18n("Trying to write to a read-only directory: '%1'", iCalDirectoryName()));
        cancelTask();
        return;
    }

    // remove it from the cache...
    mIncidences.remove(item.remoteId());

    // ... and remove it from the file system
    QFile::remove(iCalDirectoryFileName(item.remoteId()));

    changeProcessed();
}

void ICalDirResource::collectionChanged(const Collection &collection)
{
    if (collection.hasAttribute<EntityDisplayAttribute>()) {
        auto attr = collection.attribute<EntityDisplayAttribute>();
        if (attr->displayName() != name()) {
            setName(attr->displayName());
        }
    }

    changeProcessed();
}

void ICalDirResource::retrieveCollections()
{
    Collection c;
    c.setParentCollection(Collection::root());
    c.setRemoteId(iCalDirectoryName());
    c.setName(name());

    QStringList mimetypes;
    mimetypes << KCalendarCore::Event::eventMimeType() << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType()
              << QStringLiteral("text/calendar");
    c.setContentMimeTypes(mimetypes);

    if (IcalDirResourceSettings::self()->readOnly()) {
        c.setRights(Collection::CanChangeCollection);
    } else {
        Collection::Rights rights = Collection::ReadOnly;
        rights |= Collection::CanChangeItem;
        rights |= Collection::CanCreateItem;
        rights |= Collection::CanDeleteItem;
        rights |= Collection::CanChangeCollection;
        c.setRights(rights);
    }

    auto attr = c.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(name() == identifier() ? i18n("Calendar Folder") : name());
    attr->setIconName(QStringLiteral("office-calendar"));

    const Collection::List list{c};
    collectionsRetrieved(list);
}

void ICalDirResource::retrieveItems(const Akonadi::Collection &)
{
    loadIncidences();
    Item::List items;
    items.reserve(mIncidences.count());

    for (const KCalendarCore::Incidence::Ptr &incidence : std::as_const(mIncidences)) {
        Item item;
        item.setRemoteId(incidence->instanceIdentifier());
        item.setMimeType(incidence->mimeType());
        items.append(item);
    }

    itemsRetrieved(items);
}

QString ICalDirResource::iCalDirectoryName() const
{
    return IcalDirResourceSettings::self()->path();
}

QString ICalDirResource::iCalDirectoryFileName(const QString &file) const
{
    return IcalDirResourceSettings::self()->path() + QLatin1Char('/') + file;
}

void ICalDirResource::initializeICalDirectory() const
{
    QDir dir(iCalDirectoryName());

    // if folder does not exists, create it
    if (!dir.exists()) {
        if (!QDir::root().mkpath(dir.absolutePath())) {
            qCritical() << "Failed to create ical directory" << dir.absolutePath();
        } else {
            qDebug() << "iCal directory " << dir.absolutePath() << "successfuly created";
        }
    }

    // check whether warning file is in place...
    QFile file(dir.absolutePath() + QStringLiteral("/WARNING_README.txt"));
    if (!file.exists()) {
        // ... if not, create it
        file.open(QIODevice::WriteOnly);
        file.write(
            "Important Warning!!!\n\n"
            "Don't create or copy files inside this folder manually, they are managed by the Akonadi framework!\n");
        file.close();
    }
}

AKONADI_RESOURCE_MAIN(ICalDirResource)
