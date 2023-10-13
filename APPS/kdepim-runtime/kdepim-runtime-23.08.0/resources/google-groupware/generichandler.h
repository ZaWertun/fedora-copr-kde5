/*
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <KGAPI/Types>

#include <QObject>
#include <QSharedPointer>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KLocalizedString>

#include "googleresourcestateinterface.h"

#define ITEM_PROPERTY "_AkonadiItem"
#define ITEMS_PROPERTY "_AkonadiItems"
#define COLLECTION_PROPERTY "_AkonadiCollection"

namespace KGAPI2
{
class Job;
}

class GoogleSettings;

class GenericHandler : public QObject
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<GenericHandler>;

    GenericHandler(GoogleResourceStateInterface *iface, GoogleSettings *settings);
    ~GenericHandler() override;

    virtual QString mimeType() = 0;

    virtual void retrieveCollections(const Akonadi::Collection &rootCollection) = 0;
    virtual void retrieveItems(const Akonadi::Collection &collection) = 0;

    virtual void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) = 0;
    virtual void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers) = 0;
    virtual void itemsRemoved(const Akonadi::Item::List &items) = 0;
    virtual void
    itemsMoved(const Akonadi::Item::List &items, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination) = 0;
    virtual void itemsLinked(const Akonadi::Item::List &items, const Akonadi::Collection &collection);
    virtual void itemsUnlinked(const Akonadi::Item::List &items, const Akonadi::Collection &collection);

    virtual void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) = 0;
    virtual void collectionChanged(const Akonadi::Collection &collection) = 0;
    virtual void collectionRemoved(const Akonadi::Collection &collection) = 0;

    /*
     * Helper function for various handlers
     */
    template<typename T>
    bool canPerformTask(const Akonadi::Item &item)
    {
        if (item.isValid() && (!item.hasPayload<T>() || item.mimeType() != mimeType())) {
            m_iface->cancelTask(i18n("Invalid item."));
            return false;
        }
        return m_iface->canPerformTask();
    }

    template<typename T>
    bool canPerformTask(const Akonadi::Item::List &items)
    {
        if (std::any_of(items.cbegin(), items.cend(), [this](const Akonadi::Item &item) {
                return item.isValid() && (!item.hasPayload<T>() || item.mimeType() != mimeType());
            })) {
            m_iface->cancelTask(i18n("Invalid item."));
            return false;
        }
        return m_iface->canPerformTask();
    }

    virtual bool canPerformTask(const Akonadi::Item &item) = 0;
    virtual bool canPerformTask(const Akonadi::Item::List &items) = 0;
protected Q_SLOTS:
    void slotGenericJobFinished(KGAPI2::Job *job);

protected:
    void emitReadyStatus();

    GoogleResourceStateInterface *const m_iface;
    GoogleSettings *const m_settings;
};
