/*
    SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "googleresourcemigrator.h"
#include "googlesettingsinterface.h"
#include "migration_debug.h"

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/ServerManager>

#include <KLocalizedString>
#include <KWallet>

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QSettings>

#include <memory>

GoogleResourceMigrator::GoogleResourceMigrator()
    : MigratorBase(QStringLiteral("googleresourcemigrator"))
{
}

QString GoogleResourceMigrator::displayName() const
{
    return i18nc("Name of the Migrator (intended for advanced users).", "Google Resource Migrator");
}

QString GoogleResourceMigrator::description() const
{
    return i18nc("Description of the migrator", "Migrates the old Google Calendar and Google Contacts resources to the new unified Google Groupware Resource");
}

bool GoogleResourceMigrator::shouldAutostart() const
{
    return true;
}

namespace
{
static const QStringView akonadiGoogleCalendarResource = {u"akonadi_googlecalendar_resource"};
static const QStringView akonadiGoogleContactsResource = {u"akonadi_googlecontacts_resource"};
static const QStringView akonadiGoogleGroupwareResource = {u"akonadi_google_resource"};

bool isLegacyGoogleResource(const Akonadi::AgentInstance &instance)
{
    return instance.type().identifier() == akonadiGoogleCalendarResource || instance.type().identifier() == akonadiGoogleContactsResource;
}

bool isGoogleGroupwareResource(const Akonadi::AgentInstance &instance)
{
    return instance.type().identifier() == akonadiGoogleGroupwareResource;
}

std::unique_ptr<QSettings> settingsForResource(const Akonadi::AgentInstance &instance)
{
    Q_ASSERT(instance.isValid());
    if (!instance.isValid()) {
        return {};
    }

    const QString configFile = Akonadi::ServerManager::self()->addNamespace(instance.identifier()) + QStringLiteral("rc");
    const auto configPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, configFile);
    return std::make_unique<QSettings>(configPath, QSettings::IniFormat);
}

QString getAccountNameFromResourceSettings(const Akonadi::AgentInstance &instance)
{
    Q_ASSERT(instance.isValid());
    if (!instance.isValid()) {
        return {};
    }

    const auto config = settingsForResource(instance);
    QString account = config->value(QStringLiteral("Account")).toString();
    if (account.isEmpty()) {
        account = config->value(QStringLiteral("AccountName")).toString();
    }

    return account;
}

static const auto WalletFolder = QStringLiteral("Akonadi Google");

std::unique_ptr<KWallet::Wallet> getWallet()
{
    std::unique_ptr<KWallet::Wallet> wallet{KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Synchronous)};
    if (!wallet) {
        qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: failed to open KWallet.";
        return {};
    }

    if (!wallet->hasFolder(WalletFolder)) {
        qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: couldn't find wallet folder for Google resources.";
        return {};
    }
    wallet->setFolder(WalletFolder);

    return wallet;
}

QMap<QString, QString> backupKWalletData(const QString &account)
{
    qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: backing up KWallet data for" << account;

    const auto wallet = getWallet();
    if (!wallet) {
        return {};
    }

    if (!wallet->entryList().contains(account)) {
        qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: couldn't find KWallet data for account" << account;
        return {};
    }

    QMap<QString, QString> map;
    wallet->readMap(account, map);
    return map;
}

void restoreKWalletData(const QString &account, const QMap<QString, QString> &data)
{
    qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: restoring KWallet data for" << account;

    auto wallet = getWallet();
    if (!wallet) {
        return;
    }

    wallet->writeMap(account, data);
}

void removeInstanceAndWait(const Akonadi::AgentInstance &instance)
{
    // Make sure we wait for the resource to actually stop - otherwise we are risking
    // race when we restore the KWallet secrets from backup before the removed resource
    // actually tries to remove them from the wallet.
    const QString serviceName = Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, instance.identifier());
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName)) {
        Akonadi::AgentManager::self()->removeInstance(instance);
    } else {
        QDBusServiceWatcher watcher(Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, instance.identifier()),
                                    QDBusConnection::sessionBus(),
                                    QDBusServiceWatcher::WatchForUnregistration);
        QEventLoop loop;
        QObject::connect(&watcher, &QDBusServiceWatcher::serviceUnregistered, &loop, [&loop, &instance]() {
            qCDebug(MIGRATION_LOG) << "GoogleResourceMigrator: resource" << instance.identifier() << "has disappeared from DBus";
            loop.quit();
        });
        QTimer::singleShot(std::chrono::seconds(20), &loop, [&loop, &instance]() {
            qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: timeout while waiting for resource" << instance.identifier() << "to be removed";
            loop.quit();
        });

        Akonadi::AgentManager::self()->removeInstance(instance);
        qCDebug(MIGRATION_LOG) << "GoogleResourceMigrator: waiting for" << instance.identifier() << "to disappear from DBus";
        loop.exec();
    }

    qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: removed the legacy calendar resource" << instance.identifier();
}
} // namespace

void GoogleResourceMigrator::startWork()
{
    // Discover all existing Google Contacts and Google Calendar resources
    const auto allInstances = Akonadi::AgentManager::self()->instances();
    for (const auto &instance : allInstances) {
        if (isLegacyGoogleResource(instance)) {
            const auto account = getAccountNameFromResourceSettings(instance);
            if (account.isEmpty()) {
                qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: resource" << instance.identifier() << "is not configured, removing";
                Akonadi::AgentManager::self()->removeInstance(instance);
                continue;
            }
            qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: discovered resource" << instance.identifier() << "for account" << account;
            if (instance.type().identifier() == akonadiGoogleCalendarResource) {
                mMigrations[account].calendarResource = instance;
            } else if (instance.type().identifier() == akonadiGoogleContactsResource) {
                mMigrations[account].contactResource = instance;
            }
        } else if (isGoogleGroupwareResource(instance)) {
            const auto account = getAccountNameFromResourceSettings(instance);
            mMigrations[account].alreadyExists = true;
        }
    }

    mMigrationCount = mMigrations.size();
    migrateNextAccount();
}

void GoogleResourceMigrator::removeLegacyInstances(const QString &account, const Instances &instances)
{
    // Legacy resources wipe KWallet data when removed, so we need to back the data up
    // before removing them and restore it afterwards
    const auto kwalletData = backupKWalletData(account);

    if (instances.calendarResource.isValid()) {
        removeInstanceAndWait(instances.calendarResource);
    }
    if (instances.contactResource.isValid()) {
        removeInstanceAndWait(instances.contactResource);
    }

    restoreKWalletData(account, kwalletData);
}

void GoogleResourceMigrator::migrateNextAccount()
{
    setProgress((static_cast<float>(mMigrationsDone) / mMigrationCount) * 100);
    if (mMigrations.empty()) {
        setMigrationState(MigratorBase::Complete);
        return;
    }

    QString account;
    Instances instances;
    std::tie(account, instances) = *mMigrations.constKeyValueBegin();
    mMigrations.erase(mMigrations.begin());

    if (instances.alreadyExists) {
        Q_EMIT message(Info, i18n("Google Groupware Resource for account %1 already exists, skipping.", account));
        // Just to be sure, check that there are no left-over legacy instances
        removeLegacyInstances(account, instances);

        ++mMigrationsDone;
        QMetaObject::invokeMethod(this, &GoogleResourceMigrator::migrateNextAccount, Qt::QueuedConnection);
        return;
    }

    qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: starting migration of account" << account;
    Q_EMIT message(Info, i18n("Starting migration of account %1", account));
    qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: creating new" << akonadiGoogleGroupwareResource;
    Q_EMIT message(Info, i18n("Creating new instance of Google Gropware Resource"));
    auto job = new Akonadi::AgentInstanceCreateJob(akonadiGoogleGroupwareResource.toString(), this);
    connect(job, &Akonadi::AgentInstanceCreateJob::finished, this, [this, job, account, instances](KJob *) {
        if (job->error()) {
            qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: Failed to create new Google Groupware Resource:" << job->errorString();
            Q_EMIT message(Error, i18n("Failed to create a new Google Groupware Resource: %1", job->errorString()));
            setMigrationState(MigratorBase::Failed);
            return;
        }

        const auto newInstance = job->instance();
        if (!migrateAccount(account, instances, newInstance)) {
            qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: failed to migrate account" << account;
            Q_EMIT message(Error, i18n("Failed to migrate account %1", account));
            setMigrationState(MigratorBase::Failed);
            return;
        }

        removeLegacyInstances(account, instances);

        // Reconfigure and restart the new instance
        newInstance.reconfigure();
        newInstance.restart();

        if (instances.calendarResource.isValid() ^ instances.contactResource.isValid()) {
            const auto res = instances.calendarResource.isValid() ? instances.calendarResource.identifier() : instances.contactResource.identifier();
            qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: migrated configuration from" << res << "to" << newInstance.identifier();
        } else {
            qCInfo(MIGRATION_LOG) << "GoogleResourceMigrator: migrated configuration from" << instances.calendarResource.identifier() << "and"
                                  << instances.contactResource.identifier() << "to" << newInstance.identifier();
        }
        Q_EMIT message(Success, i18n("Migrated account %1 to new Google Groupware Resource", account));

        ++mMigrationsDone;
        migrateNextAccount();
    });
    job->start();
}

QString GoogleResourceMigrator::mergeAccountNames(const ResourceValues<QString> &accountName, const Instances &oldInstances) const
{
    if (!accountName.calendar.isEmpty() && !accountName.contacts.isEmpty()) {
        if (accountName.calendar == accountName.contacts) {
            return accountName.calendar;
        } else {
            qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: account name mismatch:" << oldInstances.calendarResource.identifier() << "="
                                     << accountName.calendar << "," << oldInstances.contactResource.identifier() << "=" << accountName.contacts
                                     << ". Ignoring both.";
        }
    } else if (!accountName.calendar.isEmpty()) {
        return accountName.calendar;
    } else if (!accountName.contacts.isEmpty()) {
        return accountName.contacts;
    }

    return {};
}

int GoogleResourceMigrator::mergeAccountIds(ResourceValues<int> accountId, const Instances &oldInstances) const
{
    if (accountId.calendar > 0 && accountId.contacts > 0) {
        if (accountId.calendar == accountId.contacts) {
            return accountId.calendar;
        } else {
            qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: account id mismatch:" << oldInstances.calendarResource.identifier() << "="
                                     << accountId.calendar << "," << oldInstances.contactResource.identifier() << "=" << accountId.contacts
                                     << ". Ignoring both.";
        }
        return 0;
    }

    // Return the non-zero entry
    return std::max(accountId.calendar, accountId.contacts);
}

bool GoogleResourceMigrator::migrateAccount(const QString &account, const Instances &oldInstances, const Akonadi::AgentInstance &newInstance)
{
    org::kde::Akonadi::Google::Settings resourceSettings{
        Akonadi::ServerManager::self()->agentServiceName(Akonadi::ServerManager::Resource, newInstance.identifier()),
        QStringLiteral("/Settings"),
        QDBusConnection::sessionBus()};
    if (!resourceSettings.isValid()) {
        qCWarning(MIGRATION_LOG) << "GoogleResourceMigrator: failed to obtain settings DBus interface of " << newInstance.identifier();
        return false;
    }

    resourceSettings.setAccount(account);

    ResourceValues<QString> accountName;
    ResourceValues<int> accountId;
    ResourceValues<bool> enableIntervalCheck;
    ResourceValues<int> intervalCheck{60, 60};

    if (oldInstances.calendarResource.isValid()) {
        const auto calendarSettings = settingsForResource(oldInstances.calendarResource);
        // Calendar-specific
        resourceSettings.setCalendars(calendarSettings->value(QStringLiteral("Calendars")).toStringList());
        resourceSettings.setTaskLists(calendarSettings->value(QStringLiteral("TaskLists")).toStringList());
        resourceSettings.setEventsSince(calendarSettings->value(QStringLiteral("EventsSince")).toString());

        enableIntervalCheck.calendar = calendarSettings->value(QStringLiteral("EnableIntervalCheck"), false).toBool();
        intervalCheck.calendar = calendarSettings->value(QStringLiteral("IntervalCheckTime"), 60).toInt();

        accountName.calendar = calendarSettings->value(QStringLiteral("AccountName")).toString();
        accountId.calendar = calendarSettings->value(QStringLiteral("AccountId"), 0).toInt();
    }

    if (oldInstances.contactResource.isValid()) {
        const auto contactsSettings = settingsForResource(oldInstances.contactResource);

        enableIntervalCheck.contacts = contactsSettings->value(QStringLiteral("EnableIntervalCheck"), false).toBool();
        intervalCheck.contacts = contactsSettings->value(QStringLiteral("IntervalCheckTime"), 60).toInt();

        accountName.contacts = contactsSettings->value(QStringLiteral("AccountName")).toString();
        accountId.contacts = contactsSettings->value(QStringLiteral("AccountId"), 0).toInt();
    }

    // And now some merging:
    resourceSettings.setEnableIntervalCheck(enableIntervalCheck.calendar || enableIntervalCheck.contacts);
    resourceSettings.setIntervalCheckTime(std::min(intervalCheck.calendar, intervalCheck.contacts));

    resourceSettings.setAccountName(mergeAccountNames(accountName, oldInstances));
    resourceSettings.setAccountId(mergeAccountIds(accountId, oldInstances));

    resourceSettings.save();

    return true;
}
