/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <KJob>
namespace Akonadi
{
class Item;
class AgentInstance;
}
class QDBusInterface;
/**
 * @short A job to send a mail
 *
 * This class takes a prevalidated Item with all the required attributes,
 * sends it using MailTransport, and then stores the result of the sending
 * operation in the item.
 */
class SendJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new send job.
     *
     * @param mItem The item to send.
     * @param parent The parent object.
     */
    explicit SendJob(const Akonadi::Item &mItem, QObject *parent = nullptr);

    /**
     * Destroys the send job.
     */
    ~SendJob() override;

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * If this function is called before the job is started, the SendJob will
     * just mark the item as aborted, instead of sending it.
     * Do not call this function more than once.
     */
    void setMarkAborted();

    /**
     * Aborts sending the item.
     *
     * This will give the item an ErrorAttribute of "aborted".
     * (No need to call setMarkAborted() if you call abort().)
     */
    void abort();

private Q_SLOTS:
    void transportPercent(KJob *job, unsigned long percent);
    void resourceResult(qlonglong itemId, int result, const QString &message);

private:
    void doAkonadiTransport();
    void doTraditionalTransport();
    void doPostJob(bool transportSuccess, const QString &transportMessage);
    void storeResult(bool success, const QString &message = QString());
    void abortPostJob();
    Q_REQUIRED_RESULT bool filterItem(int filterset);

    // slots
    void doTransport();
    void transportResult(KJob *job);
    void resourceProgress(const Akonadi::AgentInstance &instance);
    void postJobResult(KJob *job);
    void doEmitResult(KJob *job);
    void slotSentMailCollectionFetched(KJob *job);

    Akonadi::Item mItem;
    QString mResourceId;
    KJob *mCurrentJob = nullptr;
    QDBusInterface *mInterface = nullptr;
    bool mAborting = false;
};
