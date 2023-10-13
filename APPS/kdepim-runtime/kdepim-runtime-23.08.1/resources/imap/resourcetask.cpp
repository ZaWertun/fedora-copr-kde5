/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcetask.h"

#include <Akonadi/MessageFlags>

#include "imapresource_debug.h"
#include "imapresource_trace.h"
#include <KLocalizedString>

#include "collectionflagsattribute.h"
#include "imapflags.h"
#include "sessionpool.h"
#include <imapaclattribute.h>

ResourceTask::ResourceTask(ActionIfNoSession action, ResourceStateInterface::Ptr resource, QObject *parent)
    : QObject(parent)
    , m_pool(nullptr)
    , m_sessionRequestId(0)
    , m_session(nullptr)
    , m_actionIfNoSession(action)
    , m_resource(resource)
    , mCancelled(false)
{
}

ResourceTask::~ResourceTask()
{
    if (m_pool) {
        if (m_sessionRequestId) {
            m_pool->cancelSessionRequest(m_sessionRequestId);
        }
        if (m_session) {
            m_pool->releaseSession(m_session);
        }
    }
}

void ResourceTask::start(SessionPool *pool)
{
    qCDebug(IMAPRESOURCE_TRACE) << metaObject()->className();
    m_pool = pool;
    connect(m_pool, &SessionPool::sessionRequestDone, this, &ResourceTask::onSessionRequested);

    m_sessionRequestId = m_pool->requestSession();

    if (m_sessionRequestId <= 0) {
        m_sessionRequestId = 0;

        abortTask(QString());
        // In this case we were likely disconnected, try to get the resource online
        m_resource->scheduleConnectionAttempt();
    }
}

void ResourceTask::abortTask(const QString &errorString)
{
    if (!mCancelled) {
        mCancelled = true;

        switch (m_actionIfNoSession) {
        case CancelIfNoSession:
            qCDebug(IMAPRESOURCE_LOG) << "Cancelling this request.";
            m_resource->cancelTask(errorString.isEmpty() ? i18n("Unable to connect to the IMAP server.") : errorString);
            break;

        case DeferIfNoSession:
            qCDebug(IMAPRESOURCE_LOG) << "Deferring this request.";
            m_resource->deferTask();
            break;
        }
    }
    deleteLater();
}

void ResourceTask::onSessionRequested(qint64 requestId, KIMAP::Session *session, int errorCode, const QString &errorString)
{
    if (requestId != m_sessionRequestId) {
        // Not for us, ignore
        return;
    }

    disconnect(m_pool, &SessionPool::sessionRequestDone, this, &ResourceTask::onSessionRequested);
    m_sessionRequestId = 0;

    if (errorCode != SessionPool::NoError) {
        abortTask(errorString);
        return;
    }

    m_session = session;

    connect(m_pool, &SessionPool::connectionLost, this, &ResourceTask::onConnectionLost);
    connect(m_pool, &SessionPool::disconnectDone, this, &ResourceTask::onPoolDisconnect);

    qCDebug(IMAPRESOURCE_TRACE) << "starting: " << metaObject()->className();
    doStart(m_session);
}

void ResourceTask::onConnectionLost(KIMAP::Session *session)
{
    if (session == m_session) {
        // Our session becomes invalid, so get rid of
        // the pointer, we don't need to release it once the
        // task is done
        m_session = nullptr;
        qCDebug(IMAPRESOURCE_TRACE) << metaObject()->className();
        abortTask(i18n("Connection lost"));
    }
}

void ResourceTask::onPoolDisconnect()
{
    // All the sessions in the pool we used changed,
    // so get rid of the pointer, we don't need to
    // release our session anymore
    m_pool = nullptr;

    qCDebug(IMAPRESOURCE_TRACE) << metaObject()->className();
    abortTask(i18n("Connection lost"));
}

QString ResourceTask::userName() const
{
    return m_resource->userName();
}

QString ResourceTask::resourceName() const
{
    return m_resource->resourceName();
}

