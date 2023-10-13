/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "settings.h"
#include "settingsadaptor.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif
using namespace QKeychain;
#include "imapaccount.h"
#include <config-imap.h>

#include <KWallet>
using KWallet::Wallet;

#include "imapresource_debug.h"

#include <QDBusConnection>

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>

/**
 * Maps the enum used to represent authentication in MailTransport (kdepimlibs)
 * to the one used by the imap resource.
 * @param authType the MailTransport auth enum value
 * @return the corresponding KIMAP auth value.
 * @note will cause fatal error if there is no mapping, so be careful not to pass invalid auth options (e.g., APOP) to this function.
 */
KIMAP::LoginJob::AuthenticationMode Settings::mapTransportAuthToKimap(MailTransport::Transport::EnumAuthenticationType authType)
{
    // typedef these for readability
    using MTAuth = MailTransport::Transport::EnumAuthenticationType;
    using KIAuth = KIMAP::LoginJob;
    switch (authType) {
    case MTAuth::ANONYMOUS:
        return KIAuth::Anonymous;
    case MTAuth::PLAIN:
        return KIAuth::Plain;
    case MTAuth::NTLM:
        return KIAuth::NTLM;
    case MTAuth::LOGIN:
        return KIAuth::Login;
    case MTAuth::GSSAPI:
        return KIAuth::GSSAPI;
    case MTAuth::DIGEST_MD5:
        return KIAuth::DigestMD5;
    case MTAuth::CRAM_MD5:
        return KIAuth::CramMD5;
    case MTAuth::CLEAR:
        return KIAuth::ClearText;
    case MTAuth::XOAUTH2:
        return KIAuth::XOAuth2;
    default:
        qFatal("mapping from Transport::EnumAuthenticationType ->  KIMAP::LoginJob::AuthenticationMode not possible");
    }
    return KIAuth::ClearText; // dummy value, shouldn't get here.
}

Settings::Settings(WId winId)
    : SettingsBase()
    , m_winId(winId)
{
    load();

    new SettingsAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Settings"),
                                                 this,
                                                 QDBusConnection::ExportAdaptors | QDBusConnection::ExportScriptableContents);
}

void Settings::setWinId(WId winId)
{
    m_winId = winId;
}

void Settings::clearCachedPassword()
{
    m_password.clear();
}

void Settings::cleanup()
{
    auto deleteJob = new DeletePasswordJob(QStringLiteral("imap"));
    deleteJob->setKey(config()->name());
    deleteJob->start();
}

void Settings::requestPassword()
{
    if (!m_password.isEmpty() || (mapTransportAuthToKimap((MailTransport::TransportBase::EnumAuthenticationType)authentication()) == KIMAP::LoginJob::GSSAPI)) {
        Q_EMIT passwordRequestCompleted(m_password, false);
    } else {
        // Already async. Just port to ReadPassword
        Wallet *wallet = Wallet::openWallet(Wallet::NetworkWallet(), m_winId, Wallet::Asynchronous);
        if (wallet) {
            connect(wallet, &KWallet::Wallet::walletOpened, this, &Settings::onWalletOpened);
        } else {
            QMetaObject::invokeMethod(this, "onWalletOpened", Qt::QueuedConnection, Q_ARG(bool, true));
        }
    }
}

void Settings::onWalletOpened(bool success)
{
    if (!success) {
        Q_EMIT passwordRequestCompleted(QString(), true);
    } else {
        auto wallet = qobject_cast<Wallet *>(sender());
        bool passwordNotStoredInWallet = true;
        if (wallet && wallet->hasFolder(QStringLiteral("imap"))) {
            wallet->setFolder(QStringLiteral("imap"));
            wallet->readPassword(config()->name(), m_password);
            passwordNotStoredInWallet = false;
        }

        Q_EMIT passwordRequestCompleted(m_password, passwordNotStoredInWallet);

        if (wallet) {
            wallet->deleteLater();
        }
    }
}

QString Settings::password(bool *userRejected) const
{
    if (userRejected != nullptr) {
        *userRejected = false;
    }

    if (!m_password.isEmpty() || (mapTransportAuthToKimap((MailTransport::TransportBase::EnumAuthenticationType)authentication()) == KIMAP::LoginJob::GSSAPI)) {
        return m_password;
    }
    // Move as async
    Wallet *wallet = Wallet::openWallet(Wallet::NetworkWallet(), m_winId);
    if (wallet && wallet->isOpen()) {
        if (wallet->hasFolder(QStringLiteral("imap"))) {
            wallet->setFolder(QStringLiteral("imap"));
            wallet->readPassword(config()->name(), m_password);
        } else {
            wallet->createFolder(QStringLiteral("imap"));
        }
    } else if (userRejected != nullptr) {
        *userRejected = true;
    }
    delete wallet;
    return m_password;
}

