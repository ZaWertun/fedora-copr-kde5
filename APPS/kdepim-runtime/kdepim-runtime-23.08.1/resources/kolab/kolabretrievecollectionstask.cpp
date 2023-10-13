/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabretrievecollectionstask.h"
#include "kolabhelpers.h"
#include "kolabresource_debug.h"
#include "kolabresource_trace.h"

#include <Akonadi/CollectionAnnotationsAttribute>
#include <KIMAP/GetMetaDataJob>
#include <KIMAP/MyRightsJob>
#include <collectionmetadatahelper.h>
#include <imapaclattribute.h>
#include <noinferiorsattribute.h>
#include <noselectattribute.h>

#include <Akonadi/MessageParts>

#include <Akonadi/BlockAlarmsAttribute>
#include <Akonadi/CachePolicy>
#include <Akonadi/CollectionColorAttribute>
#include <Akonadi/CollectionIdentificationAttribute>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/SpecialCollectionAttribute>
#include <Akonadi/VectorHelper>

#include <KMime/Message>

#include <KLocalizedString>

#include <QColor>

static bool isNamespaceFolder(const QString &path, const QList<KIMAP::MailBoxDescriptor> &namespaces, bool matchCompletePath = false)
{
    for (const KIMAP::MailBoxDescriptor &desc : namespaces) {
        if (path.startsWith(desc.name.left(desc.name.size() - 1))) { // Namespace ends with path separator and path doesn't
            if (!matchCompletePath || path.size() - desc.name.size() <= 1) { // We want to match only for the complete path
                return true;
            }
        }
    }
    return false;
}

RetrieveMetadataJob::RetrieveMetadataJob(KIMAP::Session *session,
                                         const QStringList &mailboxes,
                                         const QStringList &serverCapabilities,
                                         const QSet<QByteArray> &requestedMetadata,
                                         const QString &separator,
                                         const QList<KIMAP::MailBoxDescriptor> &sharedNamespace,
                                         const QList<KIMAP::MailBoxDescriptor> &userNamespace,
                                         QObject *parent)
    : KJob(parent)
    , mRequestedMetadata(requestedMetadata)
    , mServerCapabilities(serverCapabilities)
    , mMailboxes(mailboxes)
    , mSession(session)
    , mSeparator(separator)
    , mSharedNamespace(sharedNamespace)
    , mUserNamespace(userNamespace)
{
}

void RetrieveMetadataJob::start()
{
    qCDebug(KOLABRESOURCE_TRACE);
    // Fill the map with empty entries so we set the mimetype to mail if no metadata is retrieved
    for (const QString &mailbox : std::as_const(mMailboxes)) {
        mMetadata.insert(mailbox, QMap<QByteArray, QByteArray>());
    }

    if (mServerCapabilities.contains(QLatin1String("METADATA")) || mServerCapabilities.contains(QLatin1String("ANNOTATEMORE"))) {
        QSet<QString> toplevelMailboxes;
        for (const QString &mailbox : std::as_const(mMailboxes)) {
            const QStringList parts = mailbox.split(mSeparator);
            if (!parts.isEmpty()) {
                if (isNamespaceFolder(mailbox, mUserNamespace) && parts.length() >= 2) {
                    // Other Users can be too big to request with a single command so we request Other Users/<user>/*
                    toplevelMailboxes << parts.at(0) + mSeparator + parts.at(1) + mSeparator;
                } else if (!isNamespaceFolder(mailbox, mSharedNamespace)) {
                    toplevelMailboxes << parts.first();
                }
            }
        }
        for (const KIMAP::MailBoxDescriptor &desc : std::as_const(mSharedNamespace)) {
            toplevelMailboxes << desc.name;
        }
        // TODO perhaps exclude the shared and other users namespaces by listing only toplevel (with %), and then only getting metadata of the toplevel folders.
        for (const QString &mailbox : std::as_const(toplevelMailboxes)) {
            auto meta = new KIMAP::GetMetaDataJob(mSession);
            meta->setMailBox(mailbox + QLatin1String("*"));
            if (mServerCapabilities.contains(QLatin1String("METADATA"))) {
                meta->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
            } else {
                meta->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
            }
            meta->setDepth(KIMAP::GetMetaDataJob::AllLevels);
            for (const QByteArray &requestedEntry : std::as_const(mRequestedMetadata)) {
                meta->addRequestedEntry(requestedEntry);
            }
            connect(meta, &KJob::result, this, &RetrieveMetadataJob::onGetMetaDataDone);
            mJobs++;
            meta->start();
        }
    }

    // Get the ACLs from the mailbox if it's supported
    if (mServerCapabilities.contains(QLatin1String("ACL"))) {
        for (const QString &mailbox : std::as_const(mMailboxes)) {
            // "Shared Folders" is not a valid mailbox, so we have to skip the ACL request for this folder
            if (isNamespaceFolder(mailbox, mSharedNamespace, true)) {
                continue;
            }
            auto rights = new KIMAP::MyRightsJob(mSession);
            rights->setMailBox(mailbox);
            connect(rights, &KJob::result, this, &RetrieveMetadataJob::onRightsReceived);
            mJobs++;
            rights->start();
        }
    }
    checkDone();
}

