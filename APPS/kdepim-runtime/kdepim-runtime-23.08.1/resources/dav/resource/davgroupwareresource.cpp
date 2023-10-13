/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "davgroupwareresource.h"

#include "akonadietagcache.h"
#include "configdialog.h"
#include "ctagattribute.h"
#include "davfreebusyhandler.h"
#include "davprotocolattribute.h"
#include "settings.h"
#include "settingsadaptor.h"
#include "setupwizard.h"
#include "utils.h"

#include <KDAV/DavCollection>
#include <KDAV/DavCollectionDeleteJob>
#include <KDAV/DavCollectionModifyJob>
#include <KDAV/DavCollectionsFetchJob>
#include <KDAV/DavCollectionsMultiFetchJob>
#include <KDAV/DavItem>
#include <KDAV/DavItemCreateJob>
#include <KDAV/DavItemDeleteJob>
#include <KDAV/DavItemFetchJob>
#include <KDAV/DavItemModifyJob>
#include <KDAV/DavItemsFetchJob>
#include <KDAV/DavItemsListJob>
#include <KDAV/ProtocolInfo>

#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Incidence>
#include <KCalendarCore/MemoryCalendar>
#include <KCalendarCore/Todo>
#include <KJob>

#include "davresource_debug.h"
#include <Akonadi/AttributeFactory>
#include <Akonadi/CachePolicy>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionColorAttribute>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/RecursiveItemFetchJob>
#include <KContacts/Addressee>
#include <KContacts/VCardConverter>

#include <KLocalizedString>
#include <kwindowsystem.h>

using namespace Akonadi;

using IncidencePtr = QSharedPointer<KCalendarCore::Incidence>;

DavGroupwareResource::DavGroupwareResource(const QString &id)
    : ResourceBase(id)
    , FreeBusyProviderBase()
    , mFreeBusyHandler(new DavFreeBusyHandler(this))
{
    AttributeFactory::registerAttribute<EntityDisplayAttribute>();
    AttributeFactory::registerAttribute<DavProtocolAttribute>();
    AttributeFactory::registerAttribute<CTagAttribute>();
    AttributeFactory::registerAttribute<CollectionColorAttribute>();

    setNeedsNetwork(true);

    mDavCollectionRoot.setParentCollection(Collection::root());
    mDavCollectionRoot.setName(identifier());
    mDavCollectionRoot.setRemoteId(identifier());
    mDavCollectionRoot.setContentMimeTypes(QStringList() << Collection::mimeType());
    mDavCollectionRoot.setRights(Collection::CanCreateCollection | Collection::CanDeleteCollection | Collection::CanChangeCollection);

    auto attribute = mDavCollectionRoot.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attribute->setIconName(QStringLiteral("folder-remote"));

    int refreshInterval = Settings::self()->refreshInterval();
    if (refreshInterval == 0) {
        refreshInterval = -1;
    }

    Akonadi::CachePolicy cachePolicy;
    cachePolicy.setInheritFromParent(false);
    cachePolicy.setSyncOnDemand(false);
    cachePolicy.setCacheTimeout(-1);
    cachePolicy.setIntervalCheckTime(refreshInterval);
    cachePolicy.setLocalParts(QStringList() << QStringLiteral("ALL"));
    mDavCollectionRoot.setCachePolicy(cachePolicy);

    changeRecorder()->fetchCollection(true);
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    changeRecorder()->itemFetchScope().fetchFullPayload(true);
    changeRecorder()->itemFetchScope().setAncestorRetrieval(ItemFetchScope::All);

    Settings::self()->setWinId(winIdForDialogs());
    Settings::self()->setResourceIdentifier(identifier());

    connect(mFreeBusyHandler, &DavFreeBusyHandler::handlesFreeBusy, this, &DavGroupwareResource::onHandlesFreeBusy);
    connect(mFreeBusyHandler, &DavFreeBusyHandler::freeBusyRetrieved, this, &DavGroupwareResource::onFreeBusyRetrieved);

    connect(this, &DavGroupwareResource::reloadConfiguration, this, &DavGroupwareResource::onReloadConfig);

    scheduleCustomTask(this, "initialRetrieveCollections", QVariant(), ResourceBase::Prepend);
    scheduleCustomTask(this, "createInitialCache", QVariant(), ResourceBase::Prepend);
}

DavGroupwareResource::~DavGroupwareResource()
{
    delete mFreeBusyHandler;
}

void DavGroupwareResource::collectionRemoved(const Akonadi::Collection &collection)
{
    qCDebug(DAVRESOURCE_LOG) << "Removing collection " << collection.remoteId();

    if (!configurationIsValid()) {
        return;
    }

    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId());

    auto job = new KDAV::DavCollectionDeleteJob(davUrl);
    job->setProperty("collection", QVariant::fromValue(collection));
    connect(job, &KDAV::DavCollectionDeleteJob::result, this, &DavGroupwareResource::onCollectionRemovedFinished);
    job->start();
}

void DavGroupwareResource::cleanup()
{
    Settings::self()->cleanup();
    ResourceBase::cleanup();
}

QDateTime DavGroupwareResource::lastCacheUpdate() const
{
    return QDateTime::currentDateTime();
}

void DavGroupwareResource::canHandleFreeBusy(const QString &email) const
{
    if (!isOnline()) {
        handlesFreeBusy(email, false);
    } else {
        mFreeBusyHandler->canHandleFreeBusy(email);
    }
}

void DavGroupwareResource::onHandlesFreeBusy(const QString &email, bool handles)
{
    handlesFreeBusy(email, handles);
}

void DavGroupwareResource::retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end)
{
    if (!isOnline()) {
        freeBusyRetrieved(email, QString(), false, i18n("Unable to retrieve free-busy info while offline"));
    } else {
        mFreeBusyHandler->retrieveFreeBusy(email, start, end);
    }
}

void DavGroupwareResource::onFreeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText)
{
    freeBusyRetrieved(email, freeBusy, success, errorText);
}

