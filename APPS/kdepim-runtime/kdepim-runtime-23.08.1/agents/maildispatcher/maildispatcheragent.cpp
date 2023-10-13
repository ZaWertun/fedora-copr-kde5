/*
    SPDX-FileCopyrightText: 2008 Ingo Klöcker <kloecker@kde.org>
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "maildispatcheragent.h"

#include "maildispatcheragentadaptor.h"
#include "outboxqueue.h"
#include "sendjob.h"
#include "sentactionhandler.h"
#include "settings.h"
#include "settingsadaptor.h"

#include <Akonadi/ItemFetchScope>
#include <Akonadi/SentActionAttribute>
#include <Akonadi/SentBehaviourAttribute>
#include <Akonadi/ServerManager>

#include "maildispatcher_debug.h"
#include <KLocalizedString>
#include <KMime/Message>
#include <KNotification>
#include <chrono>

using namespace std::chrono_literals;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif

#include <QDBusConnection>
#include <QTimer>

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
#endif

using namespace Akonadi;

void MailDispatcherAgent::abort()
{
    if (!isOnline()) {
        qCDebug(MAILDISPATCHER_LOG) << "Offline. Ignoring call.";
        return;
    }

    if (mAborting) {
        qCDebug(MAILDISPATCHER_LOG) << "Already aborting.";
        return;
    }

    if (!mSendingInProgress && mQueue->isEmpty()) {
        qCDebug(MAILDISPATCHER_LOG) << "MDA is idle.";
        Q_ASSERT(status() == AgentBase::Idle);
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Aborting...";
        mAborting = true;
        if (mCurrentJob) {
            mCurrentJob->abort();
        }
        // Further SendJobs will mark remaining items in the queue as 'aborted'.
    }
}

void MailDispatcherAgent::dispatch()
{
    Q_ASSERT(mQueue);

    if (!isOnline() || mSendingInProgress) {
        qCDebug(MAILDISPATCHER_LOG) << "Offline or busy. See you later.";
        return;
    }

    if (!mQueue->isEmpty()) {
        if (!mSentAnything) {
            mSentAnything = true;
            mSentItemsSize = 0;
            Q_EMIT percent(0);
        }
        Q_EMIT status(AgentBase::Running, i18np("Sending messages (1 item in queue)...", "Sending messages (%1 items in queue)...", mQueue->count()));
        qCDebug(MAILDISPATCHER_LOG) << "Attempting to dispatch the next message.";
        mSendingInProgress = true;
        mQueue->fetchOne(); // will trigger itemFetched
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Empty queue.";
        if (mAborting) {
            // Finished marking messages as 'aborted'.
            mAborting = false;
            mSentAnything = false;
            Q_EMIT status(AgentBase::Idle, i18n("Sending canceled."));
            QTimer::singleShot(3s, this, &MailDispatcherAgent::emitStatusReady);
        } else {
            if (mSentAnything) {
                // Finished sending messages in queue.
                mSentAnything = false;
                Q_EMIT percent(100);
                Q_EMIT status(AgentBase::Idle, i18n("Finished sending messages."));

                if (!mErrorOccurred && mShowSentNotification) {
                    auto notify = new KNotification(QStringLiteral("emailsent"));
                    notify->setIconName(QStringLiteral("kmail"));
                    notify->setComponentName(QStringLiteral("akonadi_maildispatcher_agent"));
                    notify->setTitle(i18nc("Notification title when email was sent", "E-Mail Successfully Sent"));
                    notify->setText(i18nc("Notification when the email was sent", "Your E-Mail has been sent successfully."));
                    notify->sendEvent();
                }
                mShowSentNotification = true;
            } else {
                // Empty queue.
                Q_EMIT status(AgentBase::Idle, i18n("No items in queue."));
            }
            QTimer::singleShot(3s, this, &MailDispatcherAgent::emitStatusReady);
        }

        mErrorOccurred = false;
    }
}

MailDispatcherAgent::MailDispatcherAgent(const QString &id)
    : AgentBase(id)
    , mQueue(new OutboxQueue(this))
    , mSentActionHandler(new SentActionHandler(this))
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("maildispatcheragent"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("maildispatcheragentrc") << QStringLiteral("akonadi_maildispatcher_agent.notifyrc"));
    migrate.migrate();
#endif
    qCDebug(MAILDISPATCHER_LOG) << "maildispatcheragent: At your service, sir!";

    new SettingsAdaptor(Settings::self());
    new MailDispatcherAgentAdaptor(this);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"), Settings::self(), QDBusConnection::ExportAdaptors);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/MailDispatcherAgent"), this, QDBusConnection::ExportAdaptors);
    QString service = QStringLiteral("org.freedesktop.Akonadi.MailDispatcherAgent");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    }

    QDBusConnection::sessionBus().registerService(service);

    connect(mQueue, &OutboxQueue::newItems, this, &MailDispatcherAgent::dispatch);
    connect(mQueue, &OutboxQueue::itemReady, this, &MailDispatcherAgent::itemFetched);
    connect(mQueue, &OutboxQueue::error, this, &MailDispatcherAgent::queueError);
    connect(this, &MailDispatcherAgent::itemProcessed, mQueue, &OutboxQueue::itemProcessed);
    connect(this, &MailDispatcherAgent::abortRequested, this, &MailDispatcherAgent::abort);

    setNeedsNetwork(true);
}

MailDispatcherAgent::~MailDispatcherAgent() = default;

void MailDispatcherAgent::doSetOnline(bool online)
{
    Q_ASSERT(mQueue);
    if (online) {
        qCDebug(MAILDISPATCHER_LOG) << "Online. Dispatching messages.";
        Q_EMIT status(AgentBase::Idle, i18n("Online, sending messages in queue."));
        QTimer::singleShot(0, this, &MailDispatcherAgent::dispatch);
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Offline.";
        Q_EMIT status(AgentBase::Idle, i18n("Offline, message sending suspended."));

        // TODO: This way, the OutboxQueue will continue to react to changes in
        // the outbox, but the MDA will just not send anything.  Is this what we
        // want?
    }

    AgentBase::doSetOnline(online);
}

void MailDispatcherAgent::itemFetched(const Item &item)
{
    qCDebug(MAILDISPATCHER_LOG) << "Fetched item" << item.id() << "; creating SendJob.";
    Q_ASSERT(mSendingInProgress);
    Q_ASSERT(!mCurrentItem.isValid());
    mCurrentItem = item;
    Q_ASSERT(mCurrentJob == nullptr);
    Q_EMIT itemDispatchStarted();

    mCurrentJob = new SendJob(item, this);
    if (mAborting) {
        mCurrentJob->setMarkAborted();
    }

    Q_EMIT status(AgentBase::Running,
                  i18nc("Message with given subject is being sent.", "Sending: %1", item.payload<KMime::Message::Ptr>()->subject()->asUnicodeString()));

    connect(mCurrentJob, &KJob::result, this, &MailDispatcherAgent::sendResult);
    connect(mCurrentJob, &SendJob::percentChanged, this, &MailDispatcherAgent::sendPercent);

    mCurrentJob->start();
}

void MailDispatcherAgent::queueError(const QString &message)
{
    Q_EMIT error(message);
    mErrorOccurred = true;
    // FIXME figure out why this does not set the status to Broken, etc.
}

void MailDispatcherAgent::sendPercent(KJob *job, unsigned long)
{
    Q_ASSERT(mSendingInProgress);
    Q_ASSERT(job == mCurrentJob);
    // The progress here is actually the TransportJob, not the entire SendJob,
    // because the post-job doesn't report progress.  This should be fine,
    // since the TransportJob is the lengthiest operation.

    // Give the transport 80% of the weight, and move-to-sendmail 20%.
    const double transportWeight = 0.8;

    const int percentValue =
        100 * (mSentItemsSize + job->processedAmount(KJob::Bytes) * transportWeight) / (mSentItemsSize + mCurrentItem.size() + mQueue->totalSize());

    qCDebug(MAILDISPATCHER_LOG) << "sentItemsSize" << mSentItemsSize << "this job processed" << job->processedAmount(KJob::Bytes) << "queue totalSize"
                                << mQueue->totalSize() << "total total size (sent+current+queue)"
                                << (mSentItemsSize + mCurrentItem.size() + mQueue->totalSize()) << "new percentage" << percentValue << "old percentage"
                                << progress();

    if (percentValue != progress()) {
        // The progress can decrease too, if messages got added to the queue.
        Q_EMIT percent(percentValue);
    }

    // It is possible that the number of queued messages has changed.
    Q_EMIT status(AgentBase::Running, i18np("Sending messages (1 item in queue)...", "Sending messages (%1 items in queue)...", 1 + mQueue->count()));
}

void MailDispatcherAgent::sendResult(KJob *job)
{
    Q_ASSERT(mSendingInProgress);
    Q_ASSERT(job == mCurrentJob);
    mCurrentJob->disconnect(this);
    mCurrentJob = nullptr;

    Q_ASSERT(mCurrentItem.isValid());
    mSentItemsSize += mCurrentItem.size();
    Q_EMIT itemProcessed(mCurrentItem, !job->error());

    const Akonadi::Item sentItem = mCurrentItem;
    mCurrentItem = Item();

    if (job->error()) {
        // The SendJob gave the item an ErrorAttribute, so we don't have to
        // do anything.
        qCDebug(MAILDISPATCHER_LOG) << "Sending failed. error:" << job->errorString();

        auto notify = new KNotification(QStringLiteral("sendingfailed"));
        notify->setComponentName(QStringLiteral("akonadi_maildispatcher_agent"));
        notify->setIconName(QStringLiteral("kmail"));
        notify->setTitle(i18nc("Notification title when email sending failed", "E-Mail Sending Failed"));
        notify->setText(job->errorString().toHtmlEscaped());
        notify->sendEvent();

        mErrorOccurred = true;
    } else {
        qCDebug(MAILDISPATCHER_LOG) << "Sending succeeded.";

        // handle possible sent actions
        const auto attribute = sentItem.attribute<Akonadi::SentActionAttribute>();
        if (attribute) {
            const Akonadi::SentActionAttribute::Action::List lstAct = attribute->actions();
            for (const Akonadi::SentActionAttribute::Action &action : lstAct) {
                mSentActionHandler->runAction(action);
            }
        }
        const auto bhAttribute = sentItem.attribute<Akonadi::SentBehaviourAttribute>();
        if (bhAttribute) {
            mShowSentNotification = !bhAttribute->sendSilently();
        } else {
            mShowSentNotification = true;
        }
    }

    // dispatch next message
    mSendingInProgress = false;
    QTimer::singleShot(0, this, &MailDispatcherAgent::dispatch);
}

void MailDispatcherAgent::emitStatusReady()
{
    if (status() == AgentBase::Idle) {
        // If still idle after aborting, clear 'aborted' status.
        Q_EMIT status(AgentBase::Idle, i18n("Ready to dispatch messages."));
    }
}

#ifndef KDEPIM_PLUGIN_AGENT
AKONADI_AGENT_MAIN(MailDispatcherAgent)
#endif

#include "moc_maildispatcheragent.cpp"