void RetrieveMetadataJob::onGetMetaDataDone(KJob *job)
{
    mJobs--;
    auto meta = static_cast<KIMAP::GetMetaDataJob *>(job);
    if (job->error()) {
        qCDebug(KOLABRESOURCE_LOG) << "No metadata for for mailbox: " << meta->mailBox();
        if (!isNamespaceFolder(meta->mailBox(), mSharedNamespace)) {
            qCWarning(KOLABRESOURCE_LOG) << "Get metadata failed: " << job->errorString();
            // We ignore the error to avoid failing the complete sync. We can run into this when trying to retrieve rights for non-existing mailboxes.
        }
        checkDone();
        return;
    }

    const QHash<QString, QMap<QByteArray, QByteArray>> metadata = meta->allMetaDataForMailboxes();
    const QStringList lstKeys = metadata.keys();
    for (const QString &folder : lstKeys) {
        mMetadata.insert(folder, metadata.value(folder));
    }
    checkDone();
}

void RetrieveMetadataJob::onRightsReceived(KJob *job)
{
    mJobs--;
    auto rights = static_cast<KIMAP::MyRightsJob *>(job);
    if (job->error()) {
        qCDebug(KOLABRESOURCE_LOG) << "No rights for mailbox: " << rights->mailBox();
        if (!isNamespaceFolder(rights->mailBox(), mSharedNamespace)) {
            qCWarning(KOLABRESOURCE_LOG) << "MyRights for mailbox" << rights->mailBox() << "failed:" << job->errorString();
            // We ignore the error to avoid failing the complete sync. We can run into this when trying to retrieve rights for non-existing mailboxes.
        }
        checkDone();
        return;
    }

    const KIMAP::Acl::Rights imapRights = rights->rights();
    mRights.insert(rights->mailBox(), imapRights);
    checkDone();
}

void RetrieveMetadataJob::checkDone()
{
    if (!mJobs) {
        qCDebug(KOLABRESOURCE_TRACE) << "done";
        qCDebug(KOLABRESOURCE_LOG) << "done";
        emitResult();
    }
}

