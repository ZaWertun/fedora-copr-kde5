/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QObject;
#include <QString>

#include <Akonadi/SpecialMailCollections>

#include "fakeewsserver.h"

namespace Akonadi
{
class AgentInstance;
}
class FakeEwsServerThread;
class OrgKdeAkonadiEwsSettingsInterface;
class OrgKdeAkonadiEwsWalletInterface;
class OrgKdeAkonadiEwsResourceInterface;

class IsolatedTestBase : public QObject
{
    Q_OBJECT
public:
    class Folder
    {
    public:
        enum DistinguishedType {
            None,
            Root,
            Inbox,
            Outbox,
            Sent,
            Trash,
            Drafts,
            Templates,
            Calendar,
            Tasks,
            Contacts,
        };

        QString id;
        QString name;
        DistinguishedType type;
        QString parentId;
    };

    using FolderList = QVector<Folder>;

    explicit IsolatedTestBase(QObject *parent = nullptr);
    ~IsolatedTestBase() override;

    static QString loadResourceAsString(const QString &path);

protected:
    virtual void init();
    virtual void cleanup();

protected:
    QScopedPointer<FakeEwsServerThread> mFakeServerThread;
};

class TestAgentInstance : public QObject
{
    Q_OBJECT
public:
    TestAgentInstance(const QString &url);
    ~TestAgentInstance() override;

    const QString &identifier() const;

    bool setOnline(bool online, bool wait);
    bool isValid() const;

    OrgKdeAkonadiEwsSettingsInterface &settingsInterface() const;
    OrgKdeAkonadiEwsWalletInterface &walletInterface() const;
    OrgKdeAkonadiEwsResourceInterface &resourceInterface() const;
    Akonadi::AgentInstance &instance() const;

private:
    QScopedPointer<Akonadi::AgentInstance> mEwsInstance;
    QScopedPointer<OrgKdeAkonadiEwsSettingsInterface> mEwsSettingsInterface;
    QScopedPointer<OrgKdeAkonadiEwsWalletInterface> mEwsWalletInterface;
    QScopedPointer<OrgKdeAkonadiEwsResourceInterface> mEwsResourceInterface;
    QString mIdentifier;
};

class DialogEntryBase : public FakeEwsServer::DialogEntry
{
public:
    explicit DialogEntryBase(const QString &descr = QString(), const ReplyCallback &callback = ReplyCallback())
    {
        replyCallback = callback;
        description = descr;
    }
};

class MsgRootInboxDialogEntry : public DialogEntryBase
{
public:
    explicit MsgRootInboxDialogEntry(const QString &rootId,
                                     const QString &inboxId,
                                     const QString &descr = QString(),
                                     const ReplyCallback &callback = ReplyCallback());
};

class SubscribedFoldersDialogEntry : public DialogEntryBase
{
public:
    explicit SubscribedFoldersDialogEntry(const IsolatedTestBase::FolderList &folders,
                                          const QString &descr = QString(),
                                          const ReplyCallback &callback = ReplyCallback());
};

class SpecialFoldersDialogEntry : public DialogEntryBase
{
public:
    explicit SpecialFoldersDialogEntry(const IsolatedTestBase::FolderList &folders,
                                       const QString &descr = QString(),
                                       const ReplyCallback &callback = ReplyCallback());
};

class GetTagsEmptyDialogEntry : public DialogEntryBase
{
public:
    explicit GetTagsEmptyDialogEntry(const QString &rootId, const QString &descr = QString(), const ReplyCallback &callback = ReplyCallback());
};

class SubscribeStreamingDialogEntry : public DialogEntryBase
{
public:
    explicit SubscribeStreamingDialogEntry(const QString &descr = QString(), const ReplyCallback &callback = ReplyCallback());
};

class SyncFolderHierInitialDialogEntry : public DialogEntryBase
{
public:
    explicit SyncFolderHierInitialDialogEntry(const IsolatedTestBase::FolderList &folders,
                                              const QString &syncState,
                                              const QString &descr = QString(),
                                              const ReplyCallback &callback = ReplyCallback());
};

class UnsubscribeDialogEntry : public DialogEntryBase
{
public:
    explicit UnsubscribeDialogEntry(const QString &descr = QString(), const ReplyCallback &callback = ReplyCallback());
};

class ValidateFolderIdsDialogEntry : public DialogEntryBase
{
public:
    explicit ValidateFolderIdsDialogEntry(const QStringList &ids = QStringList(),
                                          const QString &descr = QString(),
                                          const ReplyCallback &callback = ReplyCallback());
};