void DavGroupwareResource::configure(WId windowId)
{
    Settings::self()->setWinId(windowId);

    // On the initial configuration we start the setup wizard
    if (Settings::self()->configuredDavUrls().isEmpty()) {
        SetupWizard wizard;

        if (windowId) {
            wizard.setAttribute(Qt::WA_NativeWindow, true);
            KWindowSystem::setMainWindow(wizard.windowHandle(), windowId);
        }

        const int result = wizard.exec();
        if (result == QDialog::Accepted) {
            const SetupWizard::Url::List urls = wizard.urls();
            for (const SetupWizard::Url &url : urls) {
                auto urlConfig = new Settings::UrlConfiguration();

                urlConfig->mUrl = url.url;
                urlConfig->mProtocol = url.protocol;
                urlConfig->mUser = url.userName;
                urlConfig->mPassword = wizard.field(QStringLiteral("credentialsPassword")).toString();

                Settings::self()->newUrlConfiguration(urlConfig);
            }

            if (!urls.isEmpty()) {
                Settings::self()->setDisplayName(wizard.displayName());
            }

            QString defaultUser = wizard.field(QStringLiteral("credentialsUserName")).toString();
            if (!defaultUser.isEmpty()) {
                Settings::self()->setDefaultUsername(defaultUser);
                Settings::self()->setDefaultPassword(wizard.field(QStringLiteral("credentialsPassword")).toString());
            }
        }
    }

    // continue with the normal config dialog
    ConfigDialog dialog;

    if (windowId) {
        dialog.setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(dialog.windowHandle(), windowId);
    }

    if (!Settings::self()->defaultUsername().isEmpty()) {
        dialog.setPassword(Settings::self()->defaultPassword());
    }

    const int result = dialog.exec();

    if (result == QDialog::Accepted) {
        Settings::self()->setSettingsVersion(3);
        Settings::self()->save();
        synchronize();
        Q_EMIT configurationDialogAccepted();
    } else {
        Q_EMIT configurationDialogRejected();
    }
}

KJob *DavGroupwareResource::createRetrieveCollectionsJob()
{
    qCDebug(DAVRESOURCE_LOG) << "Retrieving collections list";
    mSyncErrorNotified = false;

    if (!configurationIsValid()) {
        return nullptr;
    }

    Q_EMIT status(Running, i18n("Fetching collections"));

    auto job = new KDAV::DavCollectionsMultiFetchJob(Settings::self()->configuredDavUrls());
    connect(job, &KDAV::DavCollectionsMultiFetchJob::result, this, &DavGroupwareResource::onRetrieveCollectionsFinished);
    connect(job, &KDAV::DavCollectionsMultiFetchJob::collectionDiscovered, this, &DavGroupwareResource::onCollectionDiscovered);
    return job;
}

void DavGroupwareResource::initialRetrieveCollections()
{
    auto job = createRetrieveCollectionsJob();
    if (!job) {
        return;
    }
    job->setProperty("initialCacheSync", QVariant::fromValue(true));
    job->start();
}

void DavGroupwareResource::retrieveCollections()
{
    auto job = createRetrieveCollectionsJob();
    if (!job) {
        return;
    }
    job->setProperty("initialCacheSync", QVariant::fromValue(false));
    job->start();
}

void DavGroupwareResource::retrieveItems(const Akonadi::Collection &collection)
{
    if (!collection.isValid()) {
        itemsRetrievalDone();
        return;
    }

    qCDebug(DAVRESOURCE_LOG) << "Retrieving items for collection " << collection.remoteId();

    if (!configurationIsValid()) {
        return;
    }

    // As the resource root collection contains mime types for items we must
    // work around the fact that Akonadi will rightfully try to retrieve items
    // from it. So just return an empty list
    if (collection.remoteId() == identifier()) {
        itemsRetrievalDone();
        return;
    }

    if (!mEtagCaches.contains(collection.remoteId())) {
        qCDebug(DAVRESOURCE_LOG) << "Asked to retrieve items for a collection we don't have in the cache";
        itemsRetrievalDone();
        return;
    }

    // Only continue if the collection has changed or if
    // it's the first time we see it
    const auto CTagAttr = collection.attribute<CTagAttribute>();
    if (CTagAttr && mCTagCache.contains(collection.remoteId()) && mCTagCache.value(collection.remoteId()) == CTagAttr->CTag()) {
        qCDebug(DAVRESOURCE_LOG) << "CTag for collection" << collection.remoteId() << "didn't change: " << CTagAttr->CTag();
        itemsRetrievalDone();
        return;
    }

    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId());

    if (!davUrl.url().isValid()) {
        qCCritical(DAVRESOURCE_LOG) << "Can't find a configured URL, collection.remoteId() is " << collection.remoteId();
        cancelTask(i18n("Asked to retrieve items for an unknown collection: %1", collection.remoteId()));
        // Q_ASSERT_X( false, "DavGroupwareResource::retrieveItems", "Url is invalid" );
        return;
    }

    auto job = new KDAV::DavItemsListJob(davUrl, mEtagCaches.value(collection.remoteId()));
    if (Settings::self()->limitSyncRange()) {
        QDateTime start = Settings::self()->getSyncRangeStart();
        qCDebug(DAVRESOURCE_LOG) << "Start time for list job:" << start;
        if (start.isValid()) {
            job->setTimeRange(start.toString(QStringLiteral("yyyyMMddTHHMMssZ")), QString());
        }
    }
    job->setProperty("collection", QVariant::fromValue(collection));
    job->setContentMimeTypes(collection.contentMimeTypes());
    connect(job, &KDAV::DavItemsListJob::result, this, &DavGroupwareResource::onRetrieveItemsFinished);
    job->start();
}

bool DavGroupwareResource::retrieveItem(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    qCDebug(DAVRESOURCE_LOG) << "Retrieving single item. Remote id = " << item.remoteId();

    if (!configurationIsValid()) {
        return false;
    }

    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), item.remoteId());
    if (!davUrl.url().isValid()) {
        qCDebug(DAVRESOURCE_LOG) << "Failed to get a valid DavUrl. Parent collection remote ID is" << item.parentCollection().remoteId();
        cancelTask();
        return false;
    }

    KDAV::DavItem davItem;
    davItem.setContentType(QStringLiteral("text/calendar"));
    davItem.setEtag(item.remoteRevision());

    auto job = new KDAV::DavItemFetchJob(davItem);
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &KDAV::DavItemFetchJob::result, this, &DavGroupwareResource::onRetrieveItemFinished);
    job->start();

    return true;
}

void DavGroupwareResource::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    qCDebug(DAVRESOURCE_LOG) << "Received notification for added item. Local id = " << item.id() << ". Remote id = " << item.remoteId()
                             << ". Collection remote id = " << collection.remoteId();

    if (!configurationIsValid()) {
        return;
    }

    if (collection.remoteId().isEmpty()) {
        qCCritical(DAVRESOURCE_LOG) << "Invalid remote id for collection " << collection.id() << " = " << collection.remoteId();
        cancelTask(i18n("Invalid collection for item %1.", item.id()));
        return;
    }

    KDAV::DavItem davItem = Utils::createDavItem(item, collection);
    if (davItem.data().isEmpty()) {
        qCCritical(DAVRESOURCE_LOG) << "Item " << item.id() << " doesn't has a valid payload";
        cancelTask();
        return;
    }

    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId(), davItem.url().toDisplayString());
    qCDebug(DAVRESOURCE_LOG) << "Item " << item.id() << " will be put to " << davItem.url().toDisplayString();
    davItem.setUrl(davUrl);

    auto job = new KDAV::DavItemCreateJob(davItem);
    job->setProperty("collection", QVariant::fromValue(collection));
    job->setProperty("item", QVariant::fromValue(item));
    connect(job, &KDAV::DavItemCreateJob::result, this, &DavGroupwareResource::onItemAddedFinished);
    job->start();
}

