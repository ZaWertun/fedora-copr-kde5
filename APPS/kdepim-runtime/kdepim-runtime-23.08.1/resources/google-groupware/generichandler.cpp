/*
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "generichandler.h"
#include "googleresource_debug.h"

#include <Akonadi/AgentBase>

#include <KGAPI/Job>

GenericHandler::GenericHandler(GoogleResourceStateInterface *iface, GoogleSettings *settings)
    : m_iface(iface)
    , m_settings(settings)
{
}

GenericHandler::~GenericHandler() = default;

void GenericHandler::itemsLinked(const Akonadi::Item::List & /*items*/, const Akonadi::Collection & /*collection*/)
{
    m_iface->cancelTask(i18n("Cannot handle item linking"));
}

void GenericHandler::itemsUnlinked(const Akonadi::Item::List & /*items*/, const Akonadi::Collection & /*collection*/)
{
    m_iface->cancelTask(i18n("Cannot handle item unlinking"));
}

void GenericHandler::slotGenericJobFinished(KGAPI2::Job *job)
{
    if (!m_iface->handleError(job)) {
        return;
    }
    if (job->property(ITEM_PROPERTY).isValid()) {
        qCDebug(GOOGLE_LOG) << "Item change committed";
        m_iface->itemChangeCommitted(job->property(ITEM_PROPERTY).value<Akonadi::Item>());
    } else if (job->property(ITEMS_PROPERTY).isValid()) {
        qCDebug(GOOGLE_LOG) << "Items changes committed";
        m_iface->itemsChangesCommitted(job->property(ITEMS_PROPERTY).value<Akonadi::Item::List>());
    } else if (job->property(COLLECTION_PROPERTY).isValid()) {
        qCDebug(GOOGLE_LOG) << "Collection change committed";
        m_iface->collectionChangeCommitted(job->property(COLLECTION_PROPERTY).value<Akonadi::Collection>());
    } else {
        qCDebug(GOOGLE_LOG) << "Task done";
        m_iface->taskDone();
    }

    emitReadyStatus();
}

void GenericHandler::emitReadyStatus()
{
    m_iface->emitStatus(Akonadi::AgentBase::Idle, i18nc("@status", "Ready"));
}

#include "moc_generichandler.cpp"
