/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "changecollectiontask.h"

#include <KIMAP/RenameJob>
#include <KIMAP/Session>
#include <KIMAP/SetAclJob>
#include <KIMAP/SetMetaDataJob>
#include <KIMAP/SubscribeJob>
#include <KIMAP/UnsubscribeJob>

#include "imapaclattribute.h"
#include "imapquotaattribute.h"
#include <Akonadi/CollectionAnnotationsAttribute>

#include "imapresource_debug.h"
#include <KLocalizedString>

ChangeCollectionTask::ChangeCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(DeferIfNoSession, resource, parent)
{
}

ChangeCollectionTask::~ChangeCollectionTask() = default;

void ChangeCollectionTask::syncEnabledState(bool enable)
{
    m_syncEnabledState = enable;
}

void ChangeCollectionTask::doStart(KIMAP::Session *session)
{
    if (collection().remoteId().isEmpty()) {
        emitError(i18n("Cannot modify IMAP folder '%1', it does not exist on the server.", collection().name()));
        changeProcessed();
        return;
    }

    m_collection = collection();
    m_pendingJobs = 0;

    if (parts().contains("AccessRights")) {
        auto aclAttribute = m_collection.attribute<Akonadi::ImapAclAttribute>();

        if (aclAttribute == nullptr) {
            emitWarning(i18n("ACLs for '%1' need to be retrieved from the IMAP server first. Skipping ACL change", collection().name()));
        } else {
            KIMAP::Acl::Rights imapRights = aclAttribute->rights().value(userName().toUtf8());
            Akonadi::Collection::Rights newRights = collection().rights();

            if (newRights & Akonadi::Collection::CanChangeItem) {
                imapRights |= KIMAP::Acl::Write;
            } else {
                imapRights &= ~KIMAP::Acl::Write;
            }

            if (newRights & Akonadi::Collection::CanCreateItem) {
                imapRights |= KIMAP::Acl::Insert;
            } else {
                imapRights &= ~KIMAP::Acl::Insert;
            }

            if (newRights & Akonadi::Collection::CanDeleteItem) {
                imapRights |= KIMAP::Acl::DeleteMessage;
            } else {
                imapRights &= ~KIMAP::Acl::DeleteMessage;
            }

            if (newRights & (Akonadi::Collection::CanChangeCollection | Akonadi::Collection::CanCreateCollection)) {
                imapRights |= KIMAP::Acl::CreateMailbox;
                imapRights |= KIMAP::Acl::Create;
            } else {
                imapRights &= ~KIMAP::Acl::CreateMailbox;
                imapRights &= ~KIMAP::Acl::Create;
            }

            if (newRights & Akonadi::Collection::CanDeleteCollection) {
                imapRights |= KIMAP::Acl::DeleteMailbox;
            } else {
                imapRights &= ~KIMAP::Acl::DeleteMailbox;
            }

            if ((newRights & Akonadi::Collection::CanDeleteItem) && (newRights & Akonadi::Collection::CanDeleteCollection)) {
                imapRights |= KIMAP::Acl::Delete;
            } else {
                imapRights &= ~KIMAP::Acl::Delete;
            }

            qCDebug(IMAPRESOURCE_LOG) << "imapRights:" << imapRights << "newRights:" << newRights;

            auto job = new KIMAP::SetAclJob(session);
            job->setMailBox(mailBoxForCollection(collection()));
            job->setRights(KIMAP::SetAclJob::Change, imapRights);
            job->setIdentifier(userName().toUtf8());

            connect(job, &KIMAP::SetAclJob::result, this, &ChangeCollectionTask::onSetAclDone);

            job->start();

            m_pendingJobs++;
        }
    }

    if (parts().contains("collectionannotations") && serverSupportsAnnotations()) {
        Akonadi::Collection c = collection();
        auto annotationsAttribute = c.attribute<Akonadi::CollectionAnnotationsAttribute>();

        if (annotationsAttribute) { // No annotations it seems... server is lying to us?
            QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();
            qCDebug(IMAPRESOURCE_LOG) << "All annotations: " << annotations;

            const auto annotationKeys{annotations.keys()};
            for (const QByteArray &entry : annotationKeys) {
                auto job = new KIMAP::SetMetaDataJob(session);
                if (serverCapabilities().contains(QLatin1String("METADATA"))) {
                    job->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
                } else {
                    job->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
                }
                job->setMailBox(mailBoxForCollection(collection()));

                if (!entry.startsWith("/shared") && !entry.startsWith("/private")) {
                    // Support for legacy annotations that don't include the prefix
                    job->addMetaData(QByteArray("/shared") + entry, annotations[entry]);
                } else {
                    job->addMetaData(entry, annotations[entry]);
                }

                qCDebug(IMAPRESOURCE_LOG) << "Job got entry:" << entry << "value:" << annotations[entry];

                connect(job, &KIMAP::SetMetaDataJob::result, this, &ChangeCollectionTask::onSetMetaDataDone);

                job->start();

                m_pendingJobs++;
            }
        }
    }

    if (parts().contains("imapacl")) {
        Akonadi::Collection c = collection();
        auto aclAttribute = c.attribute<Akonadi::ImapAclAttribute>();

        if (aclAttribute) {
            const QMap<QByteArray, KIMAP::Acl::Rights> rights = aclAttribute->rights();
            const QMap<QByteArray, KIMAP::Acl::Rights> oldRights = aclAttribute->oldRights();
            const QList<QByteArray> oldIds = oldRights.keys();
            const QList<QByteArray> ids = rights.keys();

            // remove all ACL entries that have been deleted
            for (const QByteArray &oldId : oldIds) {
                if (!ids.contains(oldId)) {
                    auto job = new KIMAP::SetAclJob(session);
                    job->setMailBox(mailBoxForCollection(collection()));
                    job->setIdentifier(oldId);
                    job->setRights(KIMAP::SetAclJob::Remove, oldRights[oldId]);

                    connect(job, &KIMAP::SetAclJob::result, this, &ChangeCollectionTask::onSetAclDone);

                    job->start();

                    m_pendingJobs++;
                }
            }

            for (const QByteArray &id : ids) {
                auto job = new KIMAP::SetAclJob(session);
                job->setMailBox(mailBoxForCollection(collection()));
                job->setIdentifier(id);
                job->setRights(KIMAP::SetAclJob::Change, rights[id]);

                connect(job, &KIMAP::SetAclJob::result, this, &ChangeCollectionTask::onSetAclDone);

                job->start();

                m_pendingJobs++;
            }
        }
    }

    // Check if we need to rename the mailbox
    // This one goes last on purpose, we don't want the previous jobs
    // we triggered to act on the wrong mailbox name
    if (parts().contains("NAME")) {
        const QChar separator = separatorCharacter();
        m_collection.setName(m_collection.name().remove(separator));
        m_collection.setRemoteId(separator + m_collection.name());

        const QString oldMailBox = mailBoxForCollection(collection());
        const QString newMailBox = mailBoxForCollection(m_collection);

        if (oldMailBox != newMailBox) {
            auto renameJob = new KIMAP::RenameJob(session);
            renameJob->setSourceMailBox(oldMailBox);
            renameJob->setDestinationMailBox(newMailBox);
            connect(renameJob, &KIMAP::RenameJob::result, this, &ChangeCollectionTask::onRenameDone);

            renameJob->start();

            m_pendingJobs++;
        }
    }

    if (m_syncEnabledState && isSubscriptionEnabled() && parts().contains("ENABLED")) {
        if (collection().enabled()) {
            auto job = new KIMAP::SubscribeJob(session);
            job->setMailBox(mailBoxForCollection(collection()));
            connect(job, &KIMAP::SubscribeJob::result, this, &ChangeCollectionTask::onSubscribeDone);
            job->start();
        } else {
            auto job = new KIMAP::UnsubscribeJob(session);
            job->setMailBox(mailBoxForCollection(collection()));
            connect(job, &KIMAP::UnsubscribeJob::result, this, &ChangeCollectionTask::onSubscribeDone);
            job->start();
        }
        m_pendingJobs++;
    }

    // we scheduled no change on the server side, probably we got only
    // unsupported part, so just declare the task done
    if (m_pendingJobs == 0) {
        changeCommitted(collection());
    }
}