void DavGroupwareResource::itemChanged(const Akonadi::Item &item, const QSet<QByteArray> &)
{
    qCDebug(DAVRESOURCE_LOG) << "Received notification for changed item. Local id = " << item.id() << ". Remote id = " << item.remoteId();

    if (!configurationIsValid()) {
        return;
    }

    const Akonadi::Collection collection = item.parentCollection();
    if (!mEtagCaches.contains(collection.remoteId())) {
        qCDebug(DAVRESOURCE_LOG) << "Changed item is in a collection we don't have in the cache";
        // TODO: display an error
        cancelTask();
        return;
    }

    QString ridBase = item.remoteId();
    if (ridBase.contains(QLatin1Char('#'))) {
        ridBase.truncate(ridBase.indexOf(QLatin1Char('#')));
    }

    auto cache = mEtagCaches.value(collection.remoteId());
    Akonadi::Item::List extraItems;
    const QStringList lstUrls = cache->urls();
    for (const QString &rid : lstUrls) {
        if (rid.startsWith(ridBase) && rid != item.remoteId()) {
            Akonadi::Item extraItem;
            extraItem.setRemoteId(rid);
            extraItems << extraItem;
        }
    }

    if (extraItems.isEmpty()) {
        doItemChange(item);
    } else {
        auto job = new Akonadi::ItemFetchJob(extraItems);
        job->setCollection(item.parentCollection());
        job->fetchScope().fetchFullPayload();
        job->setProperty("item", QVariant::fromValue(item));
        connect(job, &Akonadi::ItemFetchJob::result, this, &DavGroupwareResource::onItemChangePrepared);
    }
}

void DavGroupwareResource::onItemChangePrepared(KJob *job)
{
    auto fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    auto item = job->property("item").value<Akonadi::Item>();
    doItemChange(item, fetchJob->items());
}

void DavGroupwareResource::doItemChange(const Akonadi::Item &item, const Akonadi::Item::List &dependentItems)
{
    KDAV::DavItem davItem = Utils::createDavItem(item, item.parentCollection(), dependentItems);
    if (davItem.data().isEmpty()) {
        qCCritical(DAVRESOURCE_LOG) << "Item " << item.id() << " doesn't has a valid payload";
        cancelTask();
        return;
    }

    QString url = item.remoteId();
    if (url.contains(QLatin1Char('#'))) {
        url.truncate(url.indexOf(QLatin1Char('#')));
    }
    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), url);

    // We have to re-set the URL as it's not necessarily valid after createDavItem()
    davItem.setUrl(davUrl);
    davItem.setEtag(item.remoteRevision());

    auto modJob = new KDAV::DavItemModifyJob(davItem);
    modJob->setProperty("collection", QVariant::fromValue(item.parentCollection()));
    modJob->setProperty("item", QVariant::fromValue(item));
    modJob->setProperty("dependentItems", QVariant::fromValue(dependentItems));
    connect(modJob, &KDAV::DavItemModifyJob::result, this, &DavGroupwareResource::onItemChangedFinished);
    modJob->start();
}

void DavGroupwareResource::itemRemoved(const Akonadi::Item &item)
{
    qCDebug(DAVRESOURCE_LOG) << "Received notification for removed item. Remote id = " << item.remoteId();

    if (!configurationIsValid()) {
        return;
    }

    const Akonadi::Collection collection = item.parentCollection();
    if (!mEtagCaches.contains(collection.remoteId())) {
        qCDebug(DAVRESOURCE_LOG) << "Removed item is in a collection we don't have in the cache";
        // TODO: display an error
        cancelTask();
        return;
    }

    QString ridBase = item.remoteId();
    if (ridBase.contains(QLatin1Char('#'))) {
        // A bit tricky: we must remove an incidence contained in a resource
        // containing multiple ones.
        ridBase.truncate(ridBase.indexOf(QLatin1Char('#')));

        auto cache = mEtagCaches.value(collection.remoteId());
        Akonadi::Item::List extraItems;
        const QStringList lstUrl = cache->urls();
        for (const QString &rid : lstUrl) {
            if (rid.startsWith(ridBase) && rid != item.remoteId()) {
                Akonadi::Item extraItem;
                extraItem.setRemoteId(rid);
                extraItems << extraItem;
            }
        }

        if (extraItems.isEmpty()) {
            // Urrrr?
            // Well, just delete the item.
            doItemRemoval(item);
        } else {
            auto job = new Akonadi::ItemFetchJob(extraItems);
            job->setCollection(item.parentCollection());
            job->fetchScope().fetchFullPayload();
            job->setProperty("item", QVariant::fromValue(item));
            connect(job, &Akonadi::ItemFetchJob::result, this, &DavGroupwareResource::onItemRemovalPrepared);
        }
    } else {
        // easy as pie: just remove everything at the URL.
        doItemRemoval(item);
    }
}

void DavGroupwareResource::onItemRemovalPrepared(KJob *job)
{
    auto fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    auto item = job->property("item").value<Akonadi::Item>();
    const Akonadi::Item::List keptItems = fetchJob->items();

    if (keptItems.isEmpty()) {
        // Urrrr? Not again!
        doItemRemoval(item);
    } else {
        Akonadi::Item mainItem;
        Akonadi::Item::List extraItems;
        QString ridBase = item.remoteId();
        ridBase.truncate(ridBase.indexOf(QLatin1Char('#')));

        for (const Akonadi::Item &kept : keptItems) {
            if (kept.remoteId() == ridBase && extraItems.isEmpty()) {
                mainItem = kept;
            } else {
                extraItems << kept;
            }
        }

        if (!mainItem.hasPayload()) {
            mainItem = extraItems.takeFirst();
        }

        const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), ridBase);

        KDAV::DavItem davItem = Utils::createDavItem(mainItem, mainItem.parentCollection(), extraItems);
        davItem.setUrl(davUrl);
        davItem.setEtag(item.remoteRevision());

        auto modJob = new KDAV::DavItemModifyJob(davItem);
        modJob->setProperty("collection", QVariant::fromValue(mainItem.parentCollection()));
        modJob->setProperty("item", QVariant::fromValue(mainItem));
        modJob->setProperty("dependentItems", QVariant::fromValue(extraItems));
        modJob->setProperty("isRemoval", QVariant::fromValue(true));
        modJob->setProperty("removedItem", QVariant::fromValue(item));
        connect(modJob, &KDAV::DavItemModifyJob::result, this, &DavGroupwareResource::onItemChangedFinished);
        modJob->start();
    }
}

