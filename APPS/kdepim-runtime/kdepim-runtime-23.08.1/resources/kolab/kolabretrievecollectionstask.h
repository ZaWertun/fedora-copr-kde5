/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <resourcetask.h>

#include <Akonadi/Collection>

#include <QElapsedTimer>
#include <kimap/listjob.h>
class KolabRetrieveCollectionsTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit KolabRetrieveCollectionsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~KolabRetrieveCollectionsTask() override;

private Q_SLOTS:
    void onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);
    void onMailBoxesReceiveDone(KJob *job);
    void onFullMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);
    void onFullMailBoxesReceiveDone(KJob *job);
    void onMetadataRetrieved(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void checkDone();
    Akonadi::Collection getOrCreateParent(const QString &parentPath);
    void createCollection(const QString &mailbox, const QList<QByteArray> &flags, bool isSubscribed);
    void setAttributes(Akonadi::Collection &c, const QStringList &pathParts, const QString &path);
    void applyRights(const QHash<QString, KIMAP::Acl::Rights> &rights);
    void applyMetadata(const QHash<QString, QMap<QByteArray, QByteArray>> &metadata);

    int mJobs = 0;
    QHash<QString, Akonadi::Collection> mMailCollections;
    QSet<QString> mSubscribedMailboxes;
    QSet<QByteArray> mRequestedMetadata;
    KIMAP::Session *mSession = nullptr;
    QElapsedTimer mTime;
    // For implicit sharing
    const QByteArray cContentMimeTypes;
    const QByteArray cAccessRights;
    const QByteArray cImapAcl;
    const QByteArray cCollectionAnnotations;
    const QSet<QByteArray> cDefaultKeepLocalChanges;
    const QStringList cDefaultMimeTypes;
    const QStringList cCollectionOnlyContentMimeTypes;
};

class RetrieveMetadataJob : public KJob
{
    Q_OBJECT
public:
    RetrieveMetadataJob(KIMAP::Session *session,
                        const QStringList &mailboxes,
                        const QStringList &serverCapabilities,
                        const QSet<QByteArray> &requestedMetadata,
                        const QString &separator,
                        const QList<KIMAP::MailBoxDescriptor> &sharedNamespace,
                        const QList<KIMAP::MailBoxDescriptor> &userNamespace,
                        QObject *parent = nullptr);
    void start() override;

    QHash<QString, QMap<QByteArray, QByteArray>> mMetadata;
    QHash<QString, KIMAP::Acl::Rights> mRights;

private:
    void checkDone();
    int mJobs = 0;
    QSet<QByteArray> mRequestedMetadata;
    QStringList mServerCapabilities;
    QStringList mMailboxes;
    KIMAP::Session *mSession = nullptr;
    QString mSeparator;
    QList<KIMAP::MailBoxDescriptor> mSharedNamespace;
    QList<KIMAP::MailBoxDescriptor> mUserNamespace;

private Q_SLOTS:
    void onGetMetaDataDone(KJob *job);
    void onRightsReceived(KJob *job);
};
