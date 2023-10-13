/*
    SPDX-FileCopyrightText: 2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "isolatedtestbase.h"

#include <QTest>

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/Control>

#include "ewsresourceinterface.h"
#include "ewssettings.h"
#include "ewswallet.h"
#include "fakeewsserverthread.h"

using namespace Akonadi;

constexpr int OnlineStateChangeTimeoutMs = 20000;

IsolatedTestBase::IsolatedTestBase(QObject *parent)
    : QObject(parent)
    , mFakeServerThread(new FakeEwsServerThread(this))
{
}

IsolatedTestBase::~IsolatedTestBase() = default;

void IsolatedTestBase::init()
{
    QVERIFY(Control::start());

    /* Switch all resources offline to reduce interference from them */
    for (AgentInstance agent : AgentManager::self()->instances()) {
        agent.setIsOnline(false);
    }

    connect(AgentManager::self(), &AgentManager::instanceAdded, this, [](const Akonadi::AgentInstance &instance) {
        qDebug() << "AgentInstanceAdded" << instance.identifier();
    });

    mFakeServerThread->start();
    QVERIFY(mFakeServerThread->waitServerStarted());
}

void IsolatedTestBase::cleanup()
{
    mFakeServerThread->exit();
    mFakeServerThread->wait();
}

QString IsolatedTestBase::loadResourceAsString(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        return QString::fromUtf8(f.readAll());
    }
    qWarning() << "Resource" << path << "not found";
    return {};
}

template<typename T>
QDBusReply<T> dBusSetAndWaitReply(std::function<QDBusReply<T>()> setFunc, std::function<QDBusReply<T>()> getFunc, const QString &name)
{
    QDBusReply<T> reply;
    int retryCnt = 4;
    do {
        setFunc();
        reply = getFunc();
        if (!reply.isValid()) {
            qDebug() << "Failed to set DBus config option" << name << reply.error().message();
            QThread::msleep(250);
        }
    } while (!reply.isValid() && retryCnt-- > 0);

    return reply;
}

TestAgentInstance::TestAgentInstance(const QString &url)
{
    AgentType ewsType = AgentManager::self()->type(QStringLiteral("akonadi_ews_resource"));
    auto agentCreateJob = new AgentInstanceCreateJob(ewsType);
    QVERIFY(agentCreateJob->exec());

    mEwsInstance.reset(new AgentInstance(agentCreateJob->instance()));
    mIdentifier = mEwsInstance->identifier();
    const QString akonadiInstanceIdentifier = QProcessEnvironment::systemEnvironment().value(QStringLiteral("AKONADI_INSTANCE"));

    mEwsSettingsInterface.reset(
        new OrgKdeAkonadiEwsSettingsInterface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + mIdentifier + QLatin1Char('.') + akonadiInstanceIdentifier,
                                              QStringLiteral("/Settings"),
                                              QDBusConnection::sessionBus(),
                                              this));
    QVERIFY(mEwsSettingsInterface->isValid());

    mEwsWalletInterface.reset(
        new OrgKdeAkonadiEwsWalletInterface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + mIdentifier + QLatin1Char('.') + akonadiInstanceIdentifier,
                                            QStringLiteral("/Settings"),
                                            QDBusConnection::sessionBus(),
                                            this));
    QVERIFY(mEwsWalletInterface->isValid());

    mEwsResourceInterface.reset(
        new OrgKdeAkonadiEwsResourceInterface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + mIdentifier + QLatin1Char('.') + akonadiInstanceIdentifier,
                                              QStringLiteral("/"),
                                              QDBusConnection::sessionBus(),
                                              this));
    QVERIFY(mEwsResourceInterface->isValid());

    /* The EWS resource initializes its DBus adapters asynchronously. Therefore it can happen that
     * due to a race access is attempted prior to their initialization. To fix this retry the DBus
     * communication a few times before declaring failure. */
    const auto baseUrlReply = dBusSetAndWaitReply<QString>(std::bind(&OrgKdeAkonadiEwsSettingsInterface::setBaseUrl, mEwsSettingsInterface.data(), url),
                                                           std::bind(&OrgKdeAkonadiEwsSettingsInterface::baseUrl, mEwsSettingsInterface.data()),
                                                           QStringLiteral("Base URL"));
    QVERIFY(baseUrlReply.isValid());
    QVERIFY(baseUrlReply.value() == url);

    const auto hasDomainReply = dBusSetAndWaitReply<bool>(std::bind(&OrgKdeAkonadiEwsSettingsInterface::setHasDomain, mEwsSettingsInterface.data(), false),
                                                          std::bind(&OrgKdeAkonadiEwsSettingsInterface::hasDomain, mEwsSettingsInterface.data()),
                                                          QStringLiteral("has domain"));
    QVERIFY(hasDomainReply.isValid());
    QVERIFY(hasDomainReply.value() == false);

    const auto username = QStringLiteral("test");
    const auto usernameReply = dBusSetAndWaitReply<QString>(std::bind(&OrgKdeAkonadiEwsSettingsInterface::setUsername, mEwsSettingsInterface.data(), username),
                                                            std::bind(&OrgKdeAkonadiEwsSettingsInterface::username, mEwsSettingsInterface.data()),
                                                            QStringLiteral("Username"));
    QVERIFY(usernameReply.isValid());
    QVERIFY(usernameReply.value() == username);

    mEwsWalletInterface->setTestPassword(QStringLiteral("test"));
    AgentManager::self()->instance(mIdentifier).reconfigure();
}

