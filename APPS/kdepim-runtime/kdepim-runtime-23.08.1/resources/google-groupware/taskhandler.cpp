/*
    SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "taskhandler.h"
#include "googleresource.h"
#include "googlesettings.h"
#include "googletasks_debug.h"

#include <Akonadi/BlockAlarmsAttribute>
#include <Akonadi/CollectionColorAttribute>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>

#include <KGAPI/Account>
#include <KGAPI/Tasks/Task>
#include <KGAPI/Tasks/TaskCreateJob>
#include <KGAPI/Tasks/TaskDeleteJob>
#include <KGAPI/Tasks/TaskFetchJob>
#include <KGAPI/Tasks/TaskList>
#include <KGAPI/Tasks/TaskListCreateJob>
#include <KGAPI/Tasks/TaskListDeleteJob>
#include <KGAPI/Tasks/TaskListFetchJob>
#include <KGAPI/Tasks/TaskListModifyJob>
#include <KGAPI/Tasks/TaskModifyJob>
#include <KGAPI/Tasks/TaskMoveJob>

#include <KCalendarCore/Todo>

#define TASK_PROPERTY "_KGAPI2::TaskPtr"

using namespace KGAPI2;
using namespace Akonadi;

QString TaskHandler::mimeType()
{
    return KCalendarCore::Todo::todoMimeType();
}

bool TaskHandler::canPerformTask(const Item &item)
{
    return GenericHandler::canPerformTask<KCalendarCore::Todo::Ptr>(item);
}

bool TaskHandler::canPerformTask(const Item::List &items)
{
    return GenericHandler::canPerformTask<KCalendarCore::Todo::Ptr>(items);
}

void TaskHandler::setupCollection(Collection &collection, const TaskListPtr &taskList)
{
    collection.setContentMimeTypes({mimeType()});
    collection.setName(taskList->uid());
    collection.setRemoteId(taskList->uid());
    collection.setRights(Collection::CanChangeCollection | Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem);

    auto attr = collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing);
    attr->setDisplayName(taskList->title());
    attr->setIconName(QStringLiteral("view-pim-tasks"));
}

void TaskHandler::retrieveCollections(const Collection &rootCollection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Retrieving task lists"));
    qCDebug(GOOGLE_TASKS_LOG) << "Retrieving tasks...";
    auto job = new TaskListFetchJob(m_settings->accountPtr(), this);
    connect(job, &TaskListFetchJob::finished, this, [this, rootCollection](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        qCDebug(GOOGLE_TASKS_LOG) << "Task lists retrieved";

        const ObjectsList taskLists = qobject_cast<TaskListFetchJob *>(job)->items();
        const QStringList activeTaskLists = m_settings->taskLists();
        Collection::List collections;
        for (const ObjectPtr &object : taskLists) {
            const TaskListPtr &taskList = object.dynamicCast<TaskList>();
            qCDebug(GOOGLE_TASKS_LOG) << " -" << taskList->title() << "(" << taskList->uid() << ")";

            if (!activeTaskLists.contains(taskList->uid())) {
                qCDebug(GOOGLE_TASKS_LOG) << "Skipping, not subscribed";
                continue;
            }

            Collection collection;
            setupCollection(collection, taskList);
            collection.setParentCollection(rootCollection);
            collections << collection;
        }

        m_iface->collectionsRetrievedFromHandler(collections);
    });
}

void TaskHandler::retrieveItems(const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Retrieving tasks for list '%1'", collection.displayName()));
    qCDebug(GOOGLE_TASKS_LOG) << "Retrieving tasks for list" << collection.remoteId();
    // https://bugs.kde.org/show_bug.cgi?id=308122: we can only request changes in
    // max. last 25 days, otherwise we get an error.
    int lastSyncDelta = -1;
    if (!collection.remoteRevision().isEmpty()) {
        lastSyncDelta = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() - collection.remoteRevision().toULongLong();
    }

    auto job = new TaskFetchJob(collection.remoteId(), m_settings->accountPtr(), this);
    if (lastSyncDelta > -1 && lastSyncDelta < 25 * 25 * 3600) {
        job->setFetchOnlyUpdated(collection.remoteRevision().toULongLong());
        job->setFetchDeleted(true);
    } else {
        // No need to fetch deleted items for non-incremental update
        job->setFetchDeleted(false);
    }
    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &TaskFetchJob::finished, this, &TaskHandler::slotItemsRetrieved);
}

void TaskHandler::slotItemsRetrieved(KGAPI2::Job *job)
{
    if (!m_iface->handleError(job)) {
        return;
    }
    Item::List changedItems, removedItems;

    const ObjectsList &objects = qobject_cast<FetchJob *>(job)->items();
    auto collection = job->property(COLLECTION_PROPERTY).value<Collection>();
    bool isIncremental = (qobject_cast<TaskFetchJob *>(job)->fetchOnlyUpdated() > 0);
    qCDebug(GOOGLE_TASKS_LOG) << "Retrieved" << objects.count() << "tasks for list" << collection.remoteId();
    for (const auto &object : objects) {
        const TaskPtr task = object.dynamicCast<Task>();

        Item item;
        item.setMimeType(mimeType());
        item.setParentCollection(collection);
        item.setRemoteId(task->uid());
        item.setRemoteRevision(task->etag());
        item.setPayload<KCalendarCore::Todo::Ptr>(task.dynamicCast<KCalendarCore::Todo>());

        if (task->deleted()) {
            qCDebug(GOOGLE_TASKS_LOG) << " - removed" << task->uid();
            removedItems << item;
        } else {
            qCDebug(GOOGLE_TASKS_LOG) << " - changed" << task->uid();
            changedItems << item;
        }
    }

    if (isIncremental) {
        m_iface->itemsRetrievedIncremental(changedItems, removedItems);
    } else {
        m_iface->itemsRetrieved(changedItems);
    }
    const QDateTime local(QDateTime::currentDateTime());
    const QDateTime UTC(local.toUTC());

    collection.setRemoteRevision(QString::number(UTC.toSecsSinceEpoch()));
    new CollectionModifyJob(collection, this);

    emitReadyStatus();
}

void TaskHandler::itemAdded(const Item &item, const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Adding event to calendar '%1'", collection.displayName()));
    TaskPtr task(new Task(*item.payload<KCalendarCore::Todo::Ptr>()));
    const QString parentRemoteId = task->relatedTo(KCalendarCore::Incidence::RelTypeParent);
    qCDebug(GOOGLE_TASKS_LOG) << "Task added to list" << collection.remoteId() << "with parent" << parentRemoteId;
    auto job = new TaskCreateJob(task, item.parentCollection().remoteId(), m_settings->accountPtr(), this);
    job->setParentItem(parentRemoteId);
    connect(job, &TaskCreateJob::finished, this, [this, item](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        Item newItem = item;
        const TaskPtr task = qobject_cast<TaskCreateJob *>(job)->items().first().dynamicCast<Task>();
        qCDebug(GOOGLE_TASKS_LOG) << "Task added";
        newItem.setRemoteId(task->uid());
        newItem.setRemoteRevision(task->etag());
        newItem.setGid(task->uid());
        m_iface->itemChangeCommitted(newItem);
        newItem.setPayload<KCalendarCore::Todo::Ptr>(task.dynamicCast<KCalendarCore::Todo>());
        new ItemModifyJob(newItem, this);
        emitReadyStatus();
    });
}

void TaskHandler::itemChanged(const Item &item, const QSet<QByteArray> & /*partIdentifiers*/)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Changing task in list '%1'", item.parentCollection().displayName()));
    qCDebug(GOOGLE_TASKS_LOG) << "Changing task" << item.remoteId();

    auto todo = item.payload<KCalendarCore::Todo::Ptr>();
    const QString parentUid = todo->relatedTo(KCalendarCore::Incidence::RelTypeParent);
    // First we move it to a new parent, if there is
    auto job = new TaskMoveJob(item.remoteId(), item.parentCollection().remoteId(), parentUid, m_settings->accountPtr(), this);
    connect(job, &TaskMoveJob::finished, this, [this, todo, item](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }
        TaskPtr task(new Task(*todo));
        auto newJob = new TaskModifyJob(task, item.parentCollection().remoteId(), job->account(), this);
        newJob->setProperty(ITEM_PROPERTY, QVariant::fromValue(item));
        connect(newJob, &TaskModifyJob::finished, this, &TaskHandler::slotGenericJobFinished);
    });
}

