/*
   SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "peopleconversionjob.h"

#include <Akonadi/CollectionFetchJob>

#include <KContacts/Addressee>

#include <KGAPI/People/ContactGroupMembership>
#include <KGAPI/People/Membership>
#include <KGAPI/People/Person>

#include "googlepeople_debug.h"

using namespace Akonadi;
using namespace KGAPI2;

PeopleConversionJob::PeopleConversionJob(const Item::List &peopleItems, QObject *parent)
    : QObject(parent)
    , m_items(peopleItems)
{
}

People::PersonList PeopleConversionJob::people() const
{
    return m_processedPeople;
}

QString PeopleConversionJob::reparentCollectionRemoteId() const
{
    return m_reparentCollectionRemoteId;
}

void PeopleConversionJob::setReparentCollectionRemoteId(const QString &reparentCollectionRemoteId)
{
    if (reparentCollectionRemoteId == m_reparentCollectionRemoteId) {
        return;
    }

    m_reparentCollectionRemoteId = reparentCollectionRemoteId;
    Q_EMIT reparentCollectionRemoteIdChanged();
}

QString PeopleConversionJob::newLinkedCollectionRemoteId() const
{
    return m_newLinkedCollectionRemoteId;
}

void PeopleConversionJob::setNewLinkedCollectionRemoteId(const QString &newLinkedCollectionRemoteId)
{
    if (newLinkedCollectionRemoteId == m_newLinkedCollectionRemoteId) {
        return;
    }

    m_newLinkedCollectionRemoteId = newLinkedCollectionRemoteId;
    Q_EMIT newLinkedCollectionRemoteIdChanged();
}

QString PeopleConversionJob::linkedCollectionToRemoveRemoteId() const
{
    return m_linkedCollectionToRemoveRemoteId;
}

void PeopleConversionJob::setLinkedCollectionToRemoveRemoteId(const QString &linkedCollectionToRemoveRemoteId)
{
    if (linkedCollectionToRemoveRemoteId == m_linkedCollectionToRemoveRemoteId) {
        return;
    }

    m_linkedCollectionToRemoveRemoteId = linkedCollectionToRemoveRemoteId;
    Q_EMIT linkedCollectionToRemoveRemoteIdChanged();
}

void PeopleConversionJob::start()
{
    if (m_items.isEmpty()) {
        qCDebug(GOOGLE_PEOPLE_LOG) << "No person items to process, finishing.";
        Q_EMIT finished();
        return;
    }

    m_processedPeople.clear();

    QSet<Collection::Id> collectionsToFetch;
    for (const auto &item : std::as_const(m_items)) {
        const auto itemVirtualReferences = item.virtualReferences();
        for (const auto &collection : itemVirtualReferences) {
            const auto collectionId = collection.id();
            qCDebug(GOOGLE_PEOPLE_LOG) << "Going to fetch data for virtual collection:" << collectionId;
            collectionsToFetch.insert(collectionId);
        };
    }

    if (collectionsToFetch.isEmpty()) {
        qCDebug(GOOGLE_PEOPLE_LOG) << "No virtual collections to fetch, processing people now.";
        processItems();
        return;
    }

    qCDebug(GOOGLE_PEOPLE_LOG) << "Going to fetch data for virtual collections now.";
    const QList<Collection::Id> idList(collectionsToFetch.cbegin(), collectionsToFetch.cend());
    auto collectionRetrievalJob = new CollectionFetchJob(idList);
    connect(collectionRetrievalJob, &CollectionFetchJob::finished, this, &PeopleConversionJob::jobFinished);
}

void PeopleConversionJob::jobFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(GOOGLE_PEOPLE_LOG) << "Virtual collections data fetch ended in error.";
        processItems();
        return;
    }

    qCDebug(GOOGLE_PEOPLE_LOG) << "Processing fetched collections";

    const auto fetchJob = qobject_cast<CollectionFetchJob *>(job);
    const auto collections = fetchJob->collections();

    for (const auto &collection : collections) {
        qCDebug(GOOGLE_PEOPLE_LOG) << "Fetched data for virtual collection:" << collection.displayName();
        m_localToRemoteIdHash.insert(collection.id(), collection.remoteId());
    }

    processItems();
}

People::Membership PeopleConversionJob::resourceNameToMembership(const QString &resourceName)
{
    People::ContactGroupMembership contactGroupMembership;
    contactGroupMembership.setContactGroupResourceName(resourceName);

    People::Membership membership;
    membership.setContactGroupMembership(contactGroupMembership);

    return membership;
}

void PeopleConversionJob::processItems()
{
    for (const auto &item : std::as_const(m_items)) {
        const auto addressee = item.payload<KContacts::Addressee>();
        const auto person = People::Person::fromKContactsAddressee(addressee);
        QVector<People::Membership> memberships;

        person->setResourceName(item.remoteId());
        person->setEtag(item.remoteRevision());

        // TODO: Domain membership?
        const auto parentCollectionRemoteId = m_reparentCollectionRemoteId.isEmpty() ?
            item.parentCollection().remoteId() : m_reparentCollectionRemoteId;
        People::ContactGroupMembership contactGroupMembership;
        contactGroupMembership.setContactGroupResourceName(parentCollectionRemoteId);

        People::Membership membership;
        membership.setContactGroupMembership(contactGroupMembership);
        memberships.append(membership);

        for (const auto &virtualCollection : item.virtualReferences()) {

            const auto virtualCollectionId = virtualCollection.id();
            if (!m_localToRemoteIdHash.contains(virtualCollectionId)) {
                qCWarning(GOOGLE_PEOPLE_LOG) << "Fetched virtual collections do not contain collection with ID:" << virtualCollectionId;
                continue;
            }

            const auto retrievedCollectionRemoteId = m_localToRemoteIdHash.value(virtualCollectionId);
            if (retrievedCollectionRemoteId == m_linkedCollectionToRemoveRemoteId) {
                // Skip adding this membership so it will be removed serverside
                continue;
            }

            const auto existingLinkedMembership = resourceNameToMembership(retrievedCollectionRemoteId);
            memberships.append(existingLinkedMembership);
        }

        if (!m_newLinkedCollectionRemoteId.isEmpty()) {
            const auto newLinkedMembership = resourceNameToMembership(m_newLinkedCollectionRemoteId);
            memberships.append(newLinkedMembership);
        }

        person->setMemberships(memberships);

        qCDebug(GOOGLE_PEOPLE_LOG) << "Processed person:" << person->resourceName();
        m_processedPeople.append(person);
    }

    Q_EMIT peopleChanged();
    Q_EMIT finished();
}