KolabRetrieveCollectionsTask::KolabRetrieveCollectionsTask(const ResourceStateInterface::Ptr &resource, QObject *parent)
    : ResourceTask(CancelIfNoSession, resource, parent)
    , cContentMimeTypes("CONTENTMIMETYPES")
    , cAccessRights("AccessRights")
    , cImapAcl("imapacl")
    , cCollectionAnnotations("collectionannotations")
    , cDefaultKeepLocalChanges(QSet<QByteArray>() << cContentMimeTypes << cAccessRights << cImapAcl << cCollectionAnnotations)
    , cDefaultMimeTypes(QStringList() << Akonadi::Collection::mimeType() << QStringLiteral("application/x-kolab-objects"))
    , cCollectionOnlyContentMimeTypes(QStringList() << Akonadi::Collection::mimeType())
{
    mRequestedMetadata << "/shared/vendor/kolab/folder-type";
    mRequestedMetadata << "/private/vendor/kolab/folder-type";
    mRequestedMetadata << "/shared" KOLAB_COLOR_ANNOTATION << "/private" KOLAB_COLOR_ANNOTATION;
}

KolabRetrieveCollectionsTask::~KolabRetrieveCollectionsTask() = default;

void KolabRetrieveCollectionsTask::doStart(KIMAP::Session *session)
{
    qCDebug(KOLABRESOURCE_LOG) << "Starting collection retrieval";
    mTime.start();
    mSession = session;

    Akonadi::Collection root;
    root.setName(resourceName());
    root.setRemoteId(rootRemoteId());
    root.setContentMimeTypes(QStringList(Akonadi::Collection::mimeType()));
    root.setParentCollection(Akonadi::Collection::root());
    root.addAttribute(new NoSelectAttribute(true));
    root.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing)->setIconName(QStringLiteral("kolab"));

    Akonadi::CachePolicy policy;
    policy.setInheritFromParent(false);
    policy.setSyncOnDemand(true);

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

    mMailCollections.insert(QString(), root);

    qCDebug(KOLABRESOURCE_TRACE) << "subscription enabled: " << isSubscriptionEnabled();
    // jobs are serialized by the session
    if (isSubscriptionEnabled()) {
        auto fullListJob = new KIMAP::ListJob(session);
        fullListJob->setOption(KIMAP::ListJob::NoOption);
        fullListJob->setQueriedNamespaces(serverNamespaces());
        connect(fullListJob, &KIMAP::ListJob::mailBoxesReceived, this, &KolabRetrieveCollectionsTask::onFullMailBoxesReceived);
        connect(fullListJob, &KJob::result, this, &KolabRetrieveCollectionsTask::onFullMailBoxesReceiveDone);
        mJobs++;
        fullListJob->start();
    }

    auto listJob = new KIMAP::ListJob(session);
    listJob->setOption(KIMAP::ListJob::IncludeUnsubscribed);
    listJob->setQueriedNamespaces(serverNamespaces());
    connect(listJob, &KIMAP::ListJob::mailBoxesReceived, this, &KolabRetrieveCollectionsTask::onMailBoxesReceived);
    connect(listJob, &KJob::result, this, &KolabRetrieveCollectionsTask::onMailBoxesReceiveDone);
    mJobs++;
    listJob->start();
}

void KolabRetrieveCollectionsTask::onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags)
{
    const int nbDescriptors = descriptors.size();
    for (int i = 0; i < nbDescriptors; ++i) {
        const KIMAP::MailBoxDescriptor descriptor = descriptors[i];
        createCollection(descriptor.name, flags.at(i), !isSubscriptionEnabled() || mSubscribedMailboxes.contains(descriptor.name));
    }
    checkDone();
}

Akonadi::Collection KolabRetrieveCollectionsTask::getOrCreateParent(const QString &path)
{
    if (mMailCollections.contains(path)) {
        return mMailCollections.value(path);
    }
    // create a dummy collection
    const QString separator = separatorCharacter();
    const QStringList pathParts = path.split(separator);
    const QString pathPart = pathParts.last();
    Akonadi::Collection c;
    c.setName(pathPart);
    c.setRemoteId(separator + pathPart);
    const QStringList parentPath = pathParts.mid(0, pathParts.size() - 1);
    const Akonadi::Collection parentCollection = getOrCreateParent(parentPath.join(separator));
    c.setParentCollection(parentCollection);

    c.addAttribute(new NoSelectAttribute(true));
    c.setContentMimeTypes(QStringList() << Akonadi::Collection::mimeType());
    c.setRights(Akonadi::Collection::ReadOnly);
    c.setEnabled(false);
    setAttributes(c, pathParts, path);

    mMailCollections.insert(path, c);
    return c;
}