void TaskHandler::itemsRemoved(const Item::List &items)
{
    m_iface->emitStatus(AgentBase::Running, i18ncp("@info:status", "Removing %1 task", "Removing %1 tasks", items.count()));
    qCDebug(GOOGLE_TASKS_LOG) << "Removing" << items.count() << "tasks";
    /* Google always automatically removes tasks with all their subtasks. In KOrganizer
     * by default we only remove the item we are given. For this reason we have to first
     * fetch all tasks, find all sub-tasks for the task being removed and detach them
     * from the task. Only then the task can be safely removed. */
    auto job = new ItemFetchJob(items.first().parentCollection());
    job->fetchScope().fetchFullPayload(true);
    connect(job, &ItemFetchJob::finished, this, [this, items](KJob *job) {
        if (job->error()) {
            m_iface->cancelTask(i18n("Failed to delete task: %1", job->errorString()));
            return;
        }
        const Item::List fetchedItems = qobject_cast<ItemFetchJob *>(job)->items();
        Item::List detachItems;
        TasksList detachTasks;
        for (const Item &fetchedItem : fetchedItems) {
            auto todo = fetchedItem.payload<KCalendarCore::Todo::Ptr>();
            TaskPtr task(new Task(*todo));
            const QString parentId = task->relatedTo(KCalendarCore::Incidence::RelTypeParent);
            if (parentId.isEmpty()) {
                continue;
            }

            auto it = std::find_if(items.cbegin(), items.cend(), [&parentId](const Item &item) {
                return item.remoteId() == parentId;
            });
            if (it != items.cend()) {
                Item newItem(fetchedItem);
                qCDebug(GOOGLE_TASKS_LOG) << "Detaching child" << newItem.remoteId() << "from" << parentId;
                todo->setRelatedTo(QString(), KCalendarCore::Incidence::RelTypeParent);
                newItem.setPayload<KCalendarCore::Todo::Ptr>(todo);
                detachItems << newItem;
                detachTasks << task;
            }
        }
        /* If there are no items do detach, then delete the task right now */
        if (detachItems.isEmpty()) {
            doRemoveTasks(items);
            return;
        }

        qCDebug(GOOGLE_TASKS_LOG) << "Reparenting" << detachItems.count() << "children...";
        auto moveJob = new TaskMoveJob(detachTasks, items.first().parentCollection().remoteId(), QString(), m_settings->accountPtr(), this);
        connect(moveJob, &TaskMoveJob::finished, this, [this, items, detachItems](KGAPI2::Job *job) {
            if (job->error()) {
                m_iface->cancelTask(i18n("Failed to reparent subtasks: %1", job->errorString()));
                return;
            }
            // Update items inside Akonadi DB too
            new ItemModifyJob(detachItems);
            // Perform actual removal
            doRemoveTasks(items);
        });
    });
}

