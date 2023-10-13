/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "retrievecollectionmetadatatask.h"

#include <KIMAP/GetAclJob>
#include <KIMAP/GetMetaDataJob>
#include <KIMAP/GetQuotaRootJob>
#include <KIMAP/MyRightsJob>
#include <KIMAP/RFCCodecs>
#include <KIMAP/Session>
#include <KLocalizedString>

#include "imapresource_debug.h"

#include "collectionmetadatahelper.h"
#include "imapaclattribute.h"
#include "imapquotaattribute.h"
#include "noselectattribute.h"
#include <Akonadi/CollectionAnnotationsAttribute>
#include <Akonadi/CollectionQuotaAttribute>
#include <Akonadi/EntityDisplayAttribute>

RetrieveCollectionMetadataTask::RetrieveCollectionMetadataTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
{
}

RetrieveCollectionMetadataTask::~RetrieveCollectionMetadataTask() = default;

void RetrieveCollectionMetadataTask::doStart(KIMAP::Session *session)
{
    qCDebug(IMAPRESOURCE_LOG) << collection().remoteId();

    // Prevent fetching metadata from noselect folders.
    if (collection().hasAttribute("noselect")) {
        NoSelectAttribute *noselect = static_cast<NoSelectAttribute *>(collection().attribute("noselect"));
        if (noselect->noSelect()) {
            qCDebug(IMAPRESOURCE_LOG) << "No Select folder";
            endTaskIfNeeded();
            return;
        }
    }

    m_session = session;
    m_collection = collection();
    const QString mailBox = mailBoxForCollection(m_collection);
    const QStringList capabilities = serverCapabilities();

    m_pendingMetaDataJobs = 0;

    // First get the annotations from the mailbox if it's supported
    if (capabilities.contains(QLatin1String("METADATA")) || capabilities.contains(QLatin1String("ANNOTATEMORE"))) {
        auto meta = new KIMAP::GetMetaDataJob(session);
        meta->setMailBox(mailBox);
        if (capabilities.contains(QLatin1String("METADATA"))) {
            meta->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
            meta->addRequestedEntry("/shared");
            meta->setDepth(KIMAP::GetMetaDataJob::AllLevels);
        } else {
            meta->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
            meta->addEntry("*", "value.shared");
        }
        connect(meta, &KJob::result, this, &RetrieveCollectionMetadataTask::onGetMetaDataDone);
        m_pendingMetaDataJobs++;
        meta->start();
    }

    // Get the ACLs from the mailbox if it's supported
    if (capabilities.contains(QLatin1String("ACL"))) {
        auto rights = new KIMAP::MyRightsJob(session);
        rights->setMailBox(mailBox);
        connect(rights, &KJob::result, this, &RetrieveCollectionMetadataTask::onRightsReceived);
        m_pendingMetaDataJobs++;
        rights->start();
    }

    // Get the QUOTA info from the mailbox if it's supported
    if (capabilities.contains(QLatin1String("QUOTA"))) {
        auto quota = new KIMAP::GetQuotaRootJob(session);
        quota->setMailBox(mailBox);
        connect(quota, &KJob::result, this, &RetrieveCollectionMetadataTask::onQuotasReceived);
        m_pendingMetaDataJobs++;
        quota->start();
    }

    // the server does not have any of the capabilities needed to get extra info, so this
    // step is done here
    if (m_pendingMetaDataJobs == 0) {
        endTaskIfNeeded();
    }
}

void RetrieveCollectionMetadataTask::onGetMetaDataDone(KJob *job)
{
    m_pendingMetaDataJobs--;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Get metadata failed: " << job->errorString();
        endTaskIfNeeded();
        return; // Well, no metadata for us then...
    }

    auto meta = qobject_cast<KIMAP::GetMetaDataJob *>(job);
    QMap<QByteArray, QByteArray> rawAnnotations = meta->allMetaData();

    // filter out unused and annoying Cyrus annotation /vendor/cmu/cyrus-imapd/lastupdate
    // which contains the current date and time and thus constantly changes for no good
    // reason which triggers a change notification and thus a bunch of Akonadi operations
    rawAnnotations.remove("/shared/vendor/cmu/cyrus-imapd/lastupdate");
    rawAnnotations.remove("/private/vendor/cmu/cyrus-imapd/lastupdate");

    // Store the mailbox metadata
    auto annotationsAttribute = m_collection.attribute<Akonadi::CollectionAnnotationsAttribute>(Akonadi::Collection::AddIfMissing);
    const QMap<QByteArray, QByteArray> oldAnnotations = annotationsAttribute->annotations();
    if (oldAnnotations != rawAnnotations) {
        annotationsAttribute->setAnnotations(rawAnnotations);
    }

    endTaskIfNeeded();
}

void RetrieveCollectionMetadataTask::onGetAclDone(KJob *job)
{
    m_pendingMetaDataJobs--;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "GetACL failed: " << job->errorString();
        endTaskIfNeeded();
        return; // Well, no metadata for us then...
    }

    auto acl = qobject_cast<KIMAP::GetAclJob *>(job);

    // Store the mailbox ACLs
    auto const aclAttribute = m_collection.attribute<Akonadi::ImapAclAttribute>(Akonadi::Collection::AddIfMissing);
    const QMap<QByteArray, KIMAP::Acl::Rights> oldRights = aclAttribute->rights();
    if (oldRights != acl->allRights()) {
        aclAttribute->setRights(acl->allRights());
    }

    endTaskIfNeeded();
}

