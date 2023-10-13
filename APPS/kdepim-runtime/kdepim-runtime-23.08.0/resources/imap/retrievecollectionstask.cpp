/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrievecollectionstask.h"

#include "noinferiorsattribute.h"
#include "noselectattribute.h"

#include <Akonadi/CachePolicy>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/MessageParts>
#include <Akonadi/SpecialCollectionAttribute>
#include <Akonadi/VectorHelper>

#include <KMime/Message>

#include "imapresource_debug.h"
#include <KLocalizedString>

RetrieveCollectionsTask::RetrieveCollectionsTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
{
}

RetrieveCollectionsTask::~RetrieveCollectionsTask() = default;

void RetrieveCollectionsTask::doStart(KIMAP::Session *session)
{
    Akonadi::Collection root;
    root.setName(resourceName());
    root.setRemoteId(rootRemoteId());
    root.setContentMimeTypes(QStringList(Akonadi::Collection::mimeType()));
    root.setParentCollection(Akonadi::Collection::root());
    root.addAttribute(new NoSelectAttribute(true));

    Akonadi::CachePolicy policy;
    policy.setInheritFromParent(false);
    policy.setSyncOnDemand(true);

    // The first in the list of namespaces is User namespace
    // If the user namespace is empty, then make it possible for user to create
    // new folders as children of the root folder
    if (serverNamespaces().value(0).name.isEmpty()) {
        root.setRights(Akonadi::Collection::CanCreateCollection);
    }

    QStringList localParts;
    localParts << QLatin1String(Akonadi::MessagePart::Envelope) << QLatin1String(Akonadi::MessagePart::Header);
    int cacheTimeout = 60;

    if (isDisconnectedModeEnabled()) {
        // For disconnected mode we also cache the body
        // and we keep all data indefinitely
        localParts << QLatin1String(Akonadi::MessagePart::Body);
        cacheTimeout = -1;
    }

    policy.setLocalParts(localParts);
    policy.setCacheTimeout(cacheTimeout);
    policy.setIntervalCheckTime(intervalCheckTime());

    root.setCachePolicy(policy);

    m_reportedCollections.insert(QString(), root);

    // this is ugly, but the result of LSUB is unfortunately not a sub-set of LIST
    // it also contains subscribed but currently not available (eg. deleted) mailboxes
    // so we need to use both and exclude mailboxes in LSUB but not in LIST
    if (isSubscriptionEnabled()) {
        auto fullListJob = new KIMAP::ListJob(session);
        fullListJob->setOption(KIMAP::ListJob::IncludeUnsubscribed);
        fullListJob->setQueriedNamespaces(serverNamespaces());
        connect(fullListJob, &KIMAP::ListJob::mailBoxesReceived, this, &RetrieveCollectionsTask::onFullMailBoxesReceived);
        connect(fullListJob, &KIMAP::ListJob::result, this, &RetrieveCollectionsTask::onFullMailBoxesReceiveDone);
        fullListJob->start();
    }

    auto listJob = new KIMAP::ListJob(session);
    listJob->setIncludeUnsubscribed(!isSubscriptionEnabled());
    listJob->setQueriedNamespaces(serverNamespaces());
    connect(listJob, &KIMAP::ListJob::mailBoxesReceived, this, &RetrieveCollectionsTask::onMailBoxesReceived);
    connect(listJob, &KIMAP::ListJob::result, this, &RetrieveCollectionsTask::onMailBoxesReceiveDone);
    listJob->start();
}