void TaskHandler::doRemoveTasks(const Item::List &items)
{
    // Make sure account is still valid
    if (!m_iface->canPerformTask()) {
        return;
    }
    QStringList taskIds;
    taskIds.reserve(items.count());
    std::transform(items.cbegin(), items.cend(), std::back_inserter(taskIds), [](const Item &item) {
        return item.remoteId();
    });

    /* Now finally we can safely remove the task we wanted to */
    auto job = new TaskDeleteJob(taskIds, items.first().parentCollection().remoteId(), m_settings->accountPtr(), this);
    job->setProperty(ITEMS_PROPERTY, QVariant::fromValue(items));
    connect(job, &TaskDeleteJob::finished, this, &TaskHandler::slotGenericJobFinished);
}

void TaskHandler::itemsMoved(const Item::List & /*item*/, const Collection & /*collectionSource*/, const Collection & /*collectionDestination*/)
{
    m_iface->cancelTask(i18n("Moving tasks between task lists is not supported"));
}

void TaskHandler::collectionAdded(const Collection &collection, const Collection & /*parent*/)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Creating new task list '%1'", collection.displayName()));
    qCDebug(GOOGLE_TASKS_LOG) << "Adding task list" << collection.displayName();
    TaskListPtr taskList(new TaskList());
    taskList->setTitle(collection.displayName());

    auto job = new TaskListCreateJob(taskList, m_settings->accountPtr(), this);
    connect(job, &TaskListCreateJob::finished, this, [this, collection](KGAPI2::Job *job) {
        if (!m_iface->handleError(job)) {
            return;
        }

        TaskListPtr taskList = qobject_cast<TaskListCreateJob *>(job)->items().first().dynamicCast<TaskList>();
        qCDebug(GOOGLE_TASKS_LOG) << "Task list created:" << taskList->uid();
        // Enable newly added task list in settings
        m_settings->addTaskList(taskList->uid());
        // Populate remoteId & other stuff
        Collection newCollection(collection);
        setupCollection(newCollection, taskList);
        m_iface->collectionChangeCommitted(newCollection);
        emitReadyStatus();
    });
}

void TaskHandler::collectionChanged(const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Changing task list '%1'", collection.displayName()));
    qCDebug(GOOGLE_TASKS_LOG) << "Changing task list" << collection.remoteId();

    TaskListPtr taskList(new TaskList());
    taskList->setUid(collection.remoteId());
    taskList->setTitle(collection.displayName());
    auto job = new TaskListModifyJob(taskList, m_settings->accountPtr(), this);
    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &TaskListModifyJob::finished, this, &TaskHandler::slotGenericJobFinished);
}

void TaskHandler::collectionRemoved(const Collection &collection)
{
    m_iface->emitStatus(AgentBase::Running, i18nc("@info:status", "Removing task list '%1'", collection.displayName()));
    qCDebug(GOOGLE_TASKS_LOG) << "Removing task list" << collection.remoteId();
    auto job = new TaskListDeleteJob(collection.remoteId(), m_settings->accountPtr(), this);
    job->setProperty(COLLECTION_PROPERTY, QVariant::fromValue(collection));
    connect(job, &TaskListDeleteJob::finished, this, &TaskHandler::slotGenericJobFinished);
}
