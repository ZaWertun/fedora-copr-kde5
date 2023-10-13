/*
    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "openxchangeresource.h"

#include "settingsadaptor.h"

#include <Akonadi/CachePolicy>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/VectorHelper>

#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>
#include <KContacts/Addressee>
#include <KLocalizedString>

#include <QStandardPaths>
#include <oxa/davmanager.h>
#include <oxa/foldercreatejob.h>
#include <oxa/folderdeletejob.h>
#include <oxa/foldermodifyjob.h>
#include <oxa/foldermovejob.h>
#include <oxa/foldersrequestdeltajob.h>
#include <oxa/foldersrequestjob.h>
#include <oxa/objectcreatejob.h>
#include <oxa/objectdeletejob.h>
#include <oxa/objectmodifyjob.h>
#include <oxa/objectmovejob.h>
#include <oxa/objectrequestjob.h>
#include <oxa/objectsrequestdeltajob.h>
#include <oxa/objectsrequestjob.h>
#include <oxa/oxerrors.h>
#include <oxa/updateusersjob.h>
#include <oxa/users.h>

using namespace Akonadi;

class RemoteInformation
{
public:
    RemoteInformation(qlonglong objectId, OXA::Folder::Module module, const QString &lastModified)
        : mObjectId(objectId)
        , mModule(module)
        , mLastModified(lastModified)
    {
    }

    Q_REQUIRED_RESULT inline qlonglong objectId() const
    {
        return mObjectId;
    }

    Q_REQUIRED_RESULT inline OXA::Folder::Module module() const
    {
        return mModule;
    }

    Q_REQUIRED_RESULT inline QString lastModified() const
    {
        return mLastModified;
    }

    inline void setLastModified(const QString &lastModified)
    {
        mLastModified = lastModified;
    }

    static RemoteInformation load(const Item &item)
    {
        return loadImpl(item);
    }

    static RemoteInformation load(const Collection &collection)
    {
        return loadImpl(collection);
    }

    void store(Item &item) const
    {
        storeImpl(item);
    }

    void store(Collection &collection) const
    {
        storeImpl(collection);
    }

private:
    template<typename T>
    static inline RemoteInformation loadImpl(const T &entity)
    {
        const QStringList parts = entity.remoteRevision().split(QLatin1Char(':'), Qt::KeepEmptyParts);

        OXA::Folder::Module module = OXA::Folder::Unbound;

        if (!parts.isEmpty()) {
            if (parts.at(0) == QLatin1String("calendar")) {
                module = OXA::Folder::Calendar;
            } else if (parts.at(0) == QLatin1String("contacts")) {
                module = OXA::Folder::Contacts;
            } else if (parts.at(0) == QLatin1String("tasks")) {
                module = OXA::Folder::Tasks;
            } else {
                module = OXA::Folder::Unbound;
            }
        }

        QString lastModified = QStringLiteral("0");
        if (parts.count() > 1) {
            lastModified = parts.at(1);
        }

        return RemoteInformation(entity.remoteId().toLongLong(), module, lastModified);
    }

    template<typename T>
    inline void storeImpl(T &entity) const
    {
        QString module;
        switch (mModule) {
        case OXA::Folder::Calendar:
            module = QStringLiteral("calendar");
            break;
        case OXA::Folder::Contacts:
            module = QStringLiteral("contacts");
            break;
        case OXA::Folder::Tasks:
            module = QStringLiteral("tasks");
            break;
        case OXA::Folder::Unbound:
            break;
        }

        QStringList parts;
        parts.append(module);
        parts.append(mLastModified);

        entity.setRemoteId(QString::number(mObjectId));
        entity.setRemoteRevision(parts.join(QLatin1Char(':')));
    }

    qlonglong mObjectId;
    OXA::Folder::Module mModule;
    QString mLastModified;
};

class ObjectsLastSync
{
public:
    ObjectsLastSync()
    {
        if (!Settings::self()->objectsLastSync().isEmpty()) {
            const QStringList pairs = Settings::self()->objectsLastSync().split(QLatin1Char(':'), Qt::KeepEmptyParts);
            for (const QString &pair : pairs) {
                const QStringList entry = pair.split(QLatin1Char('='), Qt::KeepEmptyParts);
                mObjectsMap.insert(entry.at(0).toLongLong(), entry.at(1).toULongLong());
            }
        }
    }

    void save()
    {
        QStringList pairs;
        pairs.reserve(mObjectsMap.count());

        QMapIterator<qlonglong, qulonglong> it(mObjectsMap);
        while (it.hasNext()) {
            it.next();
            pairs.append(QString::number(it.key()) + QLatin1Char('=') + QString::number(it.value()));
        }

        Settings::self()->setObjectsLastSync(pairs.join(QLatin1Char(':')));
        Settings::self()->save();
    }

    Q_REQUIRED_RESULT qulonglong lastSync(qlonglong collectionId) const
    {
        return mObjectsMap.value(collectionId, 0);
    }

    void setLastSync(qlonglong collectionId, qulonglong timeStamp)
    {
        mObjectsMap.insert(collectionId, timeStamp);
    }

private:
    QMap<qlonglong, qulonglong> mObjectsMap;
};

static Collection::Rights folderPermissionsToCollectionRights(const OXA::Folder &folder)
{
    const OXA::Folder::UserPermissions userPermissions = folder.userPermissions();

    if (!userPermissions.contains(OXA::Users::self()->currentUserId())) {
        // There are no rights given for us explicitly, so it is read-only
        return Collection::ReadOnly;
    } else {
        const OXA::Folder::Permissions permissions = userPermissions.value(OXA::Users::self()->currentUserId());
        Collection::Rights rights = Collection::ReadOnly;
        switch (permissions.folderPermission()) {
        case OXA::Folder::Permissions::FolderIsVisible:
            rights |= Collection::ReadOnly;
            break;
        case OXA::Folder::Permissions::CreateObjects:
            rights |= Collection::CanCreateItem;
            break;
        case OXA::Folder::Permissions::CreateSubfolders: // fallthrough
        case OXA::Folder::Permissions::AdminPermission:
            rights |= (Collection::CanCreateItem | Collection::CanCreateCollection);
            break;
        default:
            break;
        }

        if (permissions.objectWritePermission() != OXA::Folder::Permissions::NoWritePermission) {
            rights |= Collection::CanChangeItem;
            rights |= Collection::CanChangeCollection;
        }

        if (permissions.objectDeletePermission() != OXA::Folder::Permissions::NoDeletePermission) {
            rights |= Collection::CanDeleteItem;
            rights |= Collection::CanDeleteCollection;
        }

        return rights;
    }
}

OpenXchangeResource::OpenXchangeResource(const QString &id)
    : ResourceBase(id)
{
    // setup the resource
    Settings::instance(KSharedConfig::openConfig());
    new SettingsAdaptor(Settings::self());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), Settings::self(), QDBusConnection::ExportAdaptors);
    mUseIncrementalUpdates = Settings::self()->useIncrementalUpdates();

    changeRecorder()->fetchCollection(true);
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::Parent);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(CollectionFetchScope::Parent);

    setName(i18n("Open-Xchange"));

    OXA::Users::self()->init(identifier());

    QUrl baseUrl = QUrl::fromLocalFile(Settings::self()->baseUrl());
    baseUrl.setUserName(Settings::self()->username());
    baseUrl.setPassword(Settings::self()->password());
    OXA::DavManager::self()->setBaseUrl(baseUrl);

    // Create the standard collections.
    //
    // There exists special OX folders (e.g. private, public, shared) that are not
    // returned by a normal webdav listing, therefore we create them manually here.
    // This is possible because the remote ids of these folders are fixed values from 1
    // till 4.
    mResourceCollection.setParentCollection(Collection::root());
    const RemoteInformation resourceInformation(0, OXA::Folder::Unbound, QString());
    resourceInformation.store(mResourceCollection);
    mResourceCollection.setName(name());
    mResourceCollection.setContentMimeTypes(QStringList() << Collection::mimeType());
    mResourceCollection.setRights(Collection::ReadOnly);
    auto attribute = mResourceCollection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attribute->setIconName(QStringLiteral("ox"));

    Collection privateFolder;
    privateFolder.setParentCollection(mResourceCollection);
    const RemoteInformation privateFolderInformation(1, OXA::Folder::Unbound, QString());
    privateFolderInformation.store(privateFolder);
    privateFolder.setName(i18n("Private Folder"));
    privateFolder.setContentMimeTypes(QStringList() << Collection::mimeType());
    privateFolder.setRights(Collection::ReadOnly);

    Collection publicFolder;
    publicFolder.setParentCollection(mResourceCollection);
    const RemoteInformation publicFolderInformation(2, OXA::Folder::Unbound, QString());
    publicFolderInformation.store(publicFolder);
    publicFolder.setName(i18n("Public Folder"));
    publicFolder.setContentMimeTypes(QStringList() << Collection::mimeType());
    publicFolder.setRights(Collection::ReadOnly);

    Collection sharedFolder;
    sharedFolder.setParentCollection(mResourceCollection);
    const RemoteInformation sharedFolderInformation(3, OXA::Folder::Unbound, QString());
    sharedFolderInformation.store(sharedFolder);
    sharedFolder.setName(i18n("Shared Folder"));
    sharedFolder.setContentMimeTypes(QStringList() << Collection::mimeType());
    sharedFolder.setRights(Collection::ReadOnly);

    Collection systemFolder;
    systemFolder.setParentCollection(mResourceCollection);
    const RemoteInformation systemFolderInformation(4, OXA::Folder::Unbound, QString());
    systemFolderInformation.store(systemFolder);
    systemFolder.setName(i18n("System Folder"));
    systemFolder.setContentMimeTypes(QStringList() << Collection::mimeType());
    systemFolder.setRights(Collection::ReadOnly);

    // TODO: set cache policy depending on sync behaviour
    Akonadi::CachePolicy cachePolicy;
    cachePolicy.setInheritFromParent(false);
    cachePolicy.setSyncOnDemand(false);
    cachePolicy.setCacheTimeout(-1);
    cachePolicy.setIntervalCheckTime(5);
    mResourceCollection.setCachePolicy(cachePolicy);

    mStandardCollectionsMap.insert(0, mResourceCollection);
    mStandardCollectionsMap.insert(1, privateFolder);
    mStandardCollectionsMap.insert(2, publicFolder);
    mStandardCollectionsMap.insert(3, sharedFolder);
    mStandardCollectionsMap.insert(4, systemFolder);

    mCollectionsMap = mStandardCollectionsMap;

    if (Settings::self()->useIncrementalUpdates()) {
        syncCollectionsRemoteIdCache();
    }

    connect(this, &Akonadi::AgentBase::reloadConfiguration, this, &OpenXchangeResource::onReloadConfiguration);
}

OpenXchangeResource::~OpenXchangeResource() = default;

void OpenXchangeResource::cleanup()
{
    // be nice and remove cache file when resource is removed
    QFile::remove(OXA::Users::self()->cacheFilePath());

    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + Settings::self()->config()->name());

    ResourceBase::cleanup();
}

void OpenXchangeResource::aboutToQuit()
{
}

void OpenXchangeResource::onReloadConfiguration()
{
    if (mUseIncrementalUpdates != Settings::self()->useIncrementalUpdates()) {
        mUseIncrementalUpdates = Settings::self()->useIncrementalUpdates();
        Settings::self()->setFoldersLastSync(0);
        Settings::self()->setObjectsLastSync(QString());
    }

    clearCache();

    QUrl baseUrl = QUrl::fromLocalFile(Settings::self()->baseUrl());
    baseUrl.setUserName(Settings::self()->username());
    baseUrl.setPassword(Settings::self()->password());
    OXA::DavManager::self()->setBaseUrl(baseUrl);

    // To find out the correct ACLs we need the uid of the user that
    // logs in. For loading events and tasks we need a complete mapping of
    // user id to name as well, so the mapping must be loaded as well.
    // Both is done by UpdateUsersJob, so trigger it here before we continue
    // with synchronization in onUpdateUsersJobFinished.
    auto job = new OXA::UpdateUsersJob(this);
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onUpdateUsersJobFinished);
    job->start();
}

void OpenXchangeResource::retrieveCollections()
{
    // qDebug("tokoe: retrieve collections called");
    if (Settings::self()->useIncrementalUpdates()) {
        // qDebug( "lastSync=%llu", Settings::self()->foldersLastSync() );
        auto job = new OXA::FoldersRequestDeltaJob(Settings::self()->foldersLastSync(), this);
        connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFoldersRequestDeltaJobFinished);
        job->start();
    } else {
        auto job = new OXA::FoldersRequestJob(0, OXA::FoldersRequestJob::Modified, this);
        connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFoldersRequestJobFinished);
        job->start();
    }
}

void OpenXchangeResource::retrieveItems(const Akonadi::Collection &collection)
{
    // qDebug( "tokoe: retrieveItems on %s called", qPrintable( collection.name() ) );
    const RemoteInformation remoteInformation = RemoteInformation::load(collection);

    OXA::Folder folder;
    folder.setObjectId(remoteInformation.objectId());
    folder.setModule(remoteInformation.module());

    if (Settings::self()->useIncrementalUpdates()) {
        ObjectsLastSync lastSyncInfo;
        // qDebug( "lastSync=%llu", lastSyncInfo.lastSync( collection.id() ) );
        auto job = new OXA::ObjectsRequestDeltaJob(folder, lastSyncInfo.lastSync(collection.id()), this);
        job->setProperty("collection", QVariant::fromValue(collection));
        connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectsRequestDeltaJobFinished);
        job->start();
    } else {
        auto job = new OXA::ObjectsRequestJob(folder, 0, OXA::ObjectsRequestJob::Modified, this);
        connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectsRequestJobFinished);
        job->start();
    }
}

bool OpenXchangeResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    // qDebug( "tokoe: retrieveItem %lld called", item.id() );
    const RemoteInformation remoteInformation = RemoteInformation::load(item);

    OXA::Object object;
    object.setObjectId(remoteInformation.objectId());
    object.setModule(remoteInformation.module());

    auto job = new OXA::ObjectRequestJob(object, this);
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectRequestJobFinished);
    job->start();

    return true;
}

void OpenXchangeResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(collection);

    OXA::Object object;
    object.setFolderId(remoteInformation.objectId());
    object.setModule(remoteInformation.module());

    if (item.hasPayload<KContacts::Addressee>()) {
        object.setContact(item.payload<KContacts::Addressee>());
    } else if (item.hasPayload<KContacts::ContactGroup>()) {
        object.setContactGroup(item.payload<KContacts::ContactGroup>());
    } else if (item.hasPayload<KCalendarCore::Event::Ptr>()) {
        object.setEvent(item.payload<KCalendarCore::Incidence::Ptr>());
    } else if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
        object.setTask(item.payload<KCalendarCore::Incidence::Ptr>());
    } else {
        Q_ASSERT(false);
    }

    auto job = new OXA::ObjectCreateJob(object, this);
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectCreateJobFinished);
    job->start();
}

void OpenXchangeResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(item);
    const RemoteInformation parentRemoteInformation = RemoteInformation::load(item.parentCollection());

    OXA::Object object;
    object.setObjectId(remoteInformation.objectId());
    object.setModule(remoteInformation.module());
    object.setFolderId(parentRemoteInformation.objectId());
    object.setLastModified(remoteInformation.lastModified());

    if (item.hasPayload<KContacts::Addressee>()) {
        object.setContact(item.payload<KContacts::Addressee>());
    } else if (item.hasPayload<KContacts::ContactGroup>()) {
        object.setContactGroup(item.payload<KContacts::ContactGroup>());
    } else if (item.hasPayload<KCalendarCore::Event::Ptr>()) {
        object.setEvent(item.payload<KCalendarCore::Incidence::Ptr>());
    } else if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
        object.setTask(item.payload<KCalendarCore::Incidence::Ptr>());
    } else {
        Q_ASSERT(false);
    }

    auto job = new OXA::ObjectModifyJob(object, this);
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectModifyJobFinished);
    job->start();
}

void OpenXchangeResource::itemRemoved(const Akonadi::Item &item)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(item);
    const RemoteInformation parentRemoteInformation = RemoteInformation::load(item.parentCollection());

    OXA::Object object;
    object.setObjectId(remoteInformation.objectId());
    object.setFolderId(parentRemoteInformation.objectId());
    object.setModule(remoteInformation.module());
    object.setLastModified(remoteInformation.lastModified());

    auto job = new OXA::ObjectDeleteJob(object, this);
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectDeleteJobFinished);

    job->start();
}

void OpenXchangeResource::itemMoved(const Akonadi::Item &item, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(item);
    const RemoteInformation parentRemoteInformation = RemoteInformation::load(collectionSource);
    const RemoteInformation newParentRemoteInformation = RemoteInformation::load(collectionDestination);

    OXA::Object object;
    object.setObjectId(remoteInformation.objectId());
    object.setModule(remoteInformation.module());
    object.setFolderId(parentRemoteInformation.objectId());
    object.setLastModified(remoteInformation.lastModified());

    OXA::Folder destinationFolder;
    destinationFolder.setObjectId(newParentRemoteInformation.objectId());

    auto job = new OXA::ObjectMoveJob(object, destinationFolder, this);
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onObjectMoveJobFinished);

    job->start();
}

void OpenXchangeResource::collectionAdded(const Akonadi::Collection &collection, const Akonadi::Collection &parent)
{
    const RemoteInformation parentRemoteInformation = RemoteInformation::load(parent);

    OXA::Folder folder;
    folder.setTitle(collection.name());
    folder.setFolderId(parentRemoteInformation.objectId());
    folder.setType(OXA::Folder::Private);

    // the folder 'inherits' the module type of its parent collection
    folder.setModule(parentRemoteInformation.module());

    // fill permissions
    OXA::Folder::Permissions permissions;
    permissions.setFolderPermission(OXA::Folder::Permissions::CreateSubfolders);
    permissions.setObjectReadPermission(OXA::Folder::Permissions::ReadOwnObjects);
    permissions.setObjectWritePermission(OXA::Folder::Permissions::WriteOwnObjects);
    permissions.setObjectDeletePermission(OXA::Folder::Permissions::DeleteOwnObjects);
    permissions.setAdminFlag(true);

    // assign permissions to user
    OXA::Folder::UserPermissions userPermissions;
    userPermissions.insert(OXA::Users::self()->currentUserId(), permissions);

    // set user permissions of folder
    folder.setUserPermissions(userPermissions);

    auto job = new OXA::FolderCreateJob(folder, this);
    job->setProperty("collection", QVariant::fromValue(collection));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFolderCreateJobFinished);
    job->start();
}

void OpenXchangeResource::collectionChanged(const Akonadi::Collection &collection)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(collection);

    // do not try to change the standard collections
    if (remoteInformation.objectId() >= 0 && remoteInformation.objectId() <= 4) {
        changeCommitted(collection);
        return;
    }

    const RemoteInformation parentRemoteInformation = RemoteInformation::load(collection.parentCollection());

    OXA::Folder folder;
    folder.setObjectId(remoteInformation.objectId());
    folder.setFolderId(parentRemoteInformation.objectId());
    folder.setTitle(collection.name());
    folder.setLastModified(remoteInformation.lastModified());

    auto job = new OXA::FolderModifyJob(folder, this);
    job->setProperty("collection", QVariant::fromValue(collection));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFolderModifyJobFinished);
}

void OpenXchangeResource::collectionRemoved(const Akonadi::Collection &collection)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(collection);

    OXA::Folder folder;
    folder.setObjectId(remoteInformation.objectId());
    folder.setLastModified(remoteInformation.lastModified());

    auto job = new OXA::FolderDeleteJob(folder, this);
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFolderDeleteJobFinished);

    job->start();
}

void OpenXchangeResource::collectionMoved(const Akonadi::Collection &collection,
                                          const Akonadi::Collection &collectionSource,
                                          const Akonadi::Collection &collectionDestination)
{
    const RemoteInformation remoteInformation = RemoteInformation::load(collection);
    const RemoteInformation parentRemoteInformation = RemoteInformation::load(collectionSource);
    const RemoteInformation newParentRemoteInformation = RemoteInformation::load(collectionDestination);

    OXA::Folder folder;
    folder.setObjectId(remoteInformation.objectId());
    folder.setFolderId(parentRemoteInformation.objectId());

    OXA::Folder destinationFolder;
    destinationFolder.setObjectId(newParentRemoteInformation.objectId());

    auto job = new OXA::FolderMoveJob(folder, destinationFolder, this);
    job->setProperty("collection", QVariant::fromValue(collection));
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFolderMoveJobFinished);

    job->start();
}

//// job result slots

void OpenXchangeResource::onUpdateUsersJobFinished(KJob *job)
{
    if (job->error()) {
        // This might be an indication that we can not connect to the server...
        Q_EMIT status(Broken, i18n("Unable to connect to server"));
        return;
    }

    if (Settings::self()->useIncrementalUpdates()) {
        syncCollectionsRemoteIdCache();
    }

    // now we have all user information, so continue synchronization
    synchronize();
    Q_EMIT configurationDialogAccepted();
}

void OpenXchangeResource::onObjectsRequestJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto requestJob = qobject_cast<OXA::ObjectsRequestJob *>(job);
    Q_ASSERT(requestJob);

    Item::List items;

    const OXA::Object::List objects = requestJob->objects();
    for (const OXA::Object &object : objects) {
        Item item;
        switch (object.module()) {
        case OXA::Folder::Contacts:
            if (!object.contact().isEmpty()) {
                item.setMimeType(KContacts::Addressee::mimeType());
                item.setPayload<KContacts::Addressee>(object.contact());
            } else {
                item.setMimeType(KContacts::ContactGroup::mimeType());
                item.setPayload<KContacts::ContactGroup>(object.contactGroup());
            }
            break;
        case OXA::Folder::Calendar:
            item.setMimeType(KCalendarCore::Event::eventMimeType());
            item.setPayload<KCalendarCore::Incidence::Ptr>(object.event());
            break;
        case OXA::Folder::Tasks:
            item.setMimeType(KCalendarCore::Todo::todoMimeType());
            item.setPayload<KCalendarCore::Incidence::Ptr>(object.task());
            break;
        case OXA::Folder::Unbound:
            Q_ASSERT(false);
            break;
        }
        const RemoteInformation remoteInformation(object.objectId(), object.module(), object.lastModified());
        remoteInformation.store(item);

        items.append(item);
    }

    itemsRetrieved(items);
}

void OpenXchangeResource::onObjectsRequestDeltaJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto requestJob = qobject_cast<OXA::ObjectsRequestDeltaJob *>(job);
    Q_ASSERT(requestJob);

    const auto collection = requestJob->property("collection").value<Collection>();

    ObjectsLastSync lastSyncInfo;

    qulonglong objectsLastSync = lastSyncInfo.lastSync(collection.id());

    Item::List changedItems;

    const OXA::Object::List modifiedObjects = requestJob->modifiedObjects();
    for (const OXA::Object &object : modifiedObjects) {
        Item item;
        switch (object.module()) {
        case OXA::Folder::Contacts:
            if (!object.contact().isEmpty()) {
                item.setMimeType(KContacts::Addressee::mimeType());
                item.setPayload<KContacts::Addressee>(object.contact());
            } else {
                item.setMimeType(KContacts::ContactGroup::mimeType());
                item.setPayload<KContacts::ContactGroup>(object.contactGroup());
            }
            break;
        case OXA::Folder::Calendar:
            item.setMimeType(KCalendarCore::Event::eventMimeType());
            item.setPayload<KCalendarCore::Incidence::Ptr>(object.event());
            break;
        case OXA::Folder::Tasks:
            item.setMimeType(KCalendarCore::Todo::todoMimeType());
            item.setPayload<KCalendarCore::Incidence::Ptr>(object.task());
            break;
        case OXA::Folder::Unbound:
            Q_ASSERT(false);
            break;
        }
        const RemoteInformation remoteInformation(object.objectId(), object.module(), object.lastModified());
        remoteInformation.store(item);

        // the value of objectsLastSync is determined by the maximum last modified value
        // of the added or changed objects
        objectsLastSync = qMax(objectsLastSync, object.lastModified().toULongLong());

        changedItems.append(item);
    }

    Item::List removedItems;

    const OXA::Object::List deletedObjects = requestJob->deletedObjects();
    removedItems.reserve(deletedObjects.count());
    for (const OXA::Object &object : deletedObjects) {
        Item item;

        const RemoteInformation remoteInformation(object.objectId(), object.module(), object.lastModified());
        remoteInformation.store(item);

        removedItems.append(item);
    }

    if (objectsLastSync != lastSyncInfo.lastSync(collection.id())) {
        // according to the OX developers we should subtract one millisecond from the
        // maximum last modified value to cover multiple changes that might have been
        // done in the same millisecond to the data on the server
        lastSyncInfo.setLastSync(collection.id(), objectsLastSync - 1);
        lastSyncInfo.save();
    }

    // qDebug( "changedObjects=%d removedObjects=%d", modifiedObjects.count(), deletedObjects.count() );
    // qDebug( "changedItems=%d removedItems=%d", changedItems.count(), removedItems.count() );
    itemsRetrievedIncremental(changedItems, removedItems);
}

void OpenXchangeResource::onObjectRequestJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto requestJob = qobject_cast<OXA::ObjectRequestJob *>(job);
    Q_ASSERT(requestJob);

    const OXA::Object object = requestJob->object();

    Item item = job->property("item").value<Item>();

    const RemoteInformation remoteInformation(object.objectId(), object.module(), object.lastModified());
    remoteInformation.store(item);

    switch (object.module()) {
    case OXA::Folder::Contacts:
        if (!object.contact().isEmpty()) {
            item.setMimeType(KContacts::Addressee::mimeType());
            item.setPayload<KContacts::Addressee>(object.contact());
        } else {
            item.setMimeType(KContacts::ContactGroup::mimeType());
            item.setPayload<KContacts::ContactGroup>(object.contactGroup());
        }
        break;
    case OXA::Folder::Calendar:
        item.setMimeType(KCalendarCore::Event::eventMimeType());
        item.setPayload<KCalendarCore::Incidence::Ptr>(object.event());
        break;
    case OXA::Folder::Tasks:
        item.setMimeType(KCalendarCore::Todo::todoMimeType());
        item.setPayload<KCalendarCore::Incidence::Ptr>(object.task());
        break;
    case OXA::Folder::Unbound:
        Q_ASSERT(false);
        break;
    }

    itemRetrieved(item);
}

void OpenXchangeResource::onObjectCreateJobFinished(KJob *job)
{
    if (job->error()) {
        QString errorText = job->errorText();
        if (job->error() == KJob::UserDefinedError) {
            switch (OXA::OXErrors::getEditErrorID(job->errorText())) {
            case OXA::OXErrors::ConcurrentModification:
                errorText = i18n("The object was edited by another participant in the meantime. Please check.");
                break;
            case OXA::OXErrors::ObjectNotFound:
                errorText = i18n("Object not found. Maybe it was deleted by another participant in the meantime.");
                break;
            case OXA::OXErrors::NoPermissionForThisAction:
                errorText = i18n("You don't have the permission to perform this action on this object.");
                break;
            case OXA::OXErrors::ConflictsDetected:
                errorText = i18n("A conflict detected. Please check if there are other objects in conflict with this one.");
                break;
            case OXA::OXErrors::MissingMandatoryFields:
                errorText = i18n("A mandatory data field is missing. Please check. Otherwise contact your administrator.");
                break;
            case OXA::OXErrors::AppointmentConflicts:
                errorText = i18n("An appointment conflict detected.\nPlease check if there are other appointments in conflict with this one.");
                break;
            case OXA::OXErrors::InternalServerError:
                errorText = i18n("Internal server error. Please contact your administrator.");
                break;
            case OXA::OXErrors::EditErrorUndefined:
            default:;
            }
        }
        cancelTask(errorText);
        return;
    }

    auto createJob = qobject_cast<OXA::ObjectCreateJob *>(job);
    Q_ASSERT(createJob);

    const OXA::Object object = createJob->object();

    Item item = job->property("item").value<Item>();

    const RemoteInformation remoteInformation(object.objectId(), object.module(), object.lastModified());
    remoteInformation.store(item);

    changeCommitted(item);
}

void OpenXchangeResource::onObjectModifyJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto modifyJob = qobject_cast<OXA::ObjectModifyJob *>(job);
    Q_ASSERT(modifyJob);

    const OXA::Object object = modifyJob->object();

    Item item = job->property("item").value<Item>();

    // update last_modified property
    RemoteInformation remoteInformation = RemoteInformation::load(item);
    remoteInformation.setLastModified(object.lastModified());
    remoteInformation.store(item);

    changeCommitted(item);
}

void OpenXchangeResource::onObjectMoveJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto moveJob = qobject_cast<OXA::ObjectMoveJob *>(job);
    Q_ASSERT(moveJob);

    const OXA::Object object = moveJob->object();

    Item item = job->property("item").value<Item>();

    // update last_modified property
    RemoteInformation remoteInformation = RemoteInformation::load(item);
    remoteInformation.setLastModified(object.lastModified());
    remoteInformation.store(item);

    changeCommitted(item);
}

void OpenXchangeResource::onObjectDeleteJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    changeProcessed();
}

static Collection folderToCollection(const OXA::Folder &folder, const Collection &parentCollection)
{
    Collection collection;

    collection.setParentCollection(parentCollection);

    const RemoteInformation remoteInformation(folder.objectId(), folder.module(), folder.lastModified());
    remoteInformation.store(collection);

    // set a unique name to make Akonadi happy
    collection.setName(folder.title() + QLatin1Char('_') + QUuid::createUuid().toString());

    auto attribute = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attribute->setDisplayName(folder.title());

    QStringList mimeTypes;
    mimeTypes.append(Collection::mimeType());
    switch (folder.module()) {
    case OXA::Folder::Calendar:
        mimeTypes.append(KCalendarCore::Event::eventMimeType());
        attribute->setIconName(QStringLiteral("view-calendar"));
        break;
    case OXA::Folder::Contacts:
        mimeTypes.append(KContacts::Addressee::mimeType());
        mimeTypes.append(KContacts::ContactGroup::mimeType());
        attribute->setIconName(QStringLiteral("view-pim-contacts"));
        break;
    case OXA::Folder::Tasks:
        mimeTypes.append(KCalendarCore::Todo::todoMimeType());
        attribute->setIconName(QStringLiteral("view-pim-tasks"));
        break;
    case OXA::Folder::Unbound:
        break;
    }

    collection.setContentMimeTypes(mimeTypes);
    collection.setRights(folderPermissionsToCollectionRights(folder));

    return collection;
}

void OpenXchangeResource::onFoldersRequestJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto requestJob = qobject_cast<OXA::FoldersRequestJob *>(job);
    Q_ASSERT(requestJob);

    Collection::List collections;

    // add the standard collections
    collections << Akonadi::valuesToVector(mStandardCollectionsMap);

    QMap<qlonglong, Collection> remoteIdMap(mStandardCollectionsMap);

    // add the folders from the server
    OXA::Folder::List folders = requestJob->folders();
    while (!folders.isEmpty()) {
        const OXA::Folder folder = folders.takeFirst();
        if (remoteIdMap.contains(folder.folderId())) {
            // we have the parent collection created already
            const Collection collection = folderToCollection(folder, remoteIdMap.value(folder.folderId()));
            remoteIdMap.insert(folder.objectId(), collection);
            collections.append(collection);
        } else {
            // we have to wait until the parent folder has been created
            folders.append(folder);
            qDebug() << "Error: parent folder id" << folder.folderId() << "of folder" << folder.title() << "is unknown";
        }
    }

    collectionsRetrieved(collections);
}

void OpenXchangeResource::onFoldersRequestDeltaJobFinished(KJob *job)
{
    // qDebug( "onFoldersRequestDeltaJobFinished mCollectionsMap.count() = %d", mCollectionsMap.count() );

    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto requestJob = qobject_cast<OXA::FoldersRequestDeltaJob *>(job);
    Q_ASSERT(requestJob);

    Collection::List changedCollections;

    // add the standard collections
    changedCollections << Akonadi::valuesToVector(mStandardCollectionsMap);

    qulonglong foldersLastSync = Settings::self()->foldersLastSync();

    // add the new or modified folders from the server
    OXA::Folder::List modifiedFolders = requestJob->modifiedFolders();
    while (!modifiedFolders.isEmpty()) {
        const OXA::Folder folder = modifiedFolders.takeFirst();
        if (mCollectionsMap.contains(folder.folderId())) {
            // we have the parent collection created already
            const Collection collection = folderToCollection(folder, mCollectionsMap.value(folder.folderId()));
            mCollectionsMap.insert(folder.objectId(), collection);
            changedCollections.append(collection);

            // the value of foldersLastSync is determined by the maximum last modified value
            // of the added or changed folders
            foldersLastSync = qMax(foldersLastSync, folder.lastModified().toULongLong());
        } else {
            // we have to wait until the parent folder has been created
            modifiedFolders.append(folder);
            qDebug() << "Error: parent folder id" << folder.folderId() << "of folder" << folder.title() << "is unknown";
        }
    }

    Collection::List removedCollections;

    // add the deleted folders from the server
    const OXA::Folder::List deletedFolders = requestJob->deletedFolders();
    removedCollections.reserve(deletedFolders.count());
    for (const OXA::Folder &folder : deletedFolders) {
        Collection collection;
        collection.setRemoteId(QString::number(folder.objectId()));

        removedCollections.append(collection);
    }

    if (foldersLastSync != Settings::self()->foldersLastSync()) {
        // according to the OX developers we should subtract one millisecond from the
        // maximum last modified value to cover multiple changes that might have been
        // done in the same millisecond to the data on the server
        Settings::self()->setFoldersLastSync(foldersLastSync - 1);
        Settings::self()->save();
    }

    // qDebug( "changedFolders=%d removedFolders=%d", modifiedFolders.count(), deletedFolders.count() );
    // qDebug( "changedCollections=%d removedCollections=%d", changedCollections.count(), removedCollections.count() );
    collectionsRetrievedIncremental(changedCollections, removedCollections);
}

void OpenXchangeResource::onFolderCreateJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto createJob = qobject_cast<OXA::FolderCreateJob *>(job);
    Q_ASSERT(createJob);

    const OXA::Folder folder = createJob->folder();

    auto collection = job->property("collection").value<Collection>();

    const RemoteInformation remoteInformation(folder.objectId(), folder.module(), folder.lastModified());
    remoteInformation.store(collection);

    // set matching icon
    auto attribute = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    switch (folder.module()) {
    case OXA::Folder::Calendar:
        attribute->setIconName(QStringLiteral("view-calendar"));
        break;
    case OXA::Folder::Contacts:
        attribute->setIconName(QStringLiteral("view-pim-contacts"));
        break;
    case OXA::Folder::Tasks:
        attribute->setIconName(QStringLiteral("view-pim-tasks"));
        break;
    case OXA::Folder::Unbound:
        break;
    }

    changeCommitted(collection);
}

void OpenXchangeResource::onFolderModifyJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto modifyJob = qobject_cast<OXA::FolderModifyJob *>(job);
    Q_ASSERT(modifyJob);

    const OXA::Folder folder = modifyJob->folder();

    auto collection = job->property("collection").value<Collection>();

    // update last_modified property
    RemoteInformation remoteInformation = RemoteInformation::load(collection);
    remoteInformation.setLastModified(folder.lastModified());
    remoteInformation.store(collection);

    changeCommitted(collection);
}

void OpenXchangeResource::onFolderMoveJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    auto moveJob = qobject_cast<OXA::FolderMoveJob *>(job);
    Q_ASSERT(moveJob);

    const OXA::Folder folder = moveJob->folder();

    auto collection = job->property("collection").value<Collection>();

    // update last_modified property
    RemoteInformation remoteInformation = RemoteInformation::load(collection);
    remoteInformation.setLastModified(folder.lastModified());
    remoteInformation.store(collection);

    changeCommitted(collection);
}

void OpenXchangeResource::onFolderDeleteJobFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(job->errorText());
        return;
    }

    changeProcessed();
}

/**
 * For incremental updates we need a mapping between the folder id
 * and the collection object for all collections of this resource,
 * so that we can find out the right parent collection in
 * onFoldersRequestDeltaJob().
 *
 * Therefore we trigger this method when the resource is started and
 * configured to use incremental sync.
 */
void OpenXchangeResource::syncCollectionsRemoteIdCache()
{
    mCollectionsMap.clear();

    // copy the standard collections
    mCollectionsMap = mStandardCollectionsMap;

    auto job = new CollectionFetchJob(mResourceCollection, CollectionFetchJob::Recursive, this);
    connect(job, &OXA::UpdateUsersJob::result, this, &OpenXchangeResource::onFetchResourceCollectionsFinished);
}

void OpenXchangeResource::onFetchResourceCollectionsFinished(KJob *job)
{
    if (job->error()) {
        qDebug() << "Error: Unable to fetch resource collections:" << job->errorText();
        return;
    }

    const CollectionFetchJob *fetchJob = qobject_cast<CollectionFetchJob *>(job);

    // copy the remaining collections of the resource
    const Collection::List collections = fetchJob->collections();
    for (const Collection &collection : collections) {
        mCollectionsMap.insert(collection.remoteId().toULongLong(), collection);
    }
}

AKONADI_RESOURCE_MAIN(OpenXchangeResource)