void DavGroupwareResource::doItemRemoval(const Akonadi::Item &item)
{
    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), item.remoteId());

    KDAV::DavItem davItem;
    davItem.setUrl(davUrl);
    davItem.setEtag(item.remoteRevision());

    auto job = new KDAV::DavItemDeleteJob(davItem);
    job->setProperty("item", QVariant::fromValue(item));
    job->setProperty("collection", QVariant::fromValue(item.parentCollection()));
    connect(job, &KDAV::DavItemDeleteJob::result, this, &DavGroupwareResource::onItemRemovedFinished);
    job->start();
}

void DavGroupwareResource::collectionChanged(const Collection &collection)
{
    qCDebug(DAVRESOURCE_LOG) << "Collection changed" << collection.remoteId();

    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId());

    QColor color;
    if (collection.hasAttribute<Akonadi::CollectionColorAttribute>()) {
        const auto colorAttr = collection.attribute<Akonadi::CollectionColorAttribute>();
        if (colorAttr) {
            color = colorAttr->color();
        }
    }

    auto job = new KDAV::DavCollectionModifyJob(davUrl);
    // TODO fix renaming calendars with parent folders, right now it makes a bit of a mess
    // job->setProperty(QStringLiteral("displayname"), collection.displayName());
    if (color.isValid()) {
        job->setProperty(QStringLiteral("calendar-color"), color.name(), QStringLiteral("http://apple.com/ns/ical/"));
    }
    connect(job, &KDAV::DavCollectionModifyJob::result, this, [=](KJob *job) {
        onCollectionChangedFinished(job, collection);
    });
    job->start();
}

void DavGroupwareResource::onCollectionChangedFinished(KJob *job, const Collection &collection)
{
    if (job->error()) {
        cancelTask(i18n("Unable to modify collection: %1", job->errorText()));
        return;
    }
    changeCommitted(collection);
}

void DavGroupwareResource::doSetOnline(bool online)
{
    qCDebug(DAVRESOURCE_LOG) << "Resource changed online status to" << online;

    if (online) {
        synchronize();
    }

    ResourceBase::doSetOnline(online);
}

void DavGroupwareResource::createInitialCache()
{
    // Get all the items fetched by this resource
    auto job = new Akonadi::RecursiveItemFetchJob(mDavCollectionRoot, QStringList());
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    connect(job, &Akonadi::RecursiveItemFetchJob::result, this, &DavGroupwareResource::onCreateInitialCacheReady);
    job->start();
}

void DavGroupwareResource::onCreateInitialCacheReady(KJob *job)
{
    auto fetchJob = qobject_cast<Akonadi::RecursiveItemFetchJob *>(job);

    const Akonadi::Item::List itemsLst = fetchJob->items();
    for (const Akonadi::Item &item : itemsLst) {
        const QString rid = item.remoteId();
        if (rid.isEmpty()) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onCreateInitialCacheReady: Found an item without remote ID. " << item.id();
            continue;
        }

        const Akonadi::Collection collection = item.parentCollection();
        if (collection.remoteId().isEmpty()) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onCreateInitialCacheReady: Found an item in a collection without remote ID. " << item.remoteId();
            continue;
        }

        const QString etag = item.remoteRevision();
        if (etag.isEmpty()) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onCreateInitialCacheReady: Found an item without ETag. " << item.remoteId();
            continue;
        }

        if (!mEtagCaches.contains(collection.remoteId())) {
            auto cache = std::shared_ptr<KDAV::EtagCache>(new AkonadiEtagCache(collection));
            mEtagCaches.insert(collection.remoteId(), cache);
        }

        mEtagCaches[collection.remoteId()]->setEtag(rid, etag);
    }
    taskDone();
}

void DavGroupwareResource::onReloadConfig()
{
    Settings::self()->reloadConfig();
    synchronize();
}

void DavGroupwareResource::onCollectionRemovedFinished(KJob *job)
{
    if (job->error()) {
        cancelTask(i18n("Unable to remove collection: %1", job->errorText()));
        return;
    }

    auto collection = job->property("collection").value<Akonadi::Collection>();

    if (mEtagCaches.contains(collection.remoteId())) {
        mEtagCaches[collection.remoteId()]->deleteLater();
        mEtagCaches.remove(collection.remoteId());
    }

    changeProcessed();
}

