/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addcollectiontask.h"

#include <Akonadi/CollectionAnnotationsAttribute>

#include "imapresource_debug.h"
#include <KLocalizedString>

#include <KIMAP/CreateJob>
#include <KIMAP/Session>
#include <KIMAP/SetMetaDataJob>
#include <KIMAP/SubscribeJob>

#include <Akonadi/CollectionDeleteJob>

AddCollectionTask::AddCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

AddCollectionTask::~AddCollectionTask() = default;

void AddCollectionTask::doStart(KIMAP::Session *session)
{
    if (parentCollection().remoteId().isEmpty()) {
        qCWarning(IMAPRESOURCE_LOG) << "Parent collection has no remote id, aborting." << collection().name() << parentCollection().name();
        emitError(i18n("Cannot add IMAP folder '%1' for a non-existing parent folder '%2'.", collection().name(), parentCollection().name()));
        changeProcessed();
        return;
    }

    const QChar separator = separatorCharacter();
    m_pendingJobs = 0;
    m_session = session;
    m_collection = collection();
    m_collection.setName(m_collection.name().remove(separator));
    m_collection.setRemoteId(separator + m_collection.name());

    QString newMailBox = mailBoxForCollection(parentCollection());

    if (!newMailBox.isEmpty()) {
        newMailBox += separator;
    }

    newMailBox += m_collection.name();

    qCDebug(IMAPRESOURCE_LOG) << "New folder: " << newMailBox;

    auto job = new KIMAP::CreateJob(session);
    job->setMailBox(newMailBox);

    connect(job, &KIMAP::CreateJob::result, this, &AddCollectionTask::onCreateDone);

    job->start();
}

void AddCollectionTask::onCreateDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to create folder on server: " << job->errorString();
        emitError(i18n("Failed to create the folder '%1' on the IMAP server. ", m_collection.name()));
        cancelTask(job->errorString());
    } else {
        // Automatically subscribe to newly created mailbox
        auto create = static_cast<KIMAP::CreateJob *>(job);

        auto subscribe = new KIMAP::SubscribeJob(create->session());
        subscribe->setMailBox(create->mailBox());

        connect(subscribe, &KIMAP::SubscribeJob::result, this, &AddCollectionTask::onSubscribeDone);

        subscribe->start();
    }
}

void AddCollectionTask::onSubscribeDone(KJob *job)
{
    if (job->error() && isSubscriptionEnabled()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to subscribe to the new folder: " << job->errorString();
        emitWarning(
            i18n("Failed to subscribe to the folder '%1' on the IMAP server. "
                 "It will disappear on next sync. Use the subscription dialog to overcome that",
                 m_collection.name()));
    }

    const Akonadi::CollectionAnnotationsAttribute *attribute = m_collection.attribute<Akonadi::CollectionAnnotationsAttribute>();
    if (!attribute || !serverSupportsAnnotations()) {
        // we are finished
        changeCommitted(m_collection);
        synchronizeCollectionTree();
        return;
    }

    QMapIterator<QByteArray, QByteArray> i(attribute->annotations());
    while (i.hasNext()) {
        i.next();
        auto job = new KIMAP::SetMetaDataJob(m_session);
        if (serverCapabilities().contains(QLatin1String("METADATA"))) {
            job->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        } else {
            job->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        }
        job->setMailBox(mailBoxForCollection(m_collection));

        if (!i.key().startsWith("/shared") && !i.key().startsWith("/private")) {
            // Support for legacy annotations that don't include the prefix
            job->addMetaData(QByteArray("/shared") + i.key(), i.value());
        } else {
            job->addMetaData(i.key(), i.value());
        }

        connect(job, &KIMAP::SetMetaDataJob::result, this, &AddCollectionTask::onSetMetaDataDone);

        m_pendingJobs++;

        job->start();
    }
}

void AddCollectionTask::onSetMetaDataDone(KJob *job)
{
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Failed to write annotations: " << job->errorString();
        emitWarning(i18n("Failed to write some annotations for '%1' on the IMAP server. %2", collection().name(), job->errorText()));
    }

    m_pendingJobs--;

    if (m_pendingJobs == 0) {
        changeCommitted(m_collection);
    }
}
