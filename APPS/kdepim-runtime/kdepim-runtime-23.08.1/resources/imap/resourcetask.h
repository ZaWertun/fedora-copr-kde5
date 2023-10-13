/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KIMAP/Acl>

#include "resourcestateinterface.h"

namespace KIMAP
{
class Session;
}

class SessionPool;

class ResourceTask : public QObject
{
    Q_OBJECT

public:
    enum ActionIfNoSession {
        CancelIfNoSession,
        DeferIfNoSession,
    };
    Q_ENUM(ActionIfNoSession)

    explicit ResourceTask(ActionIfNoSession action, ResourceStateInterface::Ptr resource, QObject *parent = nullptr);
    ~ResourceTask() override;

    void start(SessionPool *pool);

    void kill();

    static QList<QByteArray> fromAkonadiToSupportedImapFlags(const QList<QByteArray> &flags, const Akonadi::Collection &collection);

    static QSet<QByteArray> toAkonadiFlags(const QList<QByteArray> &flags);

Q_SIGNALS:
    void status(int status, const QString &message = QString());

protected:
    virtual void doStart(KIMAP::Session *session) = 0;

protected:
    QString userName() const;
    QString resourceName() const;
    QStringList serverCapabilities() const;
    QList<KIMAP::MailBoxDescriptor> serverNamespaces() const;

    bool isAutomaticExpungeEnabled() const;
    bool isSubscriptionEnabled() const;
    bool isDisconnectedModeEnabled() const;
    int intervalCheckTime() const;

    Akonadi::Collection collection() const;
    Akonadi::Item item() const;
    Akonadi::Item::List items() const;

    Akonadi::Collection parentCollection() const;

    Akonadi::Collection sourceCollection() const;
    Akonadi::Collection targetCollection() const;

    QSet<QByteArray> parts() const;
    QSet<QByteArray> addedFlags() const;
    QSet<QByteArray> removedFlags() const;

    QString rootRemoteId() const;
    QString mailBoxForCollection(const Akonadi::Collection &collection) const;

    void setIdleCollection(const Akonadi::Collection &collection);
    void applyCollectionChanges(const Akonadi::Collection &collection);

    void itemRetrieved(const Akonadi::Item &item);

    void itemsRetrieved(const Akonadi::Item::List &items);
    void itemsRetrievedIncremental(const Akonadi::Item::List &changed, const Akonadi::Item::List &removed);
    void itemsRetrievalDone();

    void setTotalItems(int);

    void changeCommitted(const Akonadi::Item &item);
    void changesCommitted(const Akonadi::Item::List &items);

    void collectionsRetrieved(const Akonadi::Collection::List &collections);

    void collectionAttributesRetrieved(const Akonadi::Collection &col);

    void changeCommitted(const Akonadi::Collection &collection);

    void changeCommitted(const Akonadi::Tag &tag);

    void changeProcessed();

    void searchFinished(const QVector<qint64> &result, bool isRid = true);

    void cancelTask(const QString &errorString);
    void deferTask();
    void restartItemRetrieval(Akonadi::Collection::Id col);
    void taskDone();
    void emitPercent(int percent);
    void emitError(const QString &message);
    void emitWarning(const QString &message);

    void synchronizeCollectionTree();

    void showInformationDialog(const QString &message, const QString &title, const QString &dontShowAgainName);

    const QChar separatorCharacter() const;
    void setSeparatorCharacter(QChar separator);

    virtual bool serverSupportsAnnotations() const;
    virtual bool serverSupportsCondstore() const;

    int batchSize() const;
    void setItemMergingMode(Akonadi::ItemSync::MergeMode mode);

    ResourceStateInterface::Ptr resourceState();

    KIMAP::Acl::Rights myRights(const Akonadi::Collection &);

private:
    void abortTask(const QString &errorString);

    static QList<QByteArray> fromAkonadiFlags(const QList<QByteArray> &flags);

private Q_SLOTS:
    void onSessionRequested(qint64 requestId, KIMAP::Session *session, int errorCode, const QString &errorString);
    void onConnectionLost(KIMAP::Session *session);
    void onPoolDisconnect();

private:
    SessionPool *m_pool = nullptr;
    qint64 m_sessionRequestId = 0;

    KIMAP::Session *m_session = nullptr;
    ActionIfNoSession m_actionIfNoSession;
    ResourceStateInterface::Ptr m_resource;
    bool mCancelled = false;
};