void DavGroupwareResource::onRetrieveCollectionsFinished(KJob *job)
{
    const KDAV::DavCollectionsMultiFetchJob *fetchJob = qobject_cast<KDAV::DavCollectionsMultiFetchJob *>(job);

    if (job->error()) {
        qCWarning(DAVRESOURCE_LOG) << "Unable to fetch collections" << job->error() << job->errorText();
        cancelTask(i18n("Unable to retrieve collections: %1", job->errorText()));
        mSyncErrorNotified = true;
        return;
    }

    bool initialCacheSync = job->property("initialCacheSync").toBool();
    Akonadi::Collection::List collections{mDavCollectionRoot};
    QSet<QString> seenCollectionsUrls;

    const KDAV::DavCollection::List davCollections = fetchJob->collections();

    for (const KDAV::DavCollection &davCollection : davCollections) {
        if (seenCollectionsUrls.contains(davCollection.url().toDisplayString())) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onRetrieveCollectionsFinished: Duplicate collection reported. "
                                     << davCollection.url().toDisplayString();
            continue;
        } else {
            seenCollectionsUrls.insert(davCollection.url().toDisplayString());
        }

        Akonadi::Collection collection;
        collection.setParentCollection(mDavCollectionRoot);
        collection.setRemoteId(davCollection.url().toDisplayString());
        collection.setName(collection.remoteId());

        if (davCollection.color().isValid()) {
            auto colorAttr = collection.attribute<CollectionColorAttribute>(Akonadi::Collection::AddIfMissing);
            colorAttr->setColor(davCollection.color());
        }

        if (!davCollection.displayName().isEmpty()) {
            auto attr = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
            attr->setDisplayName(davCollection.displayName());
        }

        QStringList mimeTypes;
        mimeTypes << Collection::mimeType();

        const KDAV::DavCollection::ContentTypes contentTypes = davCollection.contentTypes();
        if (contentTypes & KDAV::DavCollection::Calendar) {
            mimeTypes << QStringLiteral("text/calendar");
        }

        if (contentTypes & KDAV::DavCollection::Events) {
            mimeTypes << KCalendarCore::Event::eventMimeType();
        }

        if (contentTypes & KDAV::DavCollection::Todos) {
            mimeTypes << KCalendarCore::Todo::todoMimeType();
        }

        if (contentTypes & KDAV::DavCollection::Contacts) {
            mimeTypes << KContacts::Addressee::mimeType();
        }

        if (contentTypes & KDAV::DavCollection::FreeBusy) {
            mimeTypes << KCalendarCore::FreeBusy::freeBusyMimeType();
        }

        if (contentTypes & KDAV::DavCollection::Journal) {
            mimeTypes << KCalendarCore::Journal::journalMimeType();
        }

        collection.setContentMimeTypes(mimeTypes);
        setCollectionIcon(collection /*by-ref*/);

        auto protoAttr = collection.attribute<DavProtocolAttribute>(Collection::AddIfMissing);
        protoAttr->setDavProtocol(davCollection.url().protocol());

        /*
         * We unfortunately have to update the CTag now in the cache
         * as this information will not be available when retrieveItems()
         * is called. We leave it untouched in the collection attribute
         * and will only update it there after successful sync.
         */
        if (!davCollection.CTag().isEmpty()) {
            mCTagCache.insert(davCollection.url().toDisplayString(), davCollection.CTag());
        }

        KDAV::Privileges privileges = davCollection.privileges();
        Akonadi::Collection::Rights rights;

        if (privileges & KDAV::All || privileges & KDAV::Write) {
            rights |= Akonadi::Collection::AllRights;
        }

        if (privileges & KDAV::WriteContent) {
            rights |= Akonadi::Collection::CanChangeItem;
        }

        if (privileges & KDAV::Bind) {
            rights |= Akonadi::Collection::CanCreateItem;
        }

        if (privileges & KDAV::Unbind) {
            rights |= Akonadi::Collection::CanDeleteItem;
        }

        if (privileges == KDAV::Read) {
            rights |= Akonadi::Collection::ReadOnly;
        }

        collection.setRights(rights);
        collections << collection;

        if (!mEtagCaches.contains(collection.remoteId())) {
            auto cache = std::shared_ptr<KDAV::EtagCache>(new AkonadiEtagCache(collection));
            mEtagCaches.insert(collection.remoteId(), cache);
        }
    }

    const auto keys{mEtagCaches.keys()};
    for (const QString &rid : keys) {
        if (!seenCollectionsUrls.contains(rid)) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onRetrieveCollectionsFinished: Collection disappeared. " << rid;
            mEtagCaches[rid]->deleteLater();
            mEtagCaches.remove(rid);
        }
    }

    if (!initialCacheSync) {
        collectionsRetrieved(collections);
    } else {
        taskDone();
    }
}

void DavGroupwareResource::onRetrieveItemsFinished(KJob *job)
{
    if (job->error()) {
        if (mSyncErrorNotified) {
            cancelTask();
        } else {
            cancelTask(i18n("Unable to retrieve items: %1", job->errorText()));
            mSyncErrorNotified = true;
        }
        return;
    }

    auto collection = job->property("collection").value<Collection>();
    const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId());
    const bool protocolSupportsMultiget = KDAV::ProtocolInfo::useMultiget(davUrl.protocol());

    const KDAV::DavItemsListJob *listJob = qobject_cast<KDAV::DavItemsListJob *>(job);
    auto cache = mEtagCaches.value(collection.remoteId());
    if (!cache) {
        qCDebug(DAVRESOURCE_LOG) << "Collection has disappeared during item fetch!";
        cancelTask();
        return;
    }

    Akonadi::Item::List changedItems;
    QSet<QString> seenRids;
    QStringList changedRids;
    changedItems.reserve(listJob->changedItems().count());
    const auto listJobChangedItems{listJob->changedItems()};
    for (const KDAV::DavItem &davItem : listJobChangedItems) {
        seenRids.insert(davItem.url().toDisplayString());

        Akonadi::Item item;
        item.setParentCollection(collection);
        item.setRemoteId(davItem.url().toDisplayString());
        item.setMimeType(davItem.contentType());
        item.setRemoteRevision(davItem.etag());

        cache->markAsChanged(item.remoteId());
        changedRids << item.remoteId();
        changedItems << item;

        // Only clear the payload (and therefore trigger a refetch from the backend) if we
        // do not use multiget, because in this case we fetch the complete payload
        // some lines below already.
        if (!protocolSupportsMultiget) {
            qCDebug(DAVRESOURCE_LOG) << "Outdated item " << item.remoteId() << " (etag = " << davItem.etag() << ")";
            item.clearPayload();
        }
    }

    const auto listJobDeleteItems{listJob->deletedItems()};
    for (const QString &rmd : listJobDeleteItems) {
        // We don't want to delete dependent items if the main item was seen
        if (rmd.contains(QLatin1Char('#'))) {
            const QString base = rmd.left(rmd.indexOf(QLatin1Char('#')));
            if (seenRids.contains(base)) {
                continue;
            }
        }

        qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onRetrieveItemsFinished: Item disappeared. " << rmd;
        Akonadi::Item item;
        item.setParentCollection(collection);
        item.setRemoteId(rmd);
        cache->removeEtag(rmd);

        // Use a job to delete items as itemsRetrievedIncremental seem to choke
        // when many items are given with just their RID.
        auto deleteJob = new Akonadi::ItemDeleteJob(item);
        deleteJob->start();
    }

    // If the protocol supports multiget then deviate from the expected behavior
    // and fetch all items with payload now instead of waiting for Akonadi to
    // request it item by item in retrieveItem().
    // This allows the resource to use the multiget query and let it be nice
    // to the remote server : only one request for n items instead of n requests.
    if (protocolSupportsMultiget && !changedRids.isEmpty()) {
        auto fetchJob = new KDAV::DavItemsFetchJob(davUrl, changedRids);
        connect(fetchJob, &KDAV::DavItemsFetchJob::result, this, &DavGroupwareResource::onMultigetFinished);
        fetchJob->setProperty("collection", QVariant::fromValue(collection));
        fetchJob->setProperty("items", QVariant::fromValue(changedItems));
        fetchJob->start();
        // delay the call of itemsRetrieved() to onMultigetFinished()
    } else {
        // Update the collection CTag attribute now as sync is done.
        if (mCTagCache.contains(collection.remoteId())) {
            auto CTagAttr = collection.attribute<CTagAttribute>(Collection::AddIfMissing);
            qCDebug(DAVRESOURCE_LOG) << "Updating collection CTag from" << CTagAttr->CTag() << "to" << mCTagCache.value(collection.remoteId());
            CTagAttr->setCTag(mCTagCache.value(collection.remoteId()));
            auto modifyJob = new Akonadi::CollectionModifyJob(collection);
            modifyJob->start();
        }

        itemsRetrievedIncremental(changedItems, Akonadi::Item::List());
    }
}