void ChangeCollectionTask::onRenameDone(KJob *job)
{
    if (job->error()) {
        const QString prevRid = collection().remoteId();
        Q_ASSERT(!prevRid.isEmpty());

        emitWarning(i18n("Failed to rename the folder, restoring folder list."));

        m_collection.setName(prevRid.mid(1));
        m_collection.setRemoteId(prevRid);

        endTaskIfNeeded();
    } else {
        auto renameJob = static_cast<KIMAP::RenameJob *>(job);
        auto subscribeJob = new KIMAP::SubscribeJob(renameJob->session());
        subscribeJob->setMailBox(renameJob->destinationMailBox());
        connect(subscribeJob, &KIMAP::SubscribeJob::result, this, &ChangeCollectionTask::onSubscribeDone);
        subscribeJob->start();
    }
}

void ChangeCollectionTask::onSubscribeDone(KJob *job)
{
    if (job->error() && isSubscriptionEnabled()) {
        emitWarning(
            i18n("Failed to subscribe to the renamed folder '%1' on the IMAP server. "
                 "It will disappear on next sync. Use the subscription dialog to overcome that",
                 m_collection.name()));
    }

    endTaskIfNeeded();
}

void ChangeCollectionTask::onSetAclDone(KJob *job)
{
    if (job->error()) {
        emitWarning(i18n("Failed to write some ACLs for '%1' on the IMAP server. %2", collection().name(), job->errorText()));
    }

    endTaskIfNeeded();
}

void ChangeCollectionTask::onSetMetaDataDone(KJob *job)
{
    if (job->error()) {
        emitWarning(i18n("Failed to write some annotations for '%1' on the IMAP server. %2", collection().name(), job->errorText()));
    }

    endTaskIfNeeded();
}

void ChangeCollectionTask::endTaskIfNeeded()
{
    if (--m_pendingJobs == 0) {
        // the others have ended, we're done, the next one can go
        changeCommitted(m_collection);
    }
}