QStringList ResourceTask::serverCapabilities() const
{
    return m_resource->serverCapabilities();
}

QList<KIMAP::MailBoxDescriptor> ResourceTask::serverNamespaces() const
{
    return m_resource->serverNamespaces();
}

bool ResourceTask::isAutomaticExpungeEnabled() const
{
    return m_resource->isAutomaticExpungeEnabled();
}

bool ResourceTask::isSubscriptionEnabled() const
{
    return m_resource->isSubscriptionEnabled();
}

bool ResourceTask::isDisconnectedModeEnabled() const
{
    return m_resource->isDisconnectedModeEnabled();
}

int ResourceTask::intervalCheckTime() const
{
    return m_resource->intervalCheckTime();
}

static Akonadi::Collection detachCollection(const Akonadi::Collection &collection)
{
    // HACK: Attributes are accessed via a const function, and the implicitly shared private pointer thus doesn't detach.
    // We force a detach to avoid surprises. (RetrieveItemsTask used to write back the collection changes, even though the task was canceled)
    // Once this is fixed this function can go away.
    Akonadi::Collection col = collection;
    col.setId(col.id());
    return col;
}

Akonadi::Collection ResourceTask::collection() const
{
    return detachCollection(m_resource->collection());
}

Akonadi::Item ResourceTask::item() const
{
    return m_resource->item();
}

Akonadi::Item::List ResourceTask::items() const
{
    return m_resource->items();
}

Akonadi::Collection ResourceTask::parentCollection() const
{
    return detachCollection(m_resource->parentCollection());
}

Akonadi::Collection ResourceTask::sourceCollection() const
{
    return detachCollection(m_resource->sourceCollection());
}

Akonadi::Collection ResourceTask::targetCollection() const
{
    return detachCollection(m_resource->targetCollection());
}

QSet<QByteArray> ResourceTask::parts() const
{
    return m_resource->parts();
}

QSet<QByteArray> ResourceTask::addedFlags() const
{
    return m_resource->addedFlags();
}

QSet<QByteArray> ResourceTask::removedFlags() const
{
    return m_resource->removedFlags();
}

QString ResourceTask::rootRemoteId() const
{
    return m_resource->rootRemoteId();
}

QString ResourceTask::mailBoxForCollection(const Akonadi::Collection &collection) const
{
    return m_resource->mailBoxForCollection(collection);
}

void ResourceTask::setIdleCollection(const Akonadi::Collection &collection)
{
    if (!mCancelled) {
        m_resource->setIdleCollection(collection);
    }
}

void ResourceTask::applyCollectionChanges(const Akonadi::Collection &collection)
{
    if (!mCancelled) {
        m_resource->applyCollectionChanges(collection);
    }
}

void ResourceTask::itemRetrieved(const Akonadi::Item &item)
{
    if (!mCancelled) {
        m_resource->itemRetrieved(item);
        emitPercent(100);
    }
    deleteLater();
}

void ResourceTask::itemsRetrieved(const Akonadi::Item::List &items)
{
    if (!mCancelled) {
        m_resource->itemsRetrieved(items);
    }
}

void ResourceTask::itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed)
{
    if (!mCancelled) {
        m_resource->itemsRetrievedIncremental(changed, removed);
    }
}

void ResourceTask::itemsRetrievalDone()
{
    if (!mCancelled) {
        m_resource->itemsRetrievalDone();
    }
    deleteLater();
}

void ResourceTask::setTotalItems(int totalItems)
{
    if (!mCancelled) {
        m_resource->setTotalItems(totalItems);
    }
}

void ResourceTask::changeCommitted(const Akonadi::Item &item)
{
    if (!mCancelled) {
        m_resource->itemChangeCommitted(item);
    }
    deleteLater();
}

void ResourceTask::changesCommitted(const Akonadi::Item::List &items)
{
    if (!mCancelled) {
        m_resource->itemsChangesCommitted(items);
    }
    deleteLater();
}

