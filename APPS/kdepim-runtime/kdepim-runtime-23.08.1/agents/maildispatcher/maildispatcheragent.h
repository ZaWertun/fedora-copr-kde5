/*
    SPDX-FileCopyrightText: 2008 Ingo Klöcker <kloecker@kde.org>
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/AgentBase>
#include <Akonadi/Item>

class OutboxQueue;
class SendJob;
class SentActionHandler;
/**
 * @short This agent dispatches mail put into the outbox collection.
 */
class MailDispatcherAgent : public Akonadi::AgentBase
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Akonadi.MailDispatcherAgent")

public:
    explicit MailDispatcherAgent(const QString &id);
    ~MailDispatcherAgent() override;

Q_SIGNALS:
    /**
     * Emitted when the MDA has attempted to send an item.
     */
    void itemProcessed(const Akonadi::Item &item, bool result);

    /**
     * Emitted when the MDA has begun processing an item
     */
    Q_SCRIPTABLE void itemDispatchStarted();

protected:
    void doSetOnline(bool online) override;

private Q_SLOTS:
    void sendPercent(KJob *job, unsigned long percent);

private:
    // Q_SLOTS:
    void abort();
    void dispatch();
    void itemFetched(const Akonadi::Item &item);
    void queueError(const QString &message);
    void sendResult(KJob *job);
    void emitStatusReady();

    OutboxQueue *const mQueue;
    SentActionHandler *const mSentActionHandler;
    SendJob *mCurrentJob = nullptr;
    Akonadi::Item mCurrentItem;
    bool mAborting = false;
    bool mSendingInProgress = false;
    bool mSentAnything = false;
    bool mErrorOccurred = false;
    bool mShowSentNotification = true;
    qulonglong mSentItemsSize = 0;
};