void DavGroupwareResource::onMultigetFinished(KJob *job)
{
    if (job->error()) {
        if (mSyncErrorNotified) {
            cancelTask();
        } else {
            cancelTask(i18n("Unable to retrieve items: %1", job->errorText()));
            mSyncErrorNotified = true;
        }
        return;
    }

    auto collection = job->property("collection").value<Akonadi::Collection>();
    auto cache = mEtagCaches.value(collection.remoteId());
    if (!cache) {
        qCDebug(DAVRESOURCE_LOG) << "Collection has disappeared during item fetch!";
        cancelTask();
        return;
    }

    const auto origItems = job->property("items").value<Akonadi::Item::List>();
    const KDAV::DavItemsFetchJob *davJob = qobject_cast<KDAV::DavItemsFetchJob *>(job);

    Akonadi::Item::List items;
    for (Akonadi::Item item : std::as_const(origItems)) {
        const KDAV::DavItem davItem = davJob->item(item.remoteId());

        // No data was retrieved for this item, maybe because it is not out of date
        if (davItem.data().isEmpty()) {
            qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onMultigetFinished: Empty item returned. " << item.remoteId();
            if (!cache->isOutOfDate(item.remoteId())) {
                qCDebug(DAVRESOURCE_LOG) << "DavGroupwareResource::onMultigetFinished: Item is not changed, including it. " << item.remoteId();
                items << item;
            }
            continue;
        }

        Akonadi::Item::List extraItems;
        if (!Utils::parseDavData(davItem, item, extraItems)) {
            qCWarning(DAVRESOURCE_LOG) << "DavGroupwareResource::onMultigetFinished: Failed to parse item data. " << item.remoteId();
            continue;
        }

        // update etag
        item.setRemoteRevision(davItem.etag());
        cache->setEtag(item.remoteId(), davItem.etag());
        items << item;
        for (const Akonadi::Item &extraItem : std::as_const(extraItems)) {
            cache->setEtag(extraItem.remoteId(), davItem.etag());
            items << extraItem;
        }
    }

    // Update the collection CTag attribute now as sync is done.
    if (mCTagCache.contains(collection.remoteId())) {
        auto CTagAttr = collection.attribute<CTagAttribute>(Collection::AddIfMissing);
        qCDebug(DAVRESOURCE_LOG) << "Updating collection CTag from" << CTagAttr->CTag() << "to" << mCTagCache.value(collection.remoteId());
        CTagAttr->setCTag(mCTagCache.value(collection.remoteId()));
        auto modifyJob = new Akonadi::CollectionModifyJob(collection);
        modifyJob->start();
    }

    itemsRetrievedIncremental(items, Akonadi::Item::List());
}

void DavGroupwareResource::onRetrieveItemFinished(KJob *job)
{
    onItemFetched(job, ItemUpdateAdd);
}

void DavGroupwareResource::onItemRefreshed(KJob *job)
{
    ItemFetchUpdateType update = ItemUpdateChange;
    if (job->property("isRemoval").isValid() && job->property("isRemoval").toBool()) {
        update = ItemUpdateNone;
    }

    onItemFetched(job, update);
}

void DavGroupwareResource::onItemFetched(KJob *job, ItemFetchUpdateType updateType)
{
    if (job->error()) {
        if (mSyncErrorNotified) {
            cancelTask();
        } else {
            cancelTask(i18n("Unable to retrieve item: %1", job->errorText()));
            mSyncErrorNotified = true;
        }
        return;
    }

    const KDAV::DavItemFetchJob *fetchJob = qobject_cast<KDAV::DavItemFetchJob *>(job);
    const KDAV::DavItem davItem = fetchJob->item();
    auto item = fetchJob->property("item").value<Akonadi::Item>();
    auto collection = fetchJob->property("collection").value<Akonadi::Collection>();

    Akonadi::Item::List extraItems;
    if (!Utils::parseDavData(davItem, item, extraItems)) {
        qCWarning(DAVRESOURCE_LOG) << "DavGroupwareResource::onItemFetched: Failed to parse item data. " << item.remoteId();
        // We get some XML error when the item doesn't exist:
        // <d:error xmlns:d="DAV:" xmlns:s="http://sabredav.org/ns">
        //  <s:sabredav-version>2.1.11</s:sabredav-version>
        //  <s:exception>Sabre\DAV\Exception\NotFound</s:exception>
        //  <s:message>Calendar object not found</s:message>
        //</d:error>
        cancelTask(i18n("Unable to retrieve item: failed to parse item data. Maybe it was deleted already."));
        return;
    }

    // update etag
    item.setRemoteRevision(davItem.etag());
    auto etag = mEtagCaches[collection.remoteId()];
    etag->setEtag(item.remoteId(), davItem.etag());

    if (!extraItems.isEmpty()) {
        for (int i = 0, total = extraItems.size(); i < total; ++i) {
            etag->setEtag(extraItems.at(i).remoteId(), davItem.etag());
        }

        auto j = new Akonadi::ItemModifyJob(extraItems);
        j->setIgnorePayload(true);
    }

    if (updateType == ItemUpdateChange) {
        changeCommitted(item);
    } else if (updateType == ItemUpdateAdd) {
        itemRetrieved(item);
    }
}

void DavGroupwareResource::onItemAddedFinished(KJob *job)
{
    const KDAV::DavItemCreateJob *createJob = qobject_cast<KDAV::DavItemCreateJob *>(job);
    KDAV::DavItem davItem = createJob->item();
    auto item = createJob->property("item").value<Akonadi::Item>();
    item.setRemoteId(davItem.url().toDisplayString());

    if (createJob->error()) {
        qCCritical(DAVRESOURCE_LOG) << "Error when uploading item:" << createJob->error() << createJob->errorString();
        if (createJob->canRetryLater()) {
            retryAfterFailure(createJob->errorString());
        } else {
            cancelTask(i18n("Unable to add item: %1", createJob->errorString()));
        }
        return;
    }

    auto collection = createJob->property("collection").value<Akonadi::Collection>();

    if (davItem.etag().isEmpty()) {
        const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId(), item.remoteId());
        davItem.setUrl(davUrl);
        auto fetchJob = new KDAV::DavItemFetchJob(davItem);
        fetchJob->setProperty("item", QVariant::fromValue(item));
        fetchJob->setProperty("collection", QVariant::fromValue(collection));
        connect(fetchJob, &KDAV::DavItemFetchJob::result, this, &DavGroupwareResource::onItemRefreshed);
        fetchJob->start();
    } else {
        item.setRemoteRevision(davItem.etag());
        mEtagCaches[collection.remoteId()]->setEtag(davItem.url().toDisplayString(), davItem.etag());
        changeCommitted(item);
    }
}