TestAgentInstance::~TestAgentInstance()
{
    if (mEwsInstance) {
        AgentManager::self()->removeInstance(*mEwsInstance);
    }
}

bool TestAgentInstance::isValid() const
{
    return mEwsInstance && mEwsSettingsInterface && mEwsWalletInterface && !mIdentifier.isEmpty();
}

const QString &TestAgentInstance::identifier() const
{
    return mIdentifier;
}

bool TestAgentInstance::setOnline(bool online, bool wait)
{
    if (wait) {
        QEventLoop loop;
        auto conn = connect(
            AgentManager::self(),
            &AgentManager::instanceOnline,
            this,
            [&](const AgentInstance &instance, bool state) {
                if (instance == *mEwsInstance && state == online) {
                    qDebug() << "is online" << state;
                    loop.exit(0);
                }
            },
            Qt::QueuedConnection);
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, this, [&]() {
            qWarning() << "Timeout waiting for desired resource online state.";
            loop.exit(1);
        });
        timer.start(OnlineStateChangeTimeoutMs);
        mEwsInstance->setIsOnline(online);
        bool status = (loop.exec() == 0);
        disconnect(conn);
        return status;
    } else {
        mEwsInstance->setIsOnline(online);
        return true;
    }
}

OrgKdeAkonadiEwsSettingsInterface &TestAgentInstance::settingsInterface() const
{
    return *mEwsSettingsInterface;
}

OrgKdeAkonadiEwsWalletInterface &TestAgentInstance::walletInterface() const
{
    return *mEwsWalletInterface;
}

OrgKdeAkonadiEwsResourceInterface &TestAgentInstance::resourceInterface() const
{
    return *mEwsResourceInterface;
}

Akonadi::AgentInstance &TestAgentInstance::instance() const
{
    return *mEwsInstance;
}

MsgRootInboxDialogEntry::MsgRootInboxDialogEntry(const QString &rootId, const QString &inboxId, const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/getfolder-inbox-msgroot")).arg(rootId, inboxId);
    description = QStringLiteral("GetFolder request for inbox and msgroot");
}

SubscribedFoldersDialogEntry::SubscribedFoldersDialogEntry(const IsolatedTestBase::FolderList &list, const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    static const QVector<IsolatedTestBase::Folder::DistinguishedType> specialFolders = {
        IsolatedTestBase::Folder::Inbox,
        IsolatedTestBase::Folder::Calendar,
        IsolatedTestBase::Folder::Tasks,
        IsolatedTestBase::Folder::Contacts,
    };
    QHash<IsolatedTestBase::Folder::DistinguishedType, const IsolatedTestBase::Folder *> folderHash;
    for (const auto &folder : list) {
        if (specialFolders.contains(folder.type)) {
            folderHash.insert(folder.type, &folder);
        }
    }

    QString xml;
    for (auto special : specialFolders) {
        const IsolatedTestBase::Folder *folder = folderHash[special];
        if (QTest::qVerify(folder != nullptr, "folder != nullptr", "", __FILE__, __LINE__)) {
            xml += QStringLiteral("<m:GetFolderResponseMessage ResponseClass=\"Success\">");
            xml += QStringLiteral("<m:ResponseCode>NoError</m:ResponseCode>");
            xml += QStringLiteral("<m:Folders><t:Folder>");
            xml += QStringLiteral("<t:FolderId Id=\"%1\" ChangeKey=\"MDAx\" />").arg(folder->id);
            xml += QStringLiteral("</t:Folder></m:Folders>");
            xml += QStringLiteral("</m:GetFolderResponseMessage>");
        }
    }

    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/getfolder-subscribedfolders")).arg(xml);
}

