/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSet>
#include <QString>
#include <QTimer>

#include "ewsid.h"

class EwsClient;
class KJob;
class EwsEventRequestBase;
class EwsSettings;

/**
 *  @brief  Mailbox update subscription manager class
 *
 *  This class is responsible for retrieving update notifications from the Exchange server.
 *
 *  The Exchange server has the ability to incrementally inform the client about changes made to
 *  selected folders in the mailbox. Each update informs about creation, modification or removal
 *  of an item or folder. Additionally Exchange has the ability to notify about free/busy status
 *  updates.
 *
 *  Notifications can be delivered in 3 ways:
 *   - pull (i.e. polling) - the client needs to periodically question the server.
 *   - push - the server issues a callback connection to the client with events (not supported)
 *   - streaming - a combination of pull and push, where the client makes the connection, but the
 *                 server keeps it open for a specified period of time and keeps delivering events
 *                 over this connection (supported since Exchange 2010 SP2).
 *
 *  The responsibility of this class is to retrieve and act upon change events from the Exchange
 *  server. The current implementation is simplified:
 *   - when an item update is received the folder containing the update is asked to synchronize
 *     itself.
 *   - when a folder update is received a full collection tree sync is performed.
 *
 *  The above implementation has a major drawback in that operations performed by the resource
 *  itself are also notified back as update events. This means that when for ex. an item is deleted
 *  it is removed from Akonadi database, but subsequently a delete event is received which will try
 *  to delete an item that has already been deleted from Akonadi.
 */
class EwsSubscriptionManager : public QObject
{
    Q_OBJECT
public:
    EwsSubscriptionManager(EwsClient &client, const EwsId &rootId, EwsSettings *settings, QObject *parent);
    ~EwsSubscriptionManager() override;
    void start();
Q_SIGNALS:
    void foldersModified(EwsId::List folders);
    void folderTreeModified();
    void fullSyncRequested();
    void connectionError();

private:
    void subscribeRequestFinished(KJob *job);
    void verifySubFoldersRequestFinished(KJob *job);
    void getEventsRequestFinished(KJob *job);
    void streamingEventsReceived(KJob *job);
    void getEvents();
    void streamingConnectionTimeout();

private:
    void cancelSubscription();
    void setupSubscription();
    void setupSubscriptionReq(const EwsId::List &ids);
    void reset();
    void resetSubscription();
    void processEvents(EwsEventRequestBase *req, bool finished);

    EwsClient &mEwsClient;
    QTimer mPollTimer;
    EwsId mMsgRootId;

    QSet<EwsId> mUpdatedFolderIds;
    bool mFolderTreeChanged;
    bool mStreamingEvents;
    QTimer mStreamingTimer;
    EwsEventRequestBase *mEventReq = nullptr;
    EwsSettings *mSettings = nullptr;
};