void RetrieveCollectionsTask::onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags)
{
    const QStringList contentTypes = {KMime::Message::mimeType(), Akonadi::Collection::mimeType()};

    if (!descriptors.isEmpty()) {
        // This is still not optimal way of getting the separator, but it's better
        // than guessing every time from RID of parent collection
        setSeparatorCharacter(descriptors.first().separator);
    }

    for (int i = 0; i < descriptors.size(); ++i) {
        KIMAP::MailBoxDescriptor descriptor = descriptors[i];

        // skip phantom mailboxes contained in LSUB but not LIST
        if (isSubscriptionEnabled() && !m_fullReportedCollections.contains(descriptor.name)) {
            qCDebug(IMAPRESOURCE_LOG) << "Got phantom mailbox: " << descriptor.name;
            continue;
        }

        const QString separator = descriptor.separator;
        Q_ASSERT(separator.size() == 1); // that's what the spec says

        const QString boxName = descriptor.name.endsWith(separator) ? descriptor.name.left(descriptor.name.size() - 1) : descriptor.name;

        const QStringList pathParts = boxName.split(separator);

        QString parentPath;
        QString currentPath;

        const int pathPartsSize(pathParts.size());
        for (int j = 0; j < pathPartsSize; ++j) {
            const bool isDummy = j != pathPartsSize - 1;
            const QString pathPart = pathParts.at(j);
            currentPath += separator + pathPart;

            if (m_reportedCollections.contains(currentPath)) {
                if (m_dummyCollections.contains(currentPath) && !isDummy) {
                    qCDebug(IMAPRESOURCE_LOG) << "Received the real collection for a dummy one : " << currentPath;

                    // set the correct attributes for the collection, eg. noselect needs to be removed
                    Akonadi::Collection c = m_reportedCollections.value(currentPath);
                    c.setContentMimeTypes(contentTypes);
                    c.setRights(Akonadi::Collection::AllRights);
                    c.removeAttribute<NoSelectAttribute>();

                    m_dummyCollections.remove(currentPath);
                    m_reportedCollections.remove(currentPath);
                    m_reportedCollections.insert(currentPath, c);
                }
                parentPath = currentPath;
                continue;
            }

            const QList<QByteArray> currentFlags = isDummy ? (QList<QByteArray>() << "\\noselect") : flags[i];

            Akonadi::Collection c;
            c.setName(pathPart);
            c.setRemoteId(separator + pathPart);
            const Akonadi::Collection parentCollection = m_reportedCollections.value(parentPath);
            c.setParentCollection(parentCollection);
            c.setContentMimeTypes(contentTypes);

            // If the folder is the Inbox, make some special settings.
            if (currentPath.compare(separator + QLatin1String("INBOX"), Qt::CaseInsensitive) == 0) {
                auto attr = c.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
                attr->setDisplayName(i18n("Inbox"));
                attr->setIconName(QStringLiteral("mail-folder-inbox"));
                c.attribute<Akonadi::SpecialCollectionAttribute>(Akonadi::Collection::AddIfMissing)->setCollectionType("inbox");
                setIdleCollection(c);
            }

            // If the folder is the user top-level folder, mark it as well, even although it is not officially noted in the RFC
            if (currentPath == (separator + QLatin1String("user")) && currentFlags.contains("\\noselect")) {
                auto attr = c.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
                attr->setDisplayName(i18n("Shared Folders"));
                attr->setIconName(QStringLiteral("x-mail-distribution-list"));
            }

            // If this folder is a noselect folder, make some special settings.
            if (currentFlags.contains("\\noselect")) {
                qCDebug(IMAPRESOURCE_LOG) << "Dummy collection created: " << currentPath;
                c.addAttribute(new NoSelectAttribute(true));
                c.setContentMimeTypes(QStringList() << Akonadi::Collection::mimeType());
                c.setRights(Akonadi::Collection::ReadOnly);
            } else {
                // remove the noselect attribute explicitly, in case we had set it before (eg. for non-subscribed non-leaf folders)
                c.removeAttribute<NoSelectAttribute>();
            }

            // If this folder is a noinferiors folder, it is not allowed to create subfolders inside.
            if (currentFlags.contains("\\noinferiors")) {
                // qCDebug(IMAPRESOURCE_LOG) << "Noinferiors: " << currentPath;
                c.addAttribute(new NoInferiorsAttribute(true));
                c.setRights(c.rights() & ~Akonadi::Collection::CanCreateCollection);
            }

            m_reportedCollections.insert(currentPath, c);

            if (isDummy) {
                m_dummyCollections.insert(currentPath, c);
            }

            parentPath = currentPath;
        }
    }
}

void RetrieveCollectionsTask::onMailBoxesReceiveDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    } else {
        collectionsRetrieved(Akonadi::valuesToVector(m_reportedCollections));
    }
}

void RetrieveCollectionsTask::onFullMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags)
{
    Q_UNUSED(flags)
    for (const KIMAP::MailBoxDescriptor &descriptor : descriptors) {
        m_fullReportedCollections.insert(descriptor.name);
    }
}

void RetrieveCollectionsTask::onFullMailBoxesReceiveDone(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorString());
    }
}
