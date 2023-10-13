/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/ResourceBase>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <KContacts/ContactGroupTool>
#include <KContacts/VCardConverter>

class QDir;

class ContactsResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::ObserverV2
{
    Q_OBJECT

public:
    explicit ContactsResource(const QString &id);
    ~ContactsResource() override;

public Q_SLOTS:
    void aboutToQuit() override;

protected:
    using ResourceBase::retrieveItems; // suppress -Woverload-virtual warnings

protected Q_SLOTS:
    void retrieveCollections() override;
    void retrieveItems(const Akonadi::Collection &collection) override;
    bool retrieveItems(const Akonadi::Item::List &items, const QSet<QByteArray> &parts) override;
    bool retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;

protected:
    void itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection) override;
    void itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &parts) override;
    void itemRemoved(const Akonadi::Item &item) override;

    void collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent) override;
    void collectionChanged(const Akonadi::Collection &collection) override;
    // do not hide the other variant, use implementation from base class
    // which just forwards to the one above
    using Akonadi::AgentBase::ObserverV2::collectionChanged;
    void collectionRemoved(const Akonadi::Collection &collection) override;

    void itemMoved(const Akonadi::Item &item, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination) override;
    void collectionMoved(const Akonadi::Collection &collection,
                         const Akonadi::Collection &collectionSource,
                         const Akonadi::Collection &collectionDestination) override;

private:
    void slotReloadConfig();
    Q_REQUIRED_RESULT Akonadi::Collection::List createCollectionsForDirectory(const QDir &parentDirectory, const Akonadi::Collection &parentCollection) const;
    Q_REQUIRED_RESULT QString baseDirectoryPath() const;
    void initializeDirectory(const QString &path) const;
    Q_REQUIRED_RESULT Akonadi::Collection::Rights supportedRights(bool isResourceCollection) const;
    Q_REQUIRED_RESULT QString directoryForCollection(const Akonadi::Collection &collection) const;
    Q_REQUIRED_RESULT bool doRetrieveItem(Akonadi::Item &item);

private:
    QStringList mSupportedMimeTypes;
};