void DavGroupwareResource::onItemChangedFinished(KJob *job)
{
    const KDAV::DavItemModifyJob *modifyJob = qobject_cast<KDAV::DavItemModifyJob *>(job);
    KDAV::DavItem davItem = modifyJob->item();
    auto collection = modifyJob->property("collection").value<Akonadi::Collection>();
    auto item = modifyJob->property("item").value<Akonadi::Item>();
    auto dependentItems = modifyJob->property("dependentItems").value<Akonadi::Item::List>();
    bool isRemoval = modifyJob->property("isRemoval").isValid() && modifyJob->property("isRemoval").toBool();
    auto cache = mEtagCaches.value(collection.remoteId());
    if (!cache) {
        qCDebug(DAVRESOURCE_LOG) << "Collection has disappeared during item fetch!";
        cancelTask();
        return;
    }

    if (modifyJob->error()) {
        qCCritical(DAVRESOURCE_LOG) << "Error when uploading item:" << modifyJob->error() << modifyJob->errorString();
        if (modifyJob->hasConflict()) {
            handleConflict(item, dependentItems, modifyJob->freshItem(), isRemoval, modifyJob->freshResponseCode());
        } else if (modifyJob->canRetryLater()) {
            retryAfterFailure(modifyJob->errorString());
        } else {
            cancelTask(i18n("Unable to change item: %1", modifyJob->errorString()));
        }
        return;
    }

    if (isRemoval) {
        auto removedItem = job->property("removedItem").value<Akonadi::Item>();
        if (removedItem.isValid()) {
            cache->removeEtag(removedItem.remoteId());
            changeProcessed();
        }
    }

    if (davItem.etag().isEmpty()) {
        const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), item.remoteId());
        davItem.setUrl(davUrl);
        auto fetchJob = new KDAV::DavItemFetchJob(davItem);
        fetchJob->setProperty("item", QVariant::fromValue(item));
        fetchJob->setProperty("collection", QVariant::fromValue(collection));
        fetchJob->setProperty("dependentItems", QVariant::fromValue(dependentItems));
        fetchJob->setProperty("isRemoval", QVariant::fromValue(isRemoval));
        connect(fetchJob, &KDAV::DavItemsFetchJob::result, this, &DavGroupwareResource::onItemRefreshed);
        fetchJob->start();
    } else {
        if (!isRemoval) {
            item.setRemoteRevision(davItem.etag());
            cache->setEtag(davItem.url().toDisplayString(), davItem.etag());
            changeCommitted(item);
        }

        if (!dependentItems.isEmpty()) {
            for (int i = 0, total = dependentItems.size(); i < total; ++i) {
                dependentItems[i].setRemoteRevision(davItem.etag());
                cache->setEtag(dependentItems.at(i).remoteId(), davItem.etag());
            }

            auto j = new Akonadi::ItemModifyJob(dependentItems);
            j->setIgnorePayload(true);
        }
    }
}

void DavGroupwareResource::onDeletedItemRecreated(KJob *job)
{
    const KDAV::DavItemCreateJob *createJob = qobject_cast<KDAV::DavItemCreateJob *>(job);
    KDAV::DavItem davItem = createJob->item();
    auto item = createJob->property("item").value<Akonadi::Item>();
    Akonadi::Collection collection = item.parentCollection();
    auto dependentItems = createJob->property("dependentItems").value<Akonadi::Item::List>();

    if (davItem.etag().isEmpty()) {
        const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(item.parentCollection().remoteId(), item.remoteId());
        davItem.setUrl(davUrl);
        auto fetchJob = new KDAV::DavItemFetchJob(davItem);
        fetchJob->setProperty("item", QVariant::fromValue(item));
        fetchJob->setProperty("dependentItems", QVariant::fromValue(dependentItems));
        connect(fetchJob, &KDAV::DavItemFetchJob::result, this, &DavGroupwareResource::onItemRefreshed);
        fetchJob->start();
    } else {
        item.setRemoteRevision(davItem.etag());
        auto etag = mEtagCaches[collection.remoteId()];
        etag->setEtag(davItem.url().toDisplayString(), davItem.etag());
        changeCommitted(item);

        if (!dependentItems.isEmpty()) {
            for (int i = 0, total = dependentItems.size(); i < total; ++i) {
                dependentItems[i].setRemoteRevision(davItem.etag());
                etag->setEtag(dependentItems.at(i).remoteId(), davItem.etag());
            }

            auto j = new Akonadi::ItemModifyJob(dependentItems);
            j->setIgnorePayload(true);
        }
    }
}

void DavGroupwareResource::onItemRemovedFinished(KJob *job)
{
    if (job->error()) {
        const KDAV::DavItemDeleteJob *deleteJob = qobject_cast<KDAV::DavItemDeleteJob *>(job);

        if (deleteJob->hasConflict()) {
            // Use a shortcut here as we don't show a conflict dialog to the user.
            handleConflict(Akonadi::Item(), Akonadi::Item::List(), deleteJob->freshItem(), true, 0);
        } else if (deleteJob->canRetryLater()) {
            retryAfterFailure(job->errorString());
        } else {
            cancelTask(i18n("Unable to remove item: %1", job->errorString()));
        }
    } else {
        auto item = job->property("item").value<Akonadi::Item>();
        auto collection = job->property("collection").value<Akonadi::Collection>();
        mEtagCaches[collection.remoteId()]->removeEtag(item.remoteId());
        changeProcessed();
    }
}

void DavGroupwareResource::onCollectionDiscovered(KDAV::Protocol protocol, const QString &collection, const QString &config)
{
    Settings::self()->addCollectionUrlMapping(protocol, collection, config);
}