void RetrieveCollectionMetadataTask::onRightsReceived(KJob *job)
{
    m_pendingMetaDataJobs--;
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "MyRights failed: " << job->errorString();
        endTaskIfNeeded();
        return; // Well, no metadata for us then...
    }

    auto rightsJob = qobject_cast<KIMAP::MyRightsJob *>(job);

    const KIMAP::Acl::Rights imapRights = rightsJob->rights();

    // Default value in case we have nothing better available
    KIMAP::Acl::Rights parentRights = KIMAP::Acl::CreateMailbox | KIMAP::Acl::Create;

    // FIXME I don't think we have the parent's acl's available
    if (collection().parentCollection().attribute<Akonadi::ImapAclAttribute>()) {
        parentRights = myRights(collection().parentCollection());
    }

    //  qCDebug(IMAPRESOURCE_LOG) << collection.remoteId()
    //                 << "imapRights:" << imapRights
    //                 << "newRights:" << newRights
    //                 << "oldRights:" << collection.rights();

    const bool isNewCollection = !m_collection.hasAttribute<Akonadi::ImapAclAttribute>();
    const bool accessRevoked = CollectionMetadataHelper::applyRights(m_collection, imapRights, parentRights);
    if (accessRevoked && !isNewCollection) {
        // write access revoked
        const QString collectionName = m_collection.displayName();

        showInformationDialog(i18n("<p>Your access rights to folder <b>%1</b> have been restricted, "
                                   "it will no longer be possible to add messages to this folder.</p>",
                                   collectionName),
                              i18n("Access rights revoked"),
                              QStringLiteral("ShowRightsRevokedWarning"));
    }

    // Store the mailbox ACLs
    auto aclAttribute = m_collection.attribute<Akonadi::ImapAclAttribute>(Akonadi::Collection::AddIfMissing);
    const KIMAP::Acl::Rights oldRights = aclAttribute->myRights();
    if (oldRights != imapRights) {
        aclAttribute->setMyRights(imapRights);
    }

    // The a right is required to list acl's
    if (imapRights & KIMAP::Acl::Admin) {
        auto acl = new KIMAP::GetAclJob(m_session);
        acl->setMailBox(mailBoxForCollection(m_collection));
        connect(acl, &KJob::result, this, &RetrieveCollectionMetadataTask::onGetAclDone);
        m_pendingMetaDataJobs++;
        acl->start();
    }

    endTaskIfNeeded();
}

void RetrieveCollectionMetadataTask::onQuotasReceived(KJob *job)
{
    m_pendingMetaDataJobs--;
    const QString &mailBox = mailBoxForCollection(m_collection);
    if (job->error()) {
        qCWarning(IMAPRESOURCE_LOG) << "Quota retrieval for mailbox " << mailBox << " failed: " << job->errorString();
        endTaskIfNeeded();
        return; // Well, no metadata for us then...
    }

    auto quotaJob = qobject_cast<KIMAP::GetQuotaRootJob *>(job);

    QList<QByteArray> allRoots = quotaJob->roots();
    QList<QByteArray> newRoots;
    QList<QMap<QByteArray, qint64>> newLimits;
    QList<QMap<QByteArray, qint64>> newUsages;
    qint64 newCurrent = -1;
    qint64 newMax = -1;
    newRoots.reserve(allRoots.count());
    newLimits.reserve(allRoots.count());
    newUsages.reserve(allRoots.count());

    for (const QByteArray &root : std::as_const(allRoots)) {
        const QMap<QByteArray, qint64> limit = quotaJob->allLimits(root);
        const QMap<QByteArray, qint64> usage = quotaJob->allUsages(root);

        // Process IMAP Quota roots with associated quotas only
        if (!limit.isEmpty() && !usage.isEmpty()) {
            newRoots << root;
            newLimits << limit;
            newUsages << usage;

            const QString &decodedRoot = QString::fromUtf8(KIMAP::decodeImapFolderName(root));

            if (decodedRoot == mailBox) {
                newCurrent = newUsages.last()["STORAGE"] * 1024;
                newMax = newLimits.last()["STORAGE"] * 1024;
            }
        }
    }

    // If usage and limit were not set, retrieve them from the first root, if exists
    if ((newCurrent == -1 && newMax == -1) && !newRoots.isEmpty()) {
        newCurrent = newUsages.first()["STORAGE"] * 1024;
        newMax = newLimits.first()["STORAGE"] * 1024;
    }

    // Store the mailbox IMAP Quotas
    auto imapQuotaAttribute = m_collection.attribute<Akonadi::ImapQuotaAttribute>(Akonadi::Collection::AddIfMissing);
    const QList<QByteArray> oldRoots = imapQuotaAttribute->roots();
    const QList<QMap<QByteArray, qint64>> oldLimits = imapQuotaAttribute->limits();
    const QList<QMap<QByteArray, qint64>> oldUsages = imapQuotaAttribute->usages();

    if (oldRoots != newRoots || oldLimits != newLimits || oldUsages != newUsages) {
        imapQuotaAttribute->setQuotas(newRoots, newLimits, newUsages);
    }

    // Store the collection Quota
    auto quotaAttribute = m_collection.attribute<Akonadi::CollectionQuotaAttribute>(Akonadi::Collection::AddIfMissing);
    qint64 oldCurrent = quotaAttribute->currentValue();
    qint64 oldMax = quotaAttribute->maximumValue();

    if (oldCurrent != newCurrent || oldMax != newMax) {
        quotaAttribute->setCurrentValue(newCurrent);
        quotaAttribute->setMaximumValue(newMax);
    }

    endTaskIfNeeded();
}

void RetrieveCollectionMetadataTask::endTaskIfNeeded()
{
    if (m_pendingMetaDataJobs <= 0) {
        collectionAttributesRetrieved(m_collection);
    }
}
