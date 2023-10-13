/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Event>

#include <Akonadi/ResourceBase>

#include <QHash>

namespace KContacts
{
class Addressee;
}

class QDate;

class BirthdaysResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
    Q_OBJECT

public:
    explicit BirthdaysResource(const QString &id);
    ~BirthdaysResource() override;

protected:
    using ResourceBase::retrieveItems; // Suppress -Woverload-virtual

protected:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

private:
    void addPendingEvent(const KCalendarCore::Event::Ptr &event, const QString &remoteId);
    void checkForUnknownCategories(const QString &categoryToCheck, KCalendarCore::Event::Ptr &event);

    KCalendarCore::Event::Ptr createBirthday(const KContacts::Addressee &contact, Akonadi::Item::Id itemId);
    KCalendarCore::Event::Ptr createAnniversary(const KContacts::Addressee &contact, Akonadi::Item::Id itemId);
    KCalendarCore::Event::Ptr createEvent(QDate date);

private Q_SLOTS:
    void doFullSearch();
    void listContacts(const Akonadi::Collection::List &cols);
    void createEvents(const Akonadi::Item::List &items);

    void contactChanged(const Akonadi::Item &item);
    void contactRemoved(const Akonadi::Item &item);

    void contactRetrieved(KJob *job);
    void contactsRetrieved(KJob *job);

private:
    void slotReloadConfig();
    QHash<QString, Akonadi::Item> mPendingItems;
    QHash<QString, Akonadi::Item> mDeletedItems;
};
