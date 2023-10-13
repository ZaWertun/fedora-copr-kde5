/*
    SPDX-FileCopyrightText: 2006 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2009 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "settings.h"
#include "singlefileresource.h"

#include <KCalendarCore/FileStorage>
#include <KCalendarCore/MemoryCalendar>

class ICalResourceBase : public Akonadi::SingleFileResource<SETTINGS_NAMESPACE::Settings>
{
    Q_OBJECT

public:
    explicit ICalResourceBase(const QString &id);
    ~ICalResourceBase() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected Q_SLOTS:
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    void retrieveItems(const Akonadi::Collection &col) override;

protected:
    enum CheckType {
        CheckForAdded,
        CheckForChanged,
    };

    void initialise(const QStringList &mimeTypes, const QString &icon);
    bool readFromFile(const QString &fileName) override;
    bool writeToFile(const QString &fileName) override;

    void aboutToQuit() override;

    /**
     * Add the requested payload parts and call itemsRetrieved() when done.
     * It is guaranteed that all items in the list belong to the same Collection.
     * Retrieve an incidence from the calendar, and set it into a new item's payload.
     * Retrieval of items should be signalled by calling @p itemsRetrieved().
     * @param items the incidence ID to retrieve is provided by @c item.remoteId() for each item
     * @return true if all items are retrieved, false if not.
     */
    virtual bool doRetrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) = 0;

    /**
     * Retrieve all incidences from the calendar, and set each into a new item's payload.
     * Retrieval of the items should be signalled by calling @p itemsRetrieved().
     */
    virtual void doRetrieveItems(const Akonadi::Collection &col) = 0;

    /**
     * To be called at the start of derived class implementations of itemAdded()
     * or itemChanged() to verify that required conditions are true.
     * @param type the type of change to perform the checks for.
     * @return true if all checks are successful, and processing can continue;
     *         false if a check failed, in which case itemAdded() or itemChanged()
     *               should stop processing.
     */
    template<typename PayloadPtr>
    bool checkItemAddedChanged(const Akonadi::Item &item, CheckType type);

    void itemRemoved(const Akonadi::Item &item) override;

    /** Return the local calendar. */
    KCalendarCore::MemoryCalendar::Ptr calendar() const;

    /** Return the calendar file storage. */
    KCalendarCore::FileStorage::Ptr fileStorage() const;

private:
    KCalendarCore::MemoryCalendar::Ptr mCalendar;
    KCalendarCore::FileStorage::Ptr mFileStorage;
};

template<typename PayloadPtr>
bool ICalResourceBase::checkItemAddedChanged(const Akonadi::Item &item, CheckType type)
{
    if (!mCalendar) {
        cancelTask(i18n("Calendar not loaded."));
        return false;
    }
    if (!item.hasPayload<PayloadPtr>()) {
        const QString msg =
            (type == CheckForAdded) ? i18n("Unable to retrieve added item %1.", item.id()) : i18n("Unable to retrieve modified item %1.", item.id());
        cancelTask(msg);
        return false;
    }
    return true;
}