void KolabRetrieveCollectionsTask::setAttributes(Akonadi::Collection &c, const QStringList &pathParts, const QString &path)
{
    auto attr = c.attribute<Akonadi::CollectionIdentificationAttribute>(Akonadi::Collection::AddIfMissing);
    attr->setIdentifier(path.toLatin1());

    // If the folder is a other users folder block all alarms from default
    if (isNamespaceFolder(path, resourceState()->userNamespaces())) {
        auto attr = c.attribute<Akonadi::BlockAlarmsAttribute>(Akonadi::Collection::AddIfMissing);
        attr->blockEverything(true);
    }

    // If the folder is a other users top-level folder mark it accordingly
    if (pathParts.size() == 1 && isNamespaceFolder(path, resourceState()->userNamespaces())) {
        auto attr = c.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setDisplayName(i18n("Other Users"));
        attr->setIconName(QStringLiteral("x-mail-distribution-list"));
    }

    // Mark user folders for searching
    if (pathParts.size() >= 2 && isNamespaceFolder(path, resourceState()->userNamespaces())) {
        auto attr = c.attribute<Akonadi::CollectionIdentificationAttribute>(Akonadi::Collection::AddIfMissing);
        if (pathParts.size() == 2) {
            attr->setCollectionNamespace("usertoplevel");
        } else {
            attr->setCollectionNamespace("user");
        }
    }

    // If the folder is a shared folders top-level folder mark it accordingly
    if (pathParts.size() == 1 && isNamespaceFolder(path, resourceState()->sharedNamespaces())) {
        auto attr = c.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setDisplayName(i18n("Shared Folders"));
        attr->setIconName(QStringLiteral("x-mail-distribution-list"));
    }

    // Mark shared folders for searching
    if (pathParts.size() >= 2 && isNamespaceFolder(path, resourceState()->sharedNamespaces())) {
        auto attr = c.attribute<Akonadi::CollectionIdentificationAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setCollectionNamespace("shared");
    }
}

void KolabRetrieveCollectionsTask::createCollection(const QString &mailbox, const QList<QByteArray> &currentFlags, bool isSubscribed)
{
    const QString separator = separatorCharacter();
    Q_ASSERT(separator.size() == 1);
    const QString boxName = mailbox.endsWith(separator) ? mailbox.left(mailbox.size() - 1) : mailbox;
    const QStringList pathParts = boxName.split(separator);
    const QString pathPart = pathParts.last();

    Akonadi::Collection c;
    // If we had a dummy collection we need to replace it
    if (mMailCollections.contains(mailbox)) {
        c = mMailCollections.value(mailbox);
    }
    c.setName(pathPart);
    c.setRemoteId(separator + pathPart);
    const QStringList parentPath = pathParts.mid(0, pathParts.size() - 1);
    const Akonadi::Collection parentCollection = getOrCreateParent(parentPath.join(separator));
    c.setParentCollection(parentCollection);
    // TODO get from ResourceState, and add KMime::Message::mimeType() for the normal imap resource by default
    // We add a dummy mimetype, otherwise the itemsync doesn't even work (action is disabled and resourcebase aborts the operation)
    c.setContentMimeTypes(cDefaultMimeTypes);
    c.setKeepLocalChanges(cDefaultKeepLocalChanges);

    // assume LRS, until myrights is executed
    if (serverCapabilities().contains(QLatin1String("ACL"))) {
        c.setRights(Akonadi::Collection::ReadOnly);
    } else {
        c.setRights(Akonadi::Collection::AllRights);
    }

    setAttributes(c, pathParts, mailbox);

    // If the folder is the Inbox, make some special settings.
    if (pathParts.size() == 1 && pathPart.compare(QLatin1String("inbox"), Qt::CaseInsensitive) == 0) {
        auto attr = c.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
        attr->setDisplayName(i18n("Inbox"));
        attr->setIconName(QStringLiteral("mail-folder-inbox"));
        c.attribute<Akonadi::SpecialCollectionAttribute>(Akonadi::Collection::AddIfMissing)->setCollectionType("inbox");
        setIdleCollection(c);
    }

    // If this folder is a noselect folder, make some special settings.
    if (currentFlags.contains("\\noselect")) {
        c.addAttribute(new NoSelectAttribute(true));
        c.setContentMimeTypes(cCollectionOnlyContentMimeTypes);
        c.setRights(Akonadi::Collection::ReadOnly);
    } else {
        // remove the noselect attribute explicitly, in case we had set it before (eg. for non-subscribed non-leaf folders)
        c.removeAttribute<NoSelectAttribute>();
    }

    // If this folder is a noinferiors folder, it is not allowed to create subfolders inside.
    if (currentFlags.contains("\\noinferiors")) {
        // qCDebug(KOLABRESOURCE_LOG) << "Noinferiors: " << currentPath;
        c.addAttribute(new NoInferiorsAttribute(true));
        c.setRights(c.rights() & ~Akonadi::Collection::CanCreateCollection);
    }
    c.setEnabled(isSubscribed);

    // qCDebug(KOLABRESOURCE_LOG) << "creating collection " << mailbox << " with parent " << parentPath;
    mMailCollections.insert(mailbox, c);
}

