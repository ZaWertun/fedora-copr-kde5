/*
   SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QString>

#include <Akonadi/Item>
#include <KGAPI/People/Person>
#include <KGAPI/Types>

#pragma once

namespace KGAPI2::People {
class Membership;
}

// Job to convert Akonadi items into KGAPI2 People with all the needed data.
class PeopleConversionJob : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KGAPI2::People::PersonList people READ people NOTIFY peopleChanged)
    Q_PROPERTY(QString reparentCollectionRemoteId READ reparentCollectionRemoteId WRITE setReparentCollectionRemoteId NOTIFY reparentCollectionRemoteIdChanged)
    Q_PROPERTY(QString newLinkedCollectionRemoteId READ newLinkedCollectionRemoteId WRITE setNewLinkedCollectionRemoteId NOTIFY newLinkedCollectionRemoteIdChanged)
    Q_PROPERTY(QString linkedCollectionToRemoveRemoteId READ linkedCollectionToRemoveRemoteId WRITE setLinkedCollectionToRemoveRemoteId NOTIFY linkedCollectionToRemoveRemoteIdChanged)

public:
    explicit PeopleConversionJob(const Akonadi::Item::List &peopleItems, QObject *parent = nullptr);

    Q_REQUIRED_RESULT KGAPI2::People::PersonList people() const;
    Q_REQUIRED_RESULT QString reparentCollectionRemoteId() const;
    Q_REQUIRED_RESULT QString newLinkedCollectionRemoteId() const;
    Q_REQUIRED_RESULT QString linkedCollectionToRemoveRemoteId() const;

Q_SIGNALS:
    void finished();
    void peopleChanged();
    void reparentCollectionRemoteIdChanged();
    void newLinkedCollectionRemoteIdChanged();
    void linkedCollectionToRemoveRemoteIdChanged();

public Q_SLOTS:
    void start();
    void setReparentCollectionRemoteId(const QString &reparentCollectionRemoteId);
    void setNewLinkedCollectionRemoteId(const QString &newLinkedCollectionRemoteId);
    void setLinkedCollectionToRemoveRemoteId(const QString &linkedCollectionToRemoveRemoteId);

private Q_SLOTS:
    void jobFinished(KJob *job);
    void processItems();

private:
    Q_REQUIRED_RESULT KGAPI2::People::Membership resourceNameToMembership(const QString &resourceName);

    Akonadi::Item::List m_items;
    QHash<Akonadi::Collection::Id, QString> m_localToRemoteIdHash;
    KGAPI2::People::PersonList m_processedPeople;
    QString m_reparentCollectionRemoteId;
    QString m_newLinkedCollectionRemoteId;
    QString m_linkedCollectionToRemoveRemoteId;
};
