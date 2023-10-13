/*
    SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

    SPDX-FileCopyrightText: 2010 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotifieragent.h"

#include "newmailnotificationhistorydialog.h"
#include "newmailnotificationhistorymanager.h"
#include "newmailnotifieradaptor.h"
#include "newmailnotifieragentsettings.h"
#include "specialnotifierjob.h"

#include <KIdentityManagement/IdentityManager>

#include <QDBusConnection>

#include "newmailnotifier_debug.h"
#include <Akonadi/AgentManager>
#include <Akonadi/AttributeFactory>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityHiddenAttribute>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageStatus>
#include <Akonadi/NewMailNotifierAttribute>
#include <Akonadi/ServerManager>
#include <Akonadi/Session>
#include <Akonadi/SpecialMailCollections>
#include <KLocalizedString>
#include <KMime/Message>
#include <KNotification>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#if HAVE_TEXT_TO_SPEECH_SUPPORT
#include <QTextToSpeech>
#endif
#include <KWindowSystem>
using namespace std::chrono_literals;
#include <chrono>

using namespace Akonadi;

NewMailNotifierAgent::NewMailNotifierAgent(const QString &id)
    : AgentBase(id)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("newmailnotifieragent"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("akonadi_newmailnotifier_agentrc") << QStringLiteral("akonadi_newmailnotifier_agent.notifyrc"));
    migrate.migrate();
#endif
    connect(this, &Akonadi::AgentBase::reloadConfiguration, this, &NewMailNotifierAgent::slotReloadConfiguration);
    KLocalizedString::setApplicationDomain("akonadi_newmailnotifier_agent");
    Akonadi::AttributeFactory::registerAttribute<Akonadi::NewMailNotifierAttribute>();
    new NewMailNotifierAdaptor(this);

    NewMailNotifierAgentSettings::instance(KSharedConfig::openConfig());
    mIdentityManager = KIdentityManagement::IdentityManager::self();
    connect(mIdentityManager, qOverload<>(&KIdentityManagement::IdentityManager::changed), this, &NewMailNotifierAgent::slotIdentitiesChanged);
    slotIdentitiesChanged();
    mDefaultIconName = QStringLiteral("kmail");

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/NewMailNotifierAgent"), this, QDBusConnection::ExportAdaptors);

    QString service = QStringLiteral("org.freedesktop.Akonadi.NewMailNotifierAgent");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    }
    QDBusConnection::sessionBus().registerService(service);

    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceStatusChanged, this, &NewMailNotifierAgent::slotInstanceStatusChanged);
    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceRemoved, this, &NewMailNotifierAgent::slotInstanceRemoved);
    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceAdded, this, &NewMailNotifierAgent::slotInstanceAdded);
    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceNameChanged, this, &NewMailNotifierAgent::slotInstanceNameChanged);

    changeRecorder()->setMimeTypeMonitored(KMime::Message::mimeType());
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->itemFetchScope().setFetchModificationTime(false);
    changeRecorder()->fetchCollection(true);
    changeRecorder()->setChangeRecordingEnabled(false);
    changeRecorder()->ignoreSession(Akonadi::Session::defaultSession());
    changeRecorder()->collectionFetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    changeRecorder()->setCollectionMonitored(Collection::root(), true);
    mTimer.setInterval(5s); // 5secondes
    connect(&mTimer, &QTimer::timeout, this, &NewMailNotifierAgent::slotShowNotifications);

    if (isActive()) {
        mTimer.setSingleShot(true);
    }
}

NewMailNotifierAgent::~NewMailNotifierAgent()
{
    delete mHistoryNotificationDialog;
}

void NewMailNotifierAgent::slotReloadConfiguration()
{
    NewMailNotifierAgentSettings::self()->load();
}

void NewMailNotifierAgent::slotIdentitiesChanged()
{
    mListEmails = mIdentityManager->allEmails();
}

void NewMailNotifierAgent::doSetOnline(bool online)
{
    if (!online) {
        clearAll();
    }
}

void NewMailNotifierAgent::setEnableAgent(bool enabled)
{
    NewMailNotifierAgentSettings::setEnabled(enabled);
    NewMailNotifierAgentSettings::self()->save();
    if (!enabled) {
        clearAll();
    }
}

bool NewMailNotifierAgent::enabledAgent() const
{
    return NewMailNotifierAgentSettings::enabled();
}

void NewMailNotifierAgent::clearAll()
{
    mNewMails.clear();
    mInstanceNameInProgress.clear();
}

bool NewMailNotifierAgent::excludeSpecialCollection(const Akonadi::Collection &collection) const
{
    if (collection.hasAttribute<Akonadi::EntityHiddenAttribute>()) {
        return true;
    }

    if (collection.hasAttribute<Akonadi::NewMailNotifierAttribute>()) {
        if (collection.attribute<Akonadi::NewMailNotifierAttribute>()->ignoreNewMail()) {
            return true;
        }
    }

    if (!collection.contentMimeTypes().contains(KMime::Message::mimeType())) {
        return true;
    }

    SpecialMailCollections::Type type = SpecialMailCollections::self()->specialCollectionType(collection);
    switch (type) {
    case SpecialMailCollections::Invalid: // Not a special collection
    case SpecialMailCollections::Inbox:
        return false;
    default:
        return true;
    }
}

void NewMailNotifierAgent::itemsRemoved(const Item::List &items)
{
    if (!isActive()) {
        return;
    }

    const QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::iterator end(mNewMails.end());
    for (QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::iterator it = mNewMails.begin(); it != end; ++it) {
        QList<Akonadi::Item::Id> idList = it.value();
        bool itemFound = false;
        for (const Item &item : items) {
            const int numberOfItemsRemoved = idList.removeAll(item.id());
            if (numberOfItemsRemoved > 0) {
                itemFound = true;
            }
        }
        if (itemFound) {
            if (mNewMails[it.key()].isEmpty()) {
                mNewMails.remove(it.key());
            } else {
                mNewMails[it.key()] = idList;
            }
        }
    }
}

void NewMailNotifierAgent::itemsFlagsChanged(const Akonadi::Item::List &items, const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    Q_UNUSED(removedFlags)

    if (!isActive()) {
        return;
    }
    for (const Akonadi::Item &item : items) {
        const QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::iterator end(mNewMails.end());
        for (QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::iterator it = mNewMails.begin(); it != end; ++it) {
            QList<Akonadi::Item::Id> idList = it.value();
            if (idList.contains(item.id()) && addedFlags.contains("\\SEEN")) {
                idList.removeAll(item.id());
                if (idList.isEmpty()) {
                    mNewMails.remove(it.key());
                    break;
                } else {
                    (*it) = idList;
                }
            }
        }
    }
}

void NewMailNotifierAgent::itemsMoved(const Akonadi::Item::List &items,
                                      const Akonadi::Collection &collectionSource,
                                      const Akonadi::Collection &collectionDestination)
{
    if (!isActive()) {
        return;
    }

    for (const Akonadi::Item &item : items) {
        if (ignoreStatusMail(item)) {
            continue;
        }

        if (excludeSpecialCollection(collectionSource)) {
            continue; // outbox, sent-mail, trash, drafts or templates.
        }

        if (mNewMails.contains(collectionSource)) {
            QList<Akonadi::Item::Id> idListFrom = mNewMails[collectionSource];
            const int removeItems = idListFrom.removeAll(item.id());
            if (removeItems > 0) {
                if (idListFrom.isEmpty()) {
                    mNewMails.remove(collectionSource);
                } else {
                    mNewMails[collectionSource] = idListFrom;
                }
                if (!excludeSpecialCollection(collectionDestination)) {
                    QList<Akonadi::Item::Id> idListTo = mNewMails[collectionDestination];
                    idListTo.append(item.id());
                    mNewMails[collectionDestination] = idListTo;
                }
            }
        }
    }
}

bool NewMailNotifierAgent::ignoreStatusMail(const Akonadi::Item &item)
{
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isRead() || status.isSpam() || status.isIgnored()) {
        return true;
    }
    return false;
}

void NewMailNotifierAgent::itemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    if (!isActive()) {
        return;
    }

    if (excludeSpecialCollection(collection)) {
        return; // outbox, sent-mail, trash, drafts or templates.
    }

    if (ignoreStatusMail(item)) {
        return;
    }

    if (!mTimer.isActive()) {
        mTimer.start();
    }
    mNewMails[collection].append(item.id());
}

void NewMailNotifierAgent::showNotNotificationHistoryDialog(qlonglong windowId)
{
    if (!mHistoryNotificationDialog) {
        mHistoryNotificationDialog = new NewMailNotificationHistoryDialog(nullptr);
        mHistoryNotificationDialog->setAttribute(Qt::WA_NativeWindow, true);
    }
    KWindowSystem::setMainWindow(mHistoryNotificationDialog->windowHandle(), windowId);
    mHistoryNotificationDialog->show();
    mHistoryNotificationDialog->raise();
    mHistoryNotificationDialog->activateWindow();
    mHistoryNotificationDialog->setModal(false);
}

void NewMailNotifierAgent::setVerboseMailNotification(bool b)
{
    NewMailNotifierAgentSettings::setVerboseNotification(b);
    NewMailNotifierAgentSettings::self()->save();
}

bool NewMailNotifierAgent::verboseMailNotification() const
{
    return NewMailNotifierAgentSettings::verboseNotification();
}

void NewMailNotifierAgent::slotShowNotifications()
{
    if (mNewMails.isEmpty()) {
        return;
    }

    if (!isActive()) {
        return;
    }

    if (!mInstanceNameInProgress.isEmpty()) {
        // Restart timer until all is done.
        mTimer.start();
        return;
    }

    QString message;
    if (NewMailNotifierAgentSettings::verboseNotification()) {
        bool hasUniqMessage = true;
        Akonadi::Item::Id item = -1;
        QString currentPath;
        QStringList texts;
        const int numberOfCollection(mNewMails.count());
        if (numberOfCollection > 1) {
            hasUniqMessage = false;
        }

        const QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::const_iterator end(mNewMails.constEnd());
        for (QHash<Akonadi::Collection, QList<Akonadi::Item::Id>>::const_iterator it = mNewMails.constBegin(); it != end; ++it) {
            const auto attr = it.key().attribute<Akonadi::EntityDisplayAttribute>();
            QString displayName;
            if (attr && !attr->displayName().isEmpty()) {
                displayName = attr->displayName();
            } else {
                displayName = it.key().name();
            }

            if (hasUniqMessage) {
                const int numberOfValue(it.value().count());
                if (numberOfValue == 0) {
                    // You can have an unique folder with 0 message
                    return;
                } else if (numberOfValue == 1) {
                    item = it.value().at(0);
                    currentPath = displayName;
                    break;
                } else {
                    hasUniqMessage = false;
                }
            }
            QString resourceName;
            if (!mCacheResourceName.contains(it.key().resource())) {
                const Akonadi::AgentInstance::List lst = Akonadi::AgentManager::self()->instances();
                for (const Akonadi::AgentInstance &instance : lst) {
                    if (instance.identifier() == it.key().resource()) {
                        mCacheResourceName.insert(instance.identifier(), instance.name());
                        resourceName = instance.name();
                        break;
                    }
                }
            } else {
                resourceName = mCacheResourceName.value(it.key().resource());
            }
            const int numberOfEmails(it.value().count());
            if (numberOfEmails > 0) {
                texts.append(i18ncp("%2 = name of mail folder; %3 = name of Akonadi POP3/IMAP/etc resource (as user named it)",
                                    "One new email in %2 from \"%3\"",
                                    "%1 new emails in %2 from \"%3\"",
                                    numberOfEmails,
                                    displayName,
                                    resourceName));
            }
        }
        if (hasUniqMessage) {
            auto job = new SpecialNotifierJob(mListEmails, currentPath, item, this);
            job->setDefaultIconName(mDefaultIconName);
            connect(job, &SpecialNotifierJob::displayNotification, this, &NewMailNotifierAgent::slotDisplayNotification);
#if HAVE_TEXT_TO_SPEECH_SUPPORT
            connect(job, &SpecialNotifierJob::say, this, &NewMailNotifierAgent::slotSay);
#endif
            mNewMails.clear();
            return;
        } else {
            message = texts.join(QLatin1String("<br>"));
        }
    } else {
        message = i18n("New mail arrived");
    }

    qCDebug(NEWMAILNOTIFIER_LOG) << message;

    slotDisplayNotification(QPixmap(), message);

    mNewMails.clear();
}

void NewMailNotifierAgent::slotDisplayNotification(const QPixmap &pixmap, const QString &message)
{
    if (NewMailNotifierAgentSettings::enableNotificationHistory()) {
        NewMailNotificationHistoryManager::self()->addHistory(message);
    }
    if (pixmap.isNull()) {
        KNotification::event(QStringLiteral("new-email"),
                             QString(),
                             message,
                             mDefaultIconName,
                             nullptr,
                             NewMailNotifierAgentSettings::keepPersistentNotification() ? KNotification::Persistent | KNotification::SkipGrouping
                                                                                        : KNotification::CloseOnTimeout,
                             QStringLiteral("akonadi_newmailnotifier_agent"));
    } else {
        KNotification::event(QStringLiteral("new-email"),
                             message,
                             pixmap,
                             nullptr,
                             NewMailNotifierAgentSettings::keepPersistentNotification() ? KNotification::Persistent | KNotification::SkipGrouping
                                                                                        : KNotification::CloseOnTimeout,
                             QStringLiteral("akonadi_newmailnotifier_agent"));
    }
}

void NewMailNotifierAgent::slotInstanceNameChanged(const Akonadi::AgentInstance &instance)
{
    if (!isActive()) {
        return;
    }

    const QString identifier(instance.identifier());
    int resourceNameRemoved = mCacheResourceName.remove(identifier);
    if (resourceNameRemoved > 0) {
        mCacheResourceName.insert(identifier, instance.name());
    }
}

void NewMailNotifierAgent::slotInstanceStatusChanged(const Akonadi::AgentInstance &instance)
{
    if (!isActive()) {
        return;
    }

    const QString identifier(instance.identifier());
    switch (instance.status()) {
    case Akonadi::AgentInstance::Broken:
    case Akonadi::AgentInstance::Idle:
        mInstanceNameInProgress.removeAll(identifier);
        break;
    case Akonadi::AgentInstance::Running:
        if (!excludeAgentType(instance)) {
            if (!mInstanceNameInProgress.contains(identifier)) {
                mInstanceNameInProgress.append(identifier);
            }
        }
        break;
    case Akonadi::AgentInstance::NotConfigured:
        // Nothing
        break;
    }
}

bool NewMailNotifierAgent::excludeAgentType(const Akonadi::AgentInstance &instance)
{
    if (instance.type().mimeTypes().contains(KMime::Message::mimeType())) {
        const QStringList capabilities(instance.type().capabilities());
        if (capabilities.contains(QLatin1String("Resource")) && !capabilities.contains(QLatin1String("Virtual"))
            && !capabilities.contains(QLatin1String("MailTransport"))) {
            return false;
        } else {
            return true;
        }
    }
    return true;
}

void NewMailNotifierAgent::slotInstanceRemoved(const Akonadi::AgentInstance &instance)
{
    if (!isActive()) {
        return;
    }

    const QString identifier(instance.identifier());
    mInstanceNameInProgress.removeAll(identifier);
}

void NewMailNotifierAgent::slotInstanceAdded(const Akonadi::AgentInstance &instance)
{
    mCacheResourceName.insert(instance.identifier(), instance.name());
}

void NewMailNotifierAgent::printDebug()
{
    qCDebug(NEWMAILNOTIFIER_LOG) << "instance in progress: " << mInstanceNameInProgress << "\n notifier enabled : " << NewMailNotifierAgentSettings::enabled()
                                 << "\n check in progress : " << !mInstanceNameInProgress.isEmpty();
}

bool NewMailNotifierAgent::isActive() const
{
    return isOnline() && NewMailNotifierAgentSettings::enabled();
}

void NewMailNotifierAgent::slotSay(const QString &message)
{
#if HAVE_TEXT_TO_SPEECH_SUPPORT
    if (!mTextToSpeech) {
        mTextToSpeech = new QTextToSpeech(this);
    }
    if (mTextToSpeech->availableEngines().isEmpty()) {
        qCWarning(NEWMAILNOTIFIER_LOG) << "No texttospeech engine available";
    } else {
        mTextToSpeech->say(message);
    }
#endif
}

AKONADI_AGENT_MAIN(NewMailNotifierAgent)