void KolabRetrieveCollectionsTask::onMailBoxesReceiveDone(KJob *job)
{
    qCDebug(KOLABRESOURCE_LOG) << "All mailboxes received: " << mTime.elapsed();
    qCDebug(KOLABRESOURCE_LOG) << "in total: " << mMailCollections.size();
    mJobs--;
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << QStringLiteral("Failed to retrieve mailboxes: ") + job->errorString();
        cancelTask(i18n("Collection retrieval failed"));
    } else {
        QSet<QString> mailboxes;
        const QStringList lstKeys = mMailCollections.keys();
        for (const QString &mailbox : lstKeys) {
            if (!mailbox.isEmpty() && !isNamespaceFolder(mailbox, resourceState()->userNamespaces() + resourceState()->sharedNamespaces())) {
                mailboxes << mailbox;
            }
        }

        // Only request metadata for subscribed Other Users Folders
        const QStringList metadataMailboxes = mailboxes.unite(mSubscribedMailboxes).values();

        auto metadata = new RetrieveMetadataJob(mSession,
                                                metadataMailboxes,
                                                serverCapabilities(),
                                                mRequestedMetadata,
                                                separatorCharacter(),
                                                resourceState()->sharedNamespaces(),
                                                resourceState()->userNamespaces(),
                                                this);
        connect(metadata, &KJob::result, this, &KolabRetrieveCollectionsTask::onMetadataRetrieved);
        mJobs++;
        metadata->start();
    }
}

void KolabRetrieveCollectionsTask::applyRights(const QHash<QString, KIMAP::Acl::Rights> &rights)
{
    // qCDebug(KOLABRESOURCE_LOG) << rights;
    const QStringList lstKeys = rights.keys();
    for (const QString &mailbox : lstKeys) {
        if (mMailCollections.contains(mailbox)) {
            const KIMAP::Acl::Rights imapRights = rights.value(mailbox);
            QStringList parts = mailbox.split(separatorCharacter());
            parts.removeLast();
            QString parentMailbox = parts.join(separatorCharacter());

            KIMAP::Acl::Rights parentImapRights;
            // If the parent folder is not existing we can't rename
            if (!parentMailbox.isEmpty() && rights.contains(parentMailbox)) {
                parentImapRights = rights.value(parentMailbox);
            }
            // qCDebug(KOLABRESOURCE_LOG) << mailbox << parentMailbox << imapRights << parentImapRights;

            Akonadi::Collection &collection = mMailCollections[mailbox];
            CollectionMetadataHelper::applyRights(collection, imapRights, parentImapRights);

            // Store the mailbox ACLs
            auto aclAttribute = collection.attribute<Akonadi::ImapAclAttribute>(Akonadi::Collection::AddIfMissing);
            const KIMAP::Acl::Rights oldRights = aclAttribute->myRights();
            if (oldRights != imapRights) {
                aclAttribute->setMyRights(imapRights);
            }
        } else {
            qCWarning(KOLABRESOURCE_LOG) << "Can't find mailbox " << mailbox;
        }
    }
}