void ResourceTask::searchFinished(const QVector<qint64> &result, bool isRid)
{
    if (!mCancelled) {
        m_resource->searchFinished(result, isRid);
    }
    deleteLater();
}

void ResourceTask::collectionsRetrieved(const Akonadi::Collection::List &collections)
{
    if (!mCancelled) {
        m_resource->collectionsRetrieved(collections);
    }
    deleteLater();
}

void ResourceTask::collectionAttributesRetrieved(const Akonadi::Collection &col)
{
    if (!mCancelled) {
        m_resource->collectionAttributesRetrieved(col);
    }
    deleteLater();
}

void ResourceTask::changeCommitted(const Akonadi::Collection &collection)
{
    if (!mCancelled) {
        m_resource->collectionChangeCommitted(collection);
    }
    deleteLater();
}

void ResourceTask::changeCommitted(const Akonadi::Tag &tag)
{
    if (!mCancelled) {
        m_resource->tagChangeCommitted(tag);
    }
    deleteLater();
}

void ResourceTask::changeProcessed()
{
    if (!mCancelled) {
        m_resource->changeProcessed();
    }
    deleteLater();
}

void ResourceTask::cancelTask(const QString &errorString)
{
    qCDebug(IMAPRESOURCE_LOG) << "Cancel task: " << errorString;
    if (!mCancelled) {
        mCancelled = true;
        m_resource->cancelTask(errorString);
    }
    deleteLater();
}

void ResourceTask::deferTask()
{
    if (!mCancelled) {
        mCancelled = true;
        m_resource->deferTask();
    }
    deleteLater();
}

void ResourceTask::restartItemRetrieval(Akonadi::Collection::Id col)
{
    if (!mCancelled) {
        m_resource->restartItemRetrieval(col);
    }
    deleteLater();
}

void ResourceTask::taskDone()
{
    m_resource->taskDone();
    deleteLater();
}

void ResourceTask::emitPercent(int percent)
{
    m_resource->emitPercent(percent);
}

void ResourceTask::emitError(const QString &message)
{
    m_resource->emitError(message);
}

void ResourceTask::emitWarning(const QString &message)
{
    m_resource->emitWarning(message);
}

void ResourceTask::synchronizeCollectionTree()
{
    m_resource->synchronizeCollectionTree();
}

void ResourceTask::showInformationDialog(const QString &message, const QString &title, const QString &dontShowAgainName)
{
    m_resource->showInformationDialog(message, title, dontShowAgainName);
}

QList<QByteArray> ResourceTask::fromAkonadiToSupportedImapFlags(const QList<QByteArray> &flags, const Akonadi::Collection &collection)
{
    QList<QByteArray> imapFlags = fromAkonadiFlags(flags);

    const auto flagAttr = collection.attribute<Akonadi::CollectionFlagsAttribute>();
    // the server does not support arbitrary flags, so filter out those it can't handle
    if (flagAttr && !flagAttr->flags().isEmpty() && !flagAttr->flags().contains("\\*")) {
        for (QList<QByteArray>::iterator it = imapFlags.begin(); it != imapFlags.end();) {
            if (flagAttr->flags().contains(*it)) {
                ++it;
            } else {
                qCDebug(IMAPRESOURCE_LOG) << "Server does not support flag" << *it;
                it = imapFlags.erase(it);
            }
        }
    }

    return imapFlags;
}

QList<QByteArray> ResourceTask::fromAkonadiFlags(const QList<QByteArray> &flags)
{
    QList<QByteArray> newFlags;

    for (const QByteArray &oldFlag : flags) {
        if (oldFlag == Akonadi::MessageFlags::Seen) {
            newFlags.append(ImapFlags::Seen);
        } else if (oldFlag == Akonadi::MessageFlags::Deleted) {
            newFlags.append(ImapFlags::Deleted);
        } else if (oldFlag == Akonadi::MessageFlags::Answered || oldFlag == Akonadi::MessageFlags::Replied) {
            newFlags.append(ImapFlags::Answered);
        } else if (oldFlag == Akonadi::MessageFlags::Flagged) {
            newFlags.append(ImapFlags::Flagged);
        } else {
            newFlags.append(oldFlag);
        }
    }

    return newFlags;
}