void DavGroupwareResource::handleConflict(const Item &lI, const Item::List &localDependentItems, const KDAV::DavItem &rI, bool isLocalRemoval, int responseCode)
{
    Akonadi::Item localItem(lI);
    Akonadi::Item remoteItem, tmpRemoteItem; // The tmp* vars are here to store the result of the parseDavData() call
    Akonadi::Item::List remoteDependentItems, tmpRemoteDependentItems; // as we have no idea which item triggered the conflict.
    qCDebug(DAVRESOURCE_LOG) << "Fresh response code is" << responseCode;
    bool isRemoteRemoval = (responseCode == 404 || responseCode == 410);

    if (!isRemoteRemoval) {
        if (!Utils::parseDavData(rI, tmpRemoteItem, tmpRemoteDependentItems)) {
            // TODO: set a more correct error message here
            cancelTask(i18n("Unable to change item: %1", QStringLiteral("conflict resolution failed")));
            return;
            // TODO: we can end up here if the remote item was deleted
        }

        // Now try to find the item that really triggered the conflict
        const Akonadi::Item::List allRemoteItems = Akonadi::Item::List() << tmpRemoteItem << tmpRemoteDependentItems;
        for (const Akonadi::Item &tmpItem : allRemoteItems) {
            if (tmpItem.payloadData() != localItem.payloadData()) {
                if (remoteItem.isValid()) {
                    // Oops, we can only manage one changed item at this stage, sorry...
                    // TODO: make this translatable
                    cancelTask(i18n("Unable to change item: %1", QStringLiteral("more than one item was changed in the backend")));
                    return;
                }
                remoteItem = tmpItem;
            } else {
                remoteDependentItems << tmpItem;
            }
        }
    }

    if (isLocalRemoval) {
        // TODO: implement with the configurable strategy
        /*
         * Here by default we don't delete an event that was modified in the backend, and
         * instead we just abort the current task.
         * Also, trigger an immediate sync to refresh the item.
         */
        qCDebug(DAVRESOURCE_LOG) << "Local removal conflict";
        // TODO: make this translatable
        cancelTask(i18n("Unable to remove item: %1", QStringLiteral("it was changed in the backend in the meantime")));
        synchronize();
    } else if (isRemoteRemoval) {
        // TODO: implement with the configurable strategy
        /*
         * Here also it is a bit tricky to clear the item in the local cache as the resource
         * will not get notified if the user chooses to delete the item and abandon the local
         * modification. For the time being let's just re-upload the changed item.
         */
        qCDebug(DAVRESOURCE_LOG) << "Remote removal conflict";
        Akonadi::Collection collection = localItem.parentCollection();
        KDAV::DavItem davItem = Utils::createDavItem(localItem, collection, localDependentItems);

        QString urlStr = localItem.remoteId();
        if (urlStr.contains(QLatin1Char('#'))) {
            urlStr.truncate(urlStr.indexOf(QLatin1Char('#')));
        }
        const KDAV::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl(collection.remoteId(), urlStr);
        davItem.setUrl(davUrl);

        auto job = new KDAV::DavItemCreateJob(davItem);
        job->setProperty("item", QVariant::fromValue(localItem));
        job->setProperty("dependentItems", QVariant::fromValue(localDependentItems));
        connect(job, &KJob::result, this, &DavGroupwareResource::onDeletedItemRecreated);
        job->start();
    } else {
        const QString remoteEtag = rI.etag();
        Akonadi::Collection collection = localItem.parentCollection();

        localItem.setRemoteRevision(remoteEtag);
        changeCommitted(localItem);

        // Update the ETag cache in all cases as the new ETag will have to be used
        // later for any update or deletion
        mEtagCaches[collection.remoteId()]->setEtag(rI.url().toDisplayString(), remoteEtag);

        // The first step is to fire a first modify job that will replace the item currently
        // in the local cache with the one that was found in the backend.
        Akonadi::Item updatedItem(localItem);
        updatedItem.setPayloadFromData(remoteItem.payloadData());
        updatedItem.setRemoteRevision(remoteEtag);
        auto j = new Akonadi::ItemModifyJob(updatedItem);
        j->setIgnorePayload(false);
        j->start();

        // So now we have in the cache what's in the backend but the user is not aware
        // that behind the scenes something terrible is happening. Well, nearly...
        // To notify him of this, and due to the way the conflict handler works, we have
        // to re-attempt a modification to revert the modify job that was just fired.
        // So yes, we are effectively re-submitting the client-provided content, but
        // with a revision that will trigger the conflict dialog.
        // The only problem is that the user will see that we update the item before
        // the conflict dialog has time to display (if it's not behind the application
        // window).
        localItem.setRevision(0);
        j = new Akonadi::ItemModifyJob(localItem);
        j->setIgnorePayload(false);
        connect(j, &KJob::result, this, &DavGroupwareResource::onConflictModifyJobFinished);
        j->start();

        // Hopefully for the dependent items everything will be fine. Right?
        // Not so sure in fact.
        if (!remoteDependentItems.isEmpty()) {
            auto etag = mEtagCaches[collection.remoteId()];
            for (int i = 0; i < remoteDependentItems.size(); ++i) {
                remoteDependentItems[i].setRemoteRevision(remoteEtag);
                etag->setEtag(remoteDependentItems.at(i).remoteId(), remoteEtag);
            }

            auto j = new Akonadi::ItemModifyJob(remoteDependentItems);
            j->setIgnorePayload(true);
        }
    }
}

void DavGroupwareResource::onConflictModifyJobFinished(KJob *job)
{
    auto j = qobject_cast<Akonadi::ItemModifyJob *>(job);
    if (j->error()) {
        qCCritical(DAVRESOURCE_LOG) << "Conflict update failed: " << job->errorText();
        // TODO: what do we do now? We just committed an item that's in a weird state...
    }
}

bool DavGroupwareResource::configurationIsValid()
{
    if (Settings::self()->configuredDavUrls().empty()) {
        Q_EMIT status(NotConfigured, i18n("The resource is not configured yet"));
        cancelTask(i18n("The resource is not configured yet"));
        return false;
    }

    int newICT = Settings::self()->refreshInterval();
    if (newICT == 0) {
        newICT = -1;
    }

    if (newICT != mDavCollectionRoot.cachePolicy().intervalCheckTime()) {
        Akonadi::CachePolicy cachePolicy = mDavCollectionRoot.cachePolicy();
        cachePolicy.setIntervalCheckTime(newICT);

        mDavCollectionRoot.setCachePolicy(cachePolicy);
    }

    if (!Settings::self()->displayName().isEmpty()) {
        auto attribute = mDavCollectionRoot.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
        attribute->setDisplayName(Settings::self()->displayName());
        setName(Settings::self()->displayName());
    }

    return true;
}

void DavGroupwareResource::retryAfterFailure(const QString &errorMessage)
{
    Q_EMIT status(Broken, errorMessage);
    deferTask();
    setTemporaryOffline(Settings::self()->refreshInterval() <= 0 ? 300 : Settings::self()->refreshInterval() * 60);
}

/*static*/
void DavGroupwareResource::setCollectionIcon(Akonadi::Collection &collection)
{
    const QStringList mimeTypes = collection.contentMimeTypes();
    if (mimeTypes.count() == 1) {
        QHash<QString, QString> mapping;
        mapping.insert(KCalendarCore::Event::eventMimeType(), QStringLiteral("view-calendar"));
        mapping.insert(KCalendarCore::Todo::todoMimeType(), QStringLiteral("view-calendar-tasks"));
        mapping.insert(KCalendarCore::Journal::journalMimeType(), QStringLiteral("view-pim-journal"));
        mapping.insert(KContacts::Addressee::mimeType(), QStringLiteral("view-pim-contacts"));

        const QString mimetypeFirst = mimeTypes.first();
        if (!mimetypeFirst.isEmpty()) {
            auto attribute = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
            attribute->setIconName(mimetypeFirst);
        }
    }
}

AKONADI_RESOURCE_MAIN(DavGroupwareResource)
