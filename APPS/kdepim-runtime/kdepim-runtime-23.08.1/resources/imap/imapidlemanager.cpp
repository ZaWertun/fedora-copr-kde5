/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapidlemanager.h"

#include "imapresource_debug.h"

#include <KIMAP/IdleJob>
#include <KIMAP/SelectJob>
#include <KIMAP/Session>

#include <QTimer>

#include "imapresource.h"
#include "sessionpool.h"

#include <chrono>

namespace
{
// RFC2177 says clients should restart IDLE every 29 minutes, as
// servers MAY consider clients inactive after 30 minutes.
// TODO: Make configurable to support less RF-conformant servers
static const auto IdleTimeout = std::chrono::minutes(29);
}

ImapIdleManager::ImapIdleManager(ResourceStateInterface::Ptr state, SessionPool *pool, ImapResourceBase *parent)
    : QObject(parent)
    , m_sessionRequestId(0)
    , m_pool(pool)
    , m_resource(parent)
    , m_state(state)
    , m_lastMessageCount(-1)
    , m_lastRecentCount(-1)
{
    connect(pool, &SessionPool::sessionRequestDone, this, &ImapIdleManager::onSessionRequestDone);
    m_sessionRequestId = m_pool->requestSession();

    m_idleTimeout = new QTimer(this);
    m_idleTimeout->setSingleShot(true);
    connect(m_idleTimeout, &QTimer::timeout, this, &ImapIdleManager::restartIdle);
}

ImapIdleManager::~ImapIdleManager()
{
    stop();
    if (m_pool) {
        if (m_sessionRequestId) {
            m_pool->cancelSessionRequest(m_sessionRequestId);
        }
        if (m_session) {
            m_pool->releaseSession(m_session);
        }
    }
}

void ImapIdleManager::stop()
{
    m_idleTimeout->stop();
    if (m_idle) {
        m_idle->stop();
        disconnect(m_idle, nullptr, this, nullptr);
        m_idle = nullptr;
    }
    if (m_pool) {
        disconnect(m_pool, nullptr, this, nullptr);
    }
}

KIMAP::Session *ImapIdleManager::session() const
{
    return m_session;
}

void ImapIdleManager::reconnect()
{
    qCDebug(IMAPRESOURCE_LOG) << "attempting to reconnect IDLE session";
    if (m_session == nullptr && m_pool->isConnected() && m_sessionRequestId == 0) {
        m_sessionRequestId = m_pool->requestSession();
    }
}

void ImapIdleManager::onSessionRequestDone(qint64 requestId, KIMAP::Session *session, int errorCode, const QString & /*errorString*/)
{
    if (requestId != m_sessionRequestId || session == nullptr || errorCode != SessionPool::NoError) {
        return;
    }

    m_session = session;
    m_sessionRequestId = 0;

    connect(m_pool, &SessionPool::connectionLost, this, &ImapIdleManager::onConnectionLost);
    connect(m_pool, &SessionPool::disconnectDone, this, &ImapIdleManager::onPoolDisconnect);

    startIdle();
}

void ImapIdleManager::startIdle()
{
    const auto idleMailBox = m_state->mailBoxForCollection(m_state->collection());
    if (m_session->selectedMailBox() != idleMailBox) {
        auto select = new KIMAP::SelectJob(m_session);
        select->setMailBox(idleMailBox);
        connect(select, &KIMAP::SelectJob::result, this, &ImapIdleManager::onSelectDone);
        select->start();
    }

    m_idle = new KIMAP::IdleJob(m_session);
    connect(m_idle.data(), &KIMAP::IdleJob::mailBoxStats, this, &ImapIdleManager::onStatsReceived);
    connect(m_idle.data(), &KIMAP::IdleJob::mailBoxMessageFlagsChanged, this, &ImapIdleManager::onFlagsChanged);
    connect(m_idle.data(), &KIMAP::IdleJob::result, this, &ImapIdleManager::onIdleStopped);
    m_idle->start();
    m_idleTimeout->start(std::chrono::milliseconds(IdleTimeout).count());
}

void ImapIdleManager::restartIdle()
{
    qCDebug(IMAPRESOURCE_LOG) << "Restarting IDLE to prevent server from disconnecting me!";
    if (m_idle) {
        m_idle->stop(); // this will invoke onIdleStopped(), which will automatically reconnect
    }
}

void ImapIdleManager::onConnectionLost(KIMAP::Session *session)
{
    if (session == m_session) {
        // Our session becomes invalid, so get ride of
        // the pointer, we don't need to release it once the
        // task is done
        m_session = nullptr;
        QMetaObject::invokeMethod(this, &ImapIdleManager::reconnect, Qt::QueuedConnection);
    }
}

void ImapIdleManager::onPoolDisconnect()
{
    // All the sessions in the pool we used changed,
    // so get ride of the pointer, we don't need to
    // release our session anymore
    m_pool = nullptr;
}

void ImapIdleManager::onSelectDone(KJob *job)
{
    auto select = static_cast<KIMAP::SelectJob *>(job);

    m_lastMessageCount = select->messageCount();
    m_lastRecentCount = select->recentCount();
}

void ImapIdleManager::onIdleStopped()
{
    qCDebug(IMAPRESOURCE_LOG) << "IDLE dropped maybe we should reconnect?";
    m_idle = nullptr;
    if (m_session) {
        qCDebug(IMAPRESOURCE_LOG) << "Restarting the IDLE session!";
        startIdle();
    }
}

void ImapIdleManager::onStatsReceived(KIMAP::IdleJob *job, const QString &mailBox, int messageCount, int recentCount)
{
    qCDebug(IMAPRESOURCE_LOG) << "IDLE stats received:" << job << mailBox << messageCount << recentCount;
    qCDebug(IMAPRESOURCE_LOG) << "Cached information:" << m_state->collection().remoteId() << m_state->collection().id() << m_lastMessageCount
                              << m_lastRecentCount;

    // It seems we're not in sync with the cache, resync is needed
    if (messageCount != m_lastMessageCount || recentCount != m_lastRecentCount) {
        m_lastMessageCount = messageCount;
        m_lastRecentCount = recentCount;

        qCDebug(IMAPRESOURCE_LOG) << "Resync needed for" << mailBox << m_state->collection().id();
        m_resource->synchronizeCollection(m_state->collection().id());
    }
}

void ImapIdleManager::onFlagsChanged(KIMAP::IdleJob *job)
{
    Q_UNUSED(job)
    qCDebug(IMAPRESOURCE_LOG) << "IDLE flags changed in" << m_session->selectedMailBox();
    m_resource->synchronizeCollection(m_state->collection().id());
}