QSet<QByteArray> ResourceTask::toAkonadiFlags(const QList<QByteArray> &flags)
{
    QSet<QByteArray> newFlags;

    for (const QByteArray &oldFlag : flags) {
        if (oldFlag == ImapFlags::Seen) {
            newFlags.insert(Akonadi::MessageFlags::Seen);
        } else if (oldFlag == ImapFlags::Deleted) {
            newFlags.insert(Akonadi::MessageFlags::Deleted);
        } else if (oldFlag == ImapFlags::Answered) {
            newFlags.insert(Akonadi::MessageFlags::Answered);
        } else if (oldFlag == ImapFlags::Flagged) {
            newFlags.insert(Akonadi::MessageFlags::Flagged);
        } else if (oldFlag.isEmpty()) {
            // filter out empty flags, to avoid isNull/isEmpty confusions higher up
            continue;
        } else {
            newFlags.insert(oldFlag);
        }
    }

    return newFlags;
}

void ResourceTask::kill()
{
    qCDebug(IMAPRESOURCE_TRACE) << metaObject()->className();
    abortTask(i18n("killed"));
}

const QChar ResourceTask::separatorCharacter() const
{
    const QChar separator = m_resource->separatorCharacter();
    if (!separator.isNull()) {
        return separator;
    } else {
        // If we request the separator before first folder listing, then try to guess
        // the separator:
        // If we create a toplevel folder, assume the separator to be '/'. This is not perfect, but detecting the right
        // IMAP separator is not straightforward for toplevel folders, and fixes bug 292418 and maybe other, where
        // subfolders end up with remote id's starting with "i" (the first letter of imap:// ...)

        QString remoteId;
        // We don't always have parent collection set (for example for CollectionChangeTask),
        // in such cases however we can use current collection's remoteId to get the separator
        const Akonadi::Collection parent = parentCollection();
        if (parent.isValid()) {
            remoteId = parent.remoteId();
        } else {
            remoteId = collection().remoteId();
        }
        return ((remoteId != rootRemoteId()) && !remoteId.isEmpty()) ? remoteId.at(0) : QLatin1Char('/');
    }
}

void ResourceTask::setSeparatorCharacter(QChar separator)
{
    m_resource->setSeparatorCharacter(separator);
}

bool ResourceTask::serverSupportsAnnotations() const
{
    return serverCapabilities().contains(QLatin1String("METADATA")) || serverCapabilities().contains(QLatin1String("ANNOTATEMORE"));
}

bool ResourceTask::serverSupportsCondstore() const
{
    // Don't enable CONDSTORE for GMail (X-GM-EXT-1 is a GMail-specific capability)
    // because it breaks changes synchronization when using labels.
    return serverCapabilities().contains(QLatin1String("CONDSTORE")) && !serverCapabilities().contains(QLatin1String("X-GM-EXT-1"));
}

int ResourceTask::batchSize() const
{
    return m_resource->batchSize();
}

ResourceStateInterface::Ptr ResourceTask::resourceState()
{
    return m_resource;
}

KIMAP::Acl::Rights ResourceTask::myRights(const Akonadi::Collection &col)
{
    const auto aclAttribute = col.attribute<Akonadi::ImapAclAttribute>();
    if (aclAttribute) {
        // HACK, only return myrights if they are available
        if (aclAttribute->myRights() != KIMAP::Acl::None) {
            return aclAttribute->myRights();
        } else {
            // This should be removed after 4.14, and myrights should be always used.
            return aclAttribute->rights().value(userName().toUtf8());
        }
    }
    return KIMAP::Acl::None;
}

void ResourceTask::setItemMergingMode(Akonadi::ItemSync::MergeMode mode)
{
    m_resource->setItemMergingMode(mode);
}
