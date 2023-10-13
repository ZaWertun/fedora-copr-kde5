/*
    SPDX-FileCopyrightText: 2006 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icalresourcebase.h"
#include "icalsettingsadaptor.h"

#include <QDBusConnection>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Incidence>

#include <KLocalizedString>
#include <QDebug>
#include <QTimeZone>

using namespace Akonadi;
using namespace KCalendarCore;
using namespace SETTINGS_NAMESPACE;

ICalResourceBase::ICalResourceBase(const QString &id)
    : SingleFileResource<Settings>(id)
{
}

void ICalResourceBase::initialise(const QStringList &mimeTypes, const QString &icon)
{
    setSupportedMimetypes(mimeTypes, icon);
    new ICalSettingsAdaptor(mSettings);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), mSettings, QDBusConnection::ExportAdaptors);
}

ICalResourceBase::~ICalResourceBase() = default;

bool ICalResourceBase::retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts)
{
    for (const Akonadi::Item &item : items) {
        qDebug() << "Item:" << item.url();
    }

    if (!mCalendar) {
        qCritical() << "akonadi_ical_resource: Calendar not loaded";
        Q_EMIT error(i18n("Calendar not loaded."));
        return false;
    }

    return doRetrieveItems(items, parts);
}

bool ICalResourceBase::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts)
{
    qDebug() << "Item:" << item.url();

    if (!mCalendar) {
        qCritical() << "akonadi_ical_resource: Calendar not loaded";
        Q_EMIT error(i18n("Calendar not loaded."));
        return false;
    }

    return doRetrieveItems({item}, parts);
}

void ICalResourceBase::aboutToQuit()
{
    if (!mSettings->readOnly()) {
        writeFile();
    }
    mSettings->save();
}

bool ICalResourceBase::readFromFile(const QString &fileName)
{
    mCalendar = KCalendarCore::MemoryCalendar::Ptr(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
    mFileStorage = KCalendarCore::FileStorage::Ptr(new KCalendarCore::FileStorage(mCalendar, fileName, new KCalendarCore::ICalFormat()));
    const bool result = mFileStorage->load();
    if (!result) {
        qCritical() << "akonadi_ical_resource: Error loading file " << fileName;
    }

    return result;
}

void ICalResourceBase::itemRemoved(const Akonadi::Item &item)
{
    if (!mCalendar) {
        qCritical() << "akonadi_ical_resource: mCalendar is 0!";
        cancelTask(i18n("Calendar not loaded."));
        return;
    }

    const Incidence::Ptr i = mCalendar->instance(item.remoteId());
    if (i) {
        if (!mCalendar->deleteIncidence(i)) {
            qCritical() << "akonadi_ical_resource: Can't delete incidence with instance identifier " << item.remoteId() << "; item.id() = " << item.id();
            cancelTask();
            return;
        }
    } else {
        qCritical() << "akonadi_ical_resource: itemRemoved(): Can't find incidence with instance identifier " << item.remoteId()
                    << "; item.id() = " << item.id();
    }
    scheduleWrite();
    changeProcessed();
}

void ICalResourceBase::retrieveItems(const Akonadi::Collection &col)
{
    reloadFile();
    if (mCalendar) {
        doRetrieveItems(col);
    } else {
        qCritical() << "akonadi_ical_resource: retrieveItems(): mCalendar is 0!";
    }
}

bool ICalResourceBase::writeToFile(const QString &fileName)
{
    if (!mCalendar) {
        qCritical() << "akonadi_ical_resource: writeToFile() mCalendar is 0!";
        return false;
    }

    KCalendarCore::FileStorage *fileStorage = mFileStorage.data();
    if (fileName != mFileStorage->fileName()) {
        fileStorage = new KCalendarCore::FileStorage(mCalendar, fileName, new KCalendarCore::ICalFormat());
    }

    bool success = true;
    if (!fileStorage->save()) {
        qCritical() << QStringLiteral("akonadi_ical_resource: Failed to save calendar to file ") + fileName;
        Q_EMIT error(i18n("Failed to save calendar file to %1", fileName));
        success = false;
    }

    if (fileStorage != mFileStorage.data()) {
        delete fileStorage;
    }

    return success;
}

KCalendarCore::MemoryCalendar::Ptr ICalResourceBase::calendar() const
{
    return mCalendar;
}

KCalendarCore::FileStorage::Ptr ICalResourceBase::fileStorage() const
{
    return mFileStorage;
}
