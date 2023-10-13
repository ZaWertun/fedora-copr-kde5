/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcestateinterface.h"

#include <Akonadi/Collection>

#include <QObject>
#include <QPointer>

namespace KIMAP
{
class IdleJob;
class Session;
}

class ImapResourceBase;
class SessionPool;

class KJob;

class QTimer;

class ImapIdleManager : public QObject
{
    Q_OBJECT

public:
    ImapIdleManager(ResourceStateInterface::Ptr state, SessionPool *pool, ImapResourceBase *parent);
    ~ImapIdleManager() override;
    void stop();

    KIMAP::Session *session() const;

private Q_SLOTS:
    void onConnectionLost(KIMAP::Session *session);
    void onPoolDisconnect();

    void onSessionRequestDone(qint64 requestId, KIMAP::Session *session, int errorCode, const QString &errorString);
    void onSelectDone(KJob *job);
    void onIdleStopped();
    void onStatsReceived(KIMAP::IdleJob *job, const QString &mailBox, int messageCount, int recentCount);
    void onFlagsChanged(KIMAP::IdleJob *job);
    void reconnect();
    void restartIdle();

private:
    void startIdle();

    qint64 m_sessionRequestId;
    SessionPool *m_pool = nullptr;
    KIMAP::Session *m_session = nullptr;
    QPointer<KIMAP::IdleJob> m_idle;
    ImapResourceBase *m_resource = nullptr;
    ResourceStateInterface::Ptr m_state;
    QTimer *m_idleTimeout = nullptr;
    qint64 m_lastMessageCount = -1;
    qint64 m_lastRecentCount = -1;
};