SpecialFoldersDialogEntry::SpecialFoldersDialogEntry(const IsolatedTestBase::FolderList &list, const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    static const QVector<IsolatedTestBase::Folder::DistinguishedType> specialFolders = {IsolatedTestBase::Folder::Inbox,
                                                                                        IsolatedTestBase::Folder::Outbox,
                                                                                        IsolatedTestBase::Folder::Sent,
                                                                                        IsolatedTestBase::Folder::Trash,
                                                                                        IsolatedTestBase::Folder::Drafts};
    QHash<IsolatedTestBase::Folder::DistinguishedType, const IsolatedTestBase::Folder *> folderHash;
    for (const auto &folder : list) {
        if (specialFolders.contains(folder.type)) {
            folderHash.insert(folder.type, &folder);
        }
    }

    QString xml;
    for (auto special : specialFolders) {
        const IsolatedTestBase::Folder *folder = folderHash[special];
        if (QTest::qVerify(folder != nullptr, "folder != nullptr", "", __FILE__, __LINE__)) {
            xml += QStringLiteral("<m:GetFolderResponseMessage ResponseClass=\"Success\">");
            xml += QStringLiteral("<m:ResponseCode>NoError</m:ResponseCode>");
            xml += QStringLiteral("<m:Folders><t:Folder>");
            xml += QStringLiteral("<t:FolderId Id=\"%1\" ChangeKey=\"MDAx\" />").arg(folder->id);
            xml += QStringLiteral("</t:Folder></m:Folders>");
            xml += QStringLiteral("</m:GetFolderResponseMessage>");
        }
    }

    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/getfolder-specialfolders")).arg(xml);
}

GetTagsEmptyDialogEntry::GetTagsEmptyDialogEntry(const QString &rootId, const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/getfolder-tags")).arg(rootId);
}

SubscribeStreamingDialogEntry::SubscribeStreamingDialogEntry(const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/subscribe-streaming"));
}

SyncFolderHierInitialDialogEntry::SyncFolderHierInitialDialogEntry(const IsolatedTestBase::FolderList &list,
                                                                   const QString &syncState,
                                                                   const QString &descr,
                                                                   const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    QHash<QString, int> childCount;
    for (const auto &folder : list) {
        ++childCount[folder.parentId];
    }

    QString xml;
    for (const auto &folder : list) {
        if (folder.type == IsolatedTestBase::Folder::Root) {
            continue;
        }
        xml += QStringLiteral("<t:Create>");
        xml += QStringLiteral("<t:Folder>");
        xml += QStringLiteral("<t:FolderId Id=\"%1\" ChangeKey=\"MDAx\" />").arg(folder.id);
        xml += QStringLiteral("<t:ParentFolderId Id=\"%1\" ChangeKey=\"MDAx\" />").arg(folder.parentId);
        QString folderClass;
        QString extraXml;
        switch (folder.type) {
        case IsolatedTestBase::Folder::Calendar:
            folderClass = QStringLiteral("IPF.Calendar");
            break;
        case IsolatedTestBase::Folder::Contacts:
            folderClass = QStringLiteral("IPF.Contacts");
            break;
        case IsolatedTestBase::Folder::Tasks:
            folderClass = QStringLiteral("IPF.Tasks");
            break;
        default:
            folderClass = QStringLiteral("IPF.Note");
            extraXml = QStringLiteral("<t:UnreadCount>0</t:UnreadCount>");
        }
        xml += QStringLiteral("<t:FolderClass>%1</t:FolderClass>").arg(folderClass);
        xml += QStringLiteral("<t:TotalCount>0</t:TotalCount>");
        xml += QStringLiteral("<t:DisplayName>%1</t:DisplayName>").arg(folder.name);
        xml += QStringLiteral("<t:ChildFolderCount>%1</t:ChildFolderCount>").arg(childCount[folder.id]);
        xml += extraXml;
        xml += QStringLiteral("</t:Folder>");
        xml += QStringLiteral("</t:Create>");
    }
    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/syncfolderhierarhy-emptystate")).arg(syncState, xml);
}

UnsubscribeDialogEntry::UnsubscribeDialogEntry(const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/unsubscribe"));
}

ValidateFolderIdsDialogEntry::ValidateFolderIdsDialogEntry(const QStringList &ids, const QString &descr, const ReplyCallback &callback)
    : DialogEntryBase(descr, callback)
{
    QStringList xQueryFolderIds;
    QString responseXml;
    int folderIndex = 0;

    for (const auto &folderId : ids) {
        xQueryFolderIds.append(QStringLiteral("//m:GetFolder/m:FolderIds/t:FolderId[position()=%1 and @Id=\"%2\"]").arg(++folderIndex).arg(folderId));
        responseXml += QStringLiteral("<m:GetFolderResponseMessage ResponseClass=\"Success\">");
        responseXml += QStringLiteral("<m:ResponseCode>NoError</m:ResponseCode>");
        responseXml += QStringLiteral("<m:Folders><t:Folder>");
        responseXml += QStringLiteral("<t:FolderId Id=\"%1\" ChangeKey=\"MDAx\" />").arg(folderId);
        responseXml += QStringLiteral("</t:Folder></m:Folders>");
        responseXml += QStringLiteral("</m:GetFolderResponseMessage>");
    }

    xQuery = IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xquery/getfolder-validateids"))
                 .arg(folderIndex)
                 .arg(xQueryFolderIds.join(QStringLiteral(" and ")), responseXml);
}