void KolabRetrieveCollectionsTask::applyMetadata(const QHash<QString, QMap<QByteArray, QByteArray>> &metadataMap)
{
    // qCDebug(KOLABRESOURCE_LOG) << metadataMap;
    const auto keys{metadataMap.keys()};
    for (const QString &mailbox : keys) {
        const QMap<QByteArray, QByteArray> metadata = metadataMap.value(mailbox);
        if (mMailCollections.contains(mailbox)) {
            Akonadi::Collection &collection = mMailCollections[mailbox];
            // qCDebug(KOLABRESOURCE_LOG) << mailbox << metadata << type << folderType << KolabHelpers::getContentMimeTypes(folderType);
            collection.attribute<Akonadi::CollectionAnnotationsAttribute>(Akonadi::Collection::AddIfMissing)->setAnnotations(metadata);
            const QByteArray type = KolabHelpers::getFolderTypeAnnotation(metadata);
            const Kolab::FolderType folderType = KolabHelpers::folderTypeFromString(type);
            collection.setContentMimeTypes(KolabHelpers::getContentMimeTypes(folderType));
            const QColor color = KolabHelpers::getFolderColor(metadata);
            if (color.isValid()) {
                collection.attribute<Akonadi::CollectionColorAttribute>(Akonadi::Collection::AddIfMissing)->setColor(color);
            }
            QSet<QByteArray> keepLocalChanges = collection.keepLocalChanges();
            keepLocalChanges.remove(cContentMimeTypes);
            collection.setKeepLocalChanges(keepLocalChanges);
        }
    }
}

void KolabRetrieveCollectionsTask::onMetadataRetrieved(KJob *job)
{
    qCDebug(KOLABRESOURCE_LOG) << mTime.elapsed();
    mJobs--;
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << "Error while retrieving metadata, aborting collection retrieval: " << job->errorString();
        cancelTask(i18n("Collection retrieval failed"));
    } else {
        auto metadata = static_cast<RetrieveMetadataJob *>(job);
        applyRights(metadata->mRights);
        applyMetadata(metadata->mMetadata);
        checkDone();
    }
}

void KolabRetrieveCollectionsTask::checkDone()
{
    if (!mJobs) {
        collectionsRetrieved(Akonadi::valuesToVector(mMailCollections));
        qCDebug(KOLABRESOURCE_LOG) << "done " << mTime.elapsed();
    }
}

void KolabRetrieveCollectionsTask::onFullMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags)
{
    Q_UNUSED(flags)
    for (const KIMAP::MailBoxDescriptor &descriptor : descriptors) {
        mSubscribedMailboxes.insert(descriptor.name);
    }
}

void KolabRetrieveCollectionsTask::onFullMailBoxesReceiveDone(KJob *job)
{
    qCDebug(KOLABRESOURCE_LOG) << "received subscribed collections " << mTime.elapsed();
    mJobs--;
    if (job->error()) {
        qCWarning(KOLABRESOURCE_LOG) << QStringLiteral("Failed to retrieve subscribed collections: ") + job->errorString();
        cancelTask(i18n("Collection retrieval failed"));
    } else {
        checkDone();
    }
}