QString Settings::sieveCustomPassword(bool *userRejected) const
{
    if (userRejected != nullptr) {
        *userRejected = false;
    }

    if (!m_customSievePassword.isEmpty()) {
        return m_customSievePassword;
    }

    // Move as async
    Wallet *wallet = Wallet::openWallet(Wallet::NetworkWallet(), m_winId);
    if (wallet && wallet->isOpen()) {
        if (wallet->hasFolder(QStringLiteral("imap"))) {
            wallet->setFolder(QStringLiteral("imap"));
            wallet->readPassword(QStringLiteral("custom_sieve_") + config()->name(), m_customSievePassword);
        } else {
            wallet->createFolder(QStringLiteral("imap"));
        }
    } else if (userRejected != nullptr) {
        *userRejected = true;
    }
    delete wallet;
    return m_customSievePassword;
}

void Settings::setSieveCustomPassword(const QString &password)
{
    if (m_customSievePassword == password) {
        return;
    }
    m_customSievePassword = password;
    Wallet *wallet = Wallet::openWallet(Wallet::NetworkWallet(), m_winId);
    if (wallet && wallet->isOpen()) {
        if (!wallet->hasFolder(QStringLiteral("imap"))) {
            wallet->createFolder(QStringLiteral("imap"));
        }
        wallet->setFolder(QStringLiteral("imap"));
        wallet->writePassword(QLatin1String("custom_sieve_") + config()->name(), password);
        qCDebug(IMAPRESOURCE_LOG) << "Wallet save: " << wallet->sync();
    }
    delete wallet;
}

void Settings::setPassword(const QString &password)
{
    if (password == m_password) {
        return;
    }

    if (mapTransportAuthToKimap((MailTransport::TransportBase::EnumAuthenticationType)authentication()) == KIMAP::LoginJob::GSSAPI) {
        return;
    }

    m_password = password;
    Wallet *wallet = Wallet::openWallet(Wallet::NetworkWallet(), m_winId);
    if (wallet && wallet->isOpen()) {
        if (!wallet->hasFolder(QStringLiteral("imap"))) {
            wallet->createFolder(QStringLiteral("imap"));
        }
        wallet->setFolder(QStringLiteral("imap"));
        wallet->writePassword(config()->name(), password);
        qCDebug(IMAPRESOURCE_LOG) << "Wallet save: " << wallet->sync();
    }
    delete wallet;
}

void Settings::loadAccount(ImapAccount *account) const
{
    account->setServer(imapServer());
    if (imapPort() >= 0) {
        account->setPort(imapPort());
    }

    account->setUserName(userName());
    account->setSubscriptionEnabled(subscriptionEnabled());
    account->setUseNetworkProxy(useProxy());

    const QString encryption = safety();
    if (encryption == QLatin1String("SSL")) {
        account->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
    } else if (encryption == QLatin1String("STARTTLS")) {
        account->setEncryptionMode(KIMAP::LoginJob::STARTTLS);
    } else {
        account->setEncryptionMode(KIMAP::LoginJob::Unencrypted);
    }

    // Some SSL Server fail to advertise an ssl version they support (AnySslVersion),
    // we therefore allow overriding this in the config
    //(so we don't have to make the UI unnecessarily complex for properly working servers).
    const QString overrideEncryptionMode = overrideEncryption();
    if (!overrideEncryptionMode.isEmpty()) {
        qCWarning(IMAPRESOURCE_LOG) << "Overriding encryption mode with: " << overrideEncryptionMode;
        if (overrideEncryptionMode == QLatin1String("SSLV2")) {
            account->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
        } else if (overrideEncryptionMode == QLatin1String("SSLV3")) {
            account->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
        } else if (overrideEncryptionMode == QLatin1String("TLSV1")) {
            account->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
        } else if (overrideEncryptionMode == QLatin1String("SSL")) {
            account->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
        } else if (overrideEncryptionMode == QLatin1String("STARTTLS")) {
            account->setEncryptionMode(KIMAP::LoginJob::STARTTLS);
        } else if (overrideEncryptionMode == QLatin1String("UNENCRYPTED")) {
            account->setEncryptionMode(KIMAP::LoginJob::Unencrypted);
        } else {
            qCWarning(IMAPRESOURCE_LOG) << "Tried to force invalid encryption mode: " << overrideEncryptionMode;
        }
    }

    account->setAuthenticationMode(mapTransportAuthToKimap(static_cast<MailTransport::TransportBase::EnumAuthenticationType>(authentication())));

    account->setTimeout(sessionTimeout());
}

QString Settings::rootRemoteId() const
{
    return QStringLiteral("imap://") + userName() + QLatin1Char('@') + imapServer() + QLatin1Char('/');
}

void Settings::renameRootCollection(const QString &newName)
{
    Akonadi::Collection rootCollection;
    rootCollection.setRemoteId(rootRemoteId());
    auto fetchJob = new Akonadi::CollectionFetchJob(rootCollection, Akonadi::CollectionFetchJob::Base);
    fetchJob->setProperty("collectionName", newName);
    connect(fetchJob, &KJob::result, this, &Settings::onRootCollectionFetched);
}

void Settings::onRootCollectionFetched(KJob *job)
{
    const QString newName = job->property("collectionName").toString();
    Q_ASSERT(!newName.isEmpty());
    auto fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
    if (fetchJob->collections().size() == 1) {
        Akonadi::Collection rootCollection = fetchJob->collections().at(0);
        rootCollection.setName(newName);
        new Akonadi::CollectionModifyJob(rootCollection);
        // We don't care about the result here, nothing we can/should do if the renaming fails
    }
}
