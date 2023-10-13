/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
   SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2006-2008 Omat Holding B.V. <info@omat.nl>
   SPDX-FileCopyrightText: 2006 Frode M. Døving <frode@lnix.net>

   Original copied from showfoto:
    SPDX-FileCopyrightText: 2005 Gilles Caulier <caulier.gilles@free.fr>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "setupserver.h"
#include "folderarchivesettingpage.h"
#include "imapresource.h"
#include "serverinfodialog.h"
#include "settings.h"
#include <config-imap.h>

#include <Libkdepim/LineEditCatchReturnKey>
#include <MailTransport/ServerTest>
#include <MailTransport/Transport>

#include <KMime/Message>

#include "imapresource_debug.h"
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ResourceSettings>
#include <Akonadi/SpecialMailCollections>
#include <Akonadi/SpecialMailCollectionsRequestJob>
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUser>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QNetworkInformation>
#else
#include <QNetworkConfigurationManager>
#endif
#include <QPushButton>

#include <KIdentityManagement/IdentityCombo>
#include <KIdentityManagement/IdentityManager>
#include <QFontDatabase>
#include <QPointer>
#include <QVBoxLayout>

#include "imapaccount.h"
#include "subscriptiondialog.h"

#include "ui_setupserverview_desktop.h"

/** static helper functions **/
static QString authenticationModeString(MailTransport::Transport::EnumAuthenticationType mode)
{
    switch (mode) {
    case MailTransport::Transport::EnumAuthenticationType::LOGIN:
        return QStringLiteral("LOGIN");
    case MailTransport::Transport::EnumAuthenticationType::PLAIN:
        return QStringLiteral("PLAIN");
    case MailTransport::Transport::EnumAuthenticationType::CRAM_MD5:
        return QStringLiteral("CRAM-MD5");
    case MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5:
        return QStringLiteral("DIGEST-MD5");
    case MailTransport::Transport::EnumAuthenticationType::GSSAPI:
        return QStringLiteral("GSSAPI");
    case MailTransport::Transport::EnumAuthenticationType::NTLM:
        return QStringLiteral("NTLM");
    case MailTransport::Transport::EnumAuthenticationType::CLEAR:
        return i18nc("Authentication method", "Clear text");
    case MailTransport::Transport::EnumAuthenticationType::ANONYMOUS:
        return i18nc("Authentication method", "Anonymous");
    case MailTransport::Transport::EnumAuthenticationType::XOAUTH2:
        return i18nc("Authentication method", "Gmail");
    default:
        break;
    }
    return {};
}

static void setCurrentAuthMode(QComboBox *authCombo, MailTransport::Transport::EnumAuthenticationType authtype)
{
    qCDebug(IMAPRESOURCE_LOG) << "setting authcombo: " << authenticationModeString(authtype);
    int index = authCombo->findData(authtype);
    if (index == -1) {
        qCWarning(IMAPRESOURCE_LOG) << "desired authmode not in the combo";
    }
    qCDebug(IMAPRESOURCE_LOG) << "found corresponding index: " << index << "with data"
                              << authenticationModeString((MailTransport::Transport::EnumAuthenticationType)authCombo->itemData(index).toInt());
    authCombo->setCurrentIndex(index);
    auto t = static_cast<MailTransport::Transport::EnumAuthenticationType>(authCombo->itemData(authCombo->currentIndex()).toInt());
    qCDebug(IMAPRESOURCE_LOG) << "selected auth mode:" << authenticationModeString(t);
    Q_ASSERT(t == authtype);
}

static MailTransport::Transport::EnumAuthenticationType getCurrentAuthMode(QComboBox *authCombo)
{
    auto authtype = static_cast<MailTransport::Transport::EnumAuthenticationType>(authCombo->itemData(authCombo->currentIndex()).toInt());
    qCDebug(IMAPRESOURCE_LOG) << "current auth mode: " << authenticationModeString(authtype);
    return authtype;
}

static void addAuthenticationItem(QComboBox *authCombo, MailTransport::Transport::EnumAuthenticationType authtype)
{
    qCDebug(IMAPRESOURCE_LOG) << "adding auth item " << authenticationModeString(authtype);
    authCombo->addItem(authenticationModeString(authtype), QVariant(authtype));
}

SetupServer::SetupServer(ImapResourceBase *parentResource, WId parent)
    : QDialog()
    , m_parentResource(parentResource)
    , m_ui(new Ui::SetupServerView)
    , mValidator(this)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto networkConfigMgr = new QNetworkConfigurationManager(QCoreApplication::instance());
#endif

    m_parentResource->settings()->setWinId(parent);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(mOkButton, &QPushButton::clicked, this, &SetupServer::applySettings);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SetupServer::reject);
    mainLayout->addWidget(buttonBox);

    m_ui->setupUi(mainWidget);
    m_ui->password->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
    m_ui->customPassword->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
    new KPIM::LineEditCatchReturnKey(m_ui->accountName, this);
    new KPIM::LineEditCatchReturnKey(m_ui->imapServer, this);
    new KPIM::LineEditCatchReturnKey(m_ui->userName, this);
    new KPIM::LineEditCatchReturnKey(m_ui->alternateURL, this);
    new KPIM::LineEditCatchReturnKey(m_ui->customUsername, this);

    m_folderArchiveSettingPage = new FolderArchiveSettingPage(m_parentResource->identifier());
    m_ui->tabWidget->addTab(m_folderArchiveSettingPage, i18n("Archive Folder"));
    m_ui->safeImapGroup->setId(m_ui->noRadio, KIMAP::LoginJob::Unencrypted);
    m_ui->safeImapGroup->setId(m_ui->sslRadio, KIMAP::LoginJob::SSLorTLS);
    m_ui->safeImapGroup->setId(m_ui->tlsRadio, KIMAP::LoginJob::STARTTLS);

    connect(m_ui->noRadio, &QRadioButton::toggled, this, &SetupServer::slotSafetyChanged);
    connect(m_ui->sslRadio, &QRadioButton::toggled, this, &SetupServer::slotSafetyChanged);
    connect(m_ui->tlsRadio, &QRadioButton::toggled, this, &SetupServer::slotSafetyChanged);

    m_ui->testInfo->hide();
    m_ui->testProgress->hide();
    m_ui->accountName->setFocus();
    m_ui->checkInterval->setSuffix(ki18np(" minute", " minutes"));
    m_ui->checkInterval->setMinimum(Akonadi::ResourceSettings::self()->minimumCheckInterval());
    m_ui->checkInterval->setMaximum(10000);
    m_ui->checkInterval->setSingleStep(1);
    m_ui->imapInfo->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    // regex for evaluating a valid server name/ip
    mValidator.setRegularExpression(QRegularExpression(QStringLiteral("[A-Za-z0-9_:.-]*")));
    m_ui->imapServer->setValidator(&mValidator);

    m_ui->folderRequester->setMimeTypeFilter(QStringList() << KMime::Message::mimeType());
    m_ui->folderRequester->setAccessRightsFilter(Akonadi::Collection::CanChangeItem | Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanDeleteItem);
    m_ui->folderRequester->changeCollectionDialogOptions(Akonadi::CollectionDialog::AllowToCreateNewChildCollection);
    m_identityCombobox = new KIdentityManagement::IdentityCombo(KIdentityManagement::IdentityManager::self(), this);
    m_identityCombobox->setShowDefault(true);
    m_ui->formLayoutAdvanced->insertRow(3, i18n("Identity:"), m_identityCombobox);

    connect(m_ui->testButton, &QPushButton::pressed, this, &SetupServer::slotTest);

    connect(m_ui->imapServer, &QLineEdit::textChanged, this, &SetupServer::slotServerChanged);
    connect(m_ui->imapServer, &QLineEdit::textChanged, this, &SetupServer::slotTestChanged);
    connect(m_ui->imapServer, &QLineEdit::textChanged, this, &SetupServer::slotComplete);
    connect(m_ui->userName, &QLineEdit::textChanged, this, &SetupServer::slotComplete);
    connect(m_ui->subscriptionEnabled, &QCheckBox::toggled, this, &SetupServer::slotSubcriptionCheckboxChanged);
    connect(m_ui->subscriptionButton, &QPushButton::pressed, this, &SetupServer::slotManageSubscriptions);

    connect(m_ui->managesieveCheck, &QCheckBox::toggled, this, &SetupServer::slotEnableWidgets);
    connect(m_ui->sameConfigCheck, &QCheckBox::toggled, this, &SetupServer::slotEnableWidgets);

    connect(m_ui->enableMailCheckBox, &QCheckBox::toggled, this, &SetupServer::slotMailCheckboxChanged);
    connect(m_ui->safeImapGroup, &QButtonGroup::buttonClicked, this, &SetupServer::slotEncryptionRadioChanged);
    connect(m_ui->customSieveGroup, &QButtonGroup::buttonClicked, this, &SetupServer::slotCustomSieveChanged);
    connect(m_ui->showServerInfo, &QPushButton::pressed, this, &SetupServer::slotShowServerInfo);

    readSettings();
    slotTestChanged();
    slotComplete();
    slotCustomSieveChanged();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    connect(networkConfigMgr, &QNetworkConfigurationManager::onlineStateChanged, this, &SetupServer::slotTestChanged);
    QT_WARNING_POP
#else
    QNetworkInformation::instance()->loadBackendByFeatures(QNetworkInformation::Feature::Reachability);
    connect(QNetworkInformation::instance(), &QNetworkInformation::reachabilityChanged, this, [this](QNetworkInformation::Reachability newReachability) {
        slotTestChanged();
    });
#endif
}

SetupServer::~SetupServer()
{
    delete m_ui;
}

bool SetupServer::shouldClearCache() const
{
    return m_shouldClearCache;
}

void SetupServer::slotSubcriptionCheckboxChanged()
{
    m_ui->subscriptionButton->setEnabled(m_ui->subscriptionEnabled->isChecked());
}

void SetupServer::slotMailCheckboxChanged()
{
    m_ui->checkInterval->setEnabled(m_ui->enableMailCheckBox->isChecked());
}

void SetupServer::slotEncryptionRadioChanged()
{
    // TODO these really should be defined somewhere else
    switch (m_ui->safeImapGroup->checkedId()) {
    case KIMAP::LoginJob::Unencrypted:
    case KIMAP::LoginJob::STARTTLS:
        m_ui->portSpin->setValue(143);
        break;
    case KIMAP::LoginJob::SSLorTLS:
        m_ui->portSpin->setValue(993);
        break;
    default:
        qFatal("Shouldn't happen");
    }
}

void SetupServer::slotCustomSieveChanged()
{
    QAbstractButton *checkedButton = m_ui->customSieveGroup->checkedButton();

    if (checkedButton == m_ui->imapUserPassword || checkedButton == m_ui->noAuthentification) {
        m_ui->customUsername->setEnabled(false);
        m_ui->customPassword->setEnabled(false);
    } else if (checkedButton == m_ui->customUserPassword) {
        m_ui->customUsername->setEnabled(true);
        m_ui->customPassword->setEnabled(true);
    }
}

void SetupServer::applySettings()
{
    if (!m_parentResource->settings()->imapServer().isEmpty() && m_ui->imapServer->text() != m_parentResource->settings()->imapServer()) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("You have changed the address of the server. Even if this is the same server as before "
                                                    "we will have to re-download all your emails from this account again. "
                                                    "Are you sure you want to proceed?"),
                                               i18n("Server address change"))
            == KMessageBox::Cancel) {
            return;
        }
    }
    if (!m_parentResource->settings()->userName().isEmpty() && m_ui->userName->text() != m_parentResource->settings()->userName()) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("You have changed the user name. Even if this is a user name for the same account as before "
                                                    "we will have to re-download all your emails from this account again. "
                                                    "Are you sure you want to proceed?"),
                                               i18n("User name change"))
            == KMessageBox::Cancel) {
            return;
        }
    }

    m_folderArchiveSettingPage->writeSettings();
    m_shouldClearCache =
        (m_parentResource->settings()->imapServer() != m_ui->imapServer->text()) || (m_parentResource->settings()->userName() != m_ui->userName->text());

    const MailTransport::Transport::EnumAuthenticationType authtype = getCurrentAuthMode(m_ui->authenticationCombo);
    if (!m_ui->userName->text().contains(QLatin1Char('@')) && authtype == MailTransport::Transport::EnumAuthenticationType::XOAUTH2
        && m_ui->imapServer->text().contains(QLatin1String("gmail.com"))) {
        // Normalize gmail username so that it matches the JSON account info returned by GMail authentication.
        // If we don't do this, we will look up cached auth without @gmail.com and save it with @gmail.com => very frequent auth dialog popping up.
        qCDebug(IMAPRESOURCE_LOG) << "Fixing up username" << m_ui->userName->text() << "by adding @gmail.com";
        m_ui->userName->setText(m_ui->userName->text() + QLatin1String("@gmail.com"));
    }

    m_parentResource->setName(m_ui->accountName->text());

    m_parentResource->settings()->setImapServer(m_ui->imapServer->text());
    m_parentResource->settings()->setImapPort(m_ui->portSpin->value());
    m_parentResource->settings()->setUserName(m_ui->userName->text());
    QString encryption;
    switch (m_ui->safeImapGroup->checkedId()) {
    case KIMAP::LoginJob::Unencrypted:
        encryption = QStringLiteral("None");
        break;
    case KIMAP::LoginJob::SSLorTLS:
        encryption = QStringLiteral("SSL");
        break;
    case KIMAP::LoginJob::STARTTLS:
        encryption = QStringLiteral("STARTTLS");
        break;
    default:
        qFatal("Shouldn't happen");
    }
    m_parentResource->settings()->setSafety(encryption);

    qCDebug(IMAPRESOURCE_LOG) << "saving IMAP auth mode: " << authenticationModeString(authtype);
    m_parentResource->settings()->setAuthentication(authtype);
    m_parentResource->settings()->setPassword(m_ui->password->password());
    m_parentResource->settings()->setSubscriptionEnabled(m_ui->subscriptionEnabled->isChecked());
    m_parentResource->settings()->setIntervalCheckTime(m_ui->checkInterval->value());
    m_parentResource->settings()->setDisconnectedModeEnabled(m_ui->disconnectedModeEnabled->isChecked());
    m_parentResource->settings()->setUseProxy(m_ui->useProxyCheck->isChecked());

    MailTransport::Transport::EnumAuthenticationType alternateAuthtype = getCurrentAuthMode(m_ui->authenticationAlternateCombo);
    qCDebug(IMAPRESOURCE_LOG) << "saving Alternate server sieve auth mode: " << authenticationModeString(alternateAuthtype);
    m_parentResource->settings()->setAlternateAuthentication(alternateAuthtype);
    m_parentResource->settings()->setSieveSupport(m_ui->managesieveCheck->isChecked());
    m_parentResource->settings()->setSieveReuseConfig(m_ui->sameConfigCheck->isChecked());
    m_parentResource->settings()->setSievePort(m_ui->sievePortSpin->value());
    m_parentResource->settings()->setSieveAlternateUrl(m_ui->alternateURL->text());
    m_parentResource->settings()->setSieveVacationFilename(m_vacationFileName);

    m_parentResource->settings()->setTrashCollection(m_ui->folderRequester->collection().id());
    Akonadi::Collection trash = m_ui->folderRequester->collection();
    Akonadi::SpecialMailCollections::self()->registerCollection(Akonadi::SpecialMailCollections::Trash, trash);
    auto attribute = trash.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
    attribute->setIconName(QStringLiteral("user-trash"));
    new Akonadi::CollectionModifyJob(trash);
    if (mOldTrash != trash) {
        Akonadi::SpecialMailCollections::self()->unregisterCollection(mOldTrash);
    }

    m_parentResource->settings()->setAutomaticExpungeEnabled(m_ui->autoExpungeCheck->isChecked());
    m_parentResource->settings()->setUseDefaultIdentity(m_identityCombobox->isDefaultIdentity());

    if (!m_identityCombobox->isDefaultIdentity()) {
        m_parentResource->settings()->setAccountIdentity(m_identityCombobox->currentIdentity());
    }

    m_parentResource->settings()->setIntervalCheckEnabled(m_ui->enableMailCheckBox->isChecked());
    if (m_ui->enableMailCheckBox->isChecked()) {
        m_parentResource->settings()->setIntervalCheckTime(m_ui->checkInterval->value());
    }

    m_parentResource->settings()->setSieveCustomUsername(m_ui->customUsername->text());

    QAbstractButton *checkedButton = m_ui->customSieveGroup->checkedButton();

    if (checkedButton == m_ui->imapUserPassword) {
        m_parentResource->settings()->setSieveCustomAuthentification(QStringLiteral("ImapUserPassword"));
    } else if (checkedButton == m_ui->noAuthentification) {
        m_parentResource->settings()->setSieveCustomAuthentification(QStringLiteral("NoAuthentification"));
    } else if (checkedButton == m_ui->customUserPassword) {
        m_parentResource->settings()->setSieveCustomAuthentification(QStringLiteral("CustomUserPassword"));
    }

    m_parentResource->settings()->setSieveCustomPassword(m_ui->customPassword->password());

    m_parentResource->settings()->save();
    qCDebug(IMAPRESOURCE_LOG) << "wrote" << m_ui->imapServer->text() << m_ui->userName->text() << m_ui->safeImapGroup->checkedId();

    if (m_oldResourceName != m_ui->accountName->text() && !m_ui->accountName->text().isEmpty()) {
        m_parentResource->settings()->renameRootCollection(m_ui->accountName->text());
    }

    accept();
}

void SetupServer::readSettings()
{
    m_folderArchiveSettingPage->loadSettings();
    m_ui->accountName->setText(m_parentResource->name());
    m_oldResourceName = m_ui->accountName->text();

    auto currentUser = new KUser();

    m_ui->imapServer->setText(m_parentResource->settings()->imapServer());

    m_ui->portSpin->setValue(m_parentResource->settings()->imapPort());

    m_ui->userName->setText(!m_parentResource->settings()->userName().isEmpty() ? m_parentResource->settings()->userName() : currentUser->loginName());

    const QString safety = m_parentResource->settings()->safety();
    int i = 0;
    if (safety == QLatin1String("SSL")) {
        i = KIMAP::LoginJob::SSLorTLS;
    } else if (safety == QLatin1String("STARTTLS")) {
        i = KIMAP::LoginJob::STARTTLS;
    } else {
        i = KIMAP::LoginJob::Unencrypted;
    }

    QAbstractButton *safetyButton = m_ui->safeImapGroup->button(i);
    if (safetyButton) {
        safetyButton->setChecked(true);
    }

    populateDefaultAuthenticationOptions();
    i = m_parentResource->settings()->authentication();
    qCDebug(IMAPRESOURCE_LOG) << "read IMAP auth mode: " << authenticationModeString(static_cast<MailTransport::Transport::EnumAuthenticationType>(i));
    setCurrentAuthMode(m_ui->authenticationCombo, (MailTransport::Transport::EnumAuthenticationType)i);

    i = m_parentResource->settings()->alternateAuthentication();
    setCurrentAuthMode(m_ui->authenticationAlternateCombo, static_cast<MailTransport::Transport::EnumAuthenticationType>(i));

    bool rejected = false;
    const QString password = m_parentResource->settings()->password(&rejected);
    if (rejected) {
        m_ui->password->setEnabled(false);
        KMessageBox::information(nullptr,
                                 i18n("Could not access KWallet. "
                                      "If you want to store the password permanently then you have to "
                                      "activate it. If you do not "
                                      "want to use KWallet, check the box below, but note that you will be "
                                      "prompted for your password when needed."),
                                 i18n("Do not use KWallet"),
                                 QStringLiteral("warning_kwallet_disabled"));
    } else {
        m_ui->password->lineEdit()->insert(password);
    }

    m_ui->subscriptionEnabled->setChecked(m_parentResource->settings()->subscriptionEnabled());

    m_ui->checkInterval->setValue(m_parentResource->settings()->intervalCheckTime());
    m_ui->disconnectedModeEnabled->setChecked(m_parentResource->settings()->disconnectedModeEnabled());
    m_ui->useProxyCheck->setChecked(m_parentResource->settings()->useProxy());

    m_ui->managesieveCheck->setChecked(m_parentResource->settings()->sieveSupport());
    m_ui->sameConfigCheck->setChecked(m_parentResource->settings()->sieveReuseConfig());
    m_ui->sievePortSpin->setValue(m_parentResource->settings()->sievePort());
    m_ui->alternateURL->setText(m_parentResource->settings()->sieveAlternateUrl());
    m_vacationFileName = m_parentResource->settings()->sieveVacationFilename();

    Akonadi::Collection trashCollection(m_parentResource->settings()->trashCollection());
    if (trashCollection.isValid()) {
        auto fetchJob = new Akonadi::CollectionFetchJob(trashCollection, Akonadi::CollectionFetchJob::Base, this);
        connect(fetchJob, &Akonadi::CollectionFetchJob::collectionsReceived, this, &SetupServer::targetCollectionReceived);
    } else {
        auto requestJob = new Akonadi::SpecialMailCollectionsRequestJob(this);
        connect(requestJob, &Akonadi::SpecialMailCollectionsRequestJob::result, this, &SetupServer::localFolderRequestJobFinished);
        requestJob->requestDefaultCollection(Akonadi::SpecialMailCollections::Trash);
        requestJob->start();
    }

    m_identityCombobox->setCurrentIdentity(m_parentResource->settings()->accountIdentity());

    m_ui->enableMailCheckBox->setChecked(m_parentResource->settings()->intervalCheckEnabled());
    if (m_ui->enableMailCheckBox->isChecked()) {
        m_ui->checkInterval->setValue(m_parentResource->settings()->intervalCheckTime());
    } else {
        m_ui->checkInterval->setEnabled(false);
    }

    m_ui->autoExpungeCheck->setChecked(m_parentResource->settings()->automaticExpungeEnabled());

    if (m_vacationFileName.isEmpty()) {
        m_vacationFileName = QStringLiteral("kmail-vacation.siv");
    }

    m_ui->customUsername->setText(m_parentResource->settings()->sieveCustomUsername());

    rejected = false;
    const QString customPassword = m_parentResource->settings()->sieveCustomPassword(&rejected);
    if (rejected) {
        m_ui->customPassword->setEnabled(false);
        KMessageBox::information(nullptr,
                                 i18n("Could not access KWallet. "
                                      "If you want to store the password permanently then you have to "
                                      "activate it. If you do not "
                                      "want to use KWallet, check the box below, but note that you will be "
                                      "prompted for your password when needed."),
                                 i18n("Do not use KWallet"),
                                 QStringLiteral("warning_kwallet_disabled"));
    } else {
        m_ui->customPassword->lineEdit()->insert(customPassword);
    }

    const QString sieverCustomAuth(m_parentResource->settings()->sieveCustomAuthentification());
    if (sieverCustomAuth == QLatin1String("ImapUserPassword")) {
        m_ui->imapUserPassword->setChecked(true);
    } else if (sieverCustomAuth == QLatin1String("NoAuthentification")) {
        m_ui->noAuthentification->setChecked(true);
    } else if (sieverCustomAuth == QLatin1String("CustomUserPassword")) {
        m_ui->customUserPassword->setChecked(true);
    }

    delete currentUser;
}

void SetupServer::slotTest()
{
    qCDebug(IMAPRESOURCE_LOG) << m_ui->imapServer->text();

    m_ui->testButton->setEnabled(false);
    m_ui->advancedTab->setEnabled(false);
    m_ui->authenticationCombo->setEnabled(false);

    m_ui->testInfo->clear();
    m_ui->testInfo->hide();

    delete m_serverTest;
    m_serverTest = new MailTransport::ServerTest(this);
#ifndef QT_NO_CURSOR
    qApp->setOverrideCursor(Qt::BusyCursor);
#endif

    const QString server = m_ui->imapServer->text();
    const int port = m_ui->portSpin->value();
    qCDebug(IMAPRESOURCE_LOG) << "server: " << server << "port: " << port;

    m_serverTest->setServer(server);

    if (port != 143 && port != 993) {
        m_serverTest->setPort(MailTransport::Transport::EnumEncryption::None, port);
        m_serverTest->setPort(MailTransport::Transport::EnumEncryption::SSL, port);
    }

    m_serverTest->setProtocol(QStringLiteral("imap"));
    m_serverTest->setProgressBar(m_ui->testProgress);
    connect(m_serverTest, &MailTransport::ServerTest::finished, this, &SetupServer::slotFinished);
    mOkButton->setEnabled(false);
    m_serverTest->start();
}

void SetupServer::slotFinished(const QVector<int> &testResult)
{
    qCDebug(IMAPRESOURCE_LOG) << testResult;

#ifndef QT_NO_CURSOR
    qApp->restoreOverrideCursor();
#endif
    mOkButton->setEnabled(true);

    using namespace MailTransport;

    if (!m_serverTest->isNormalPossible() && !m_serverTest->isSecurePossible()) {
        KMessageBox::error(this, i18n("Unable to connect to the server, please verify the server address."));
    }

    m_ui->testInfo->show();

    m_ui->sslRadio->setEnabled(testResult.contains(Transport::EnumEncryption::SSL));
    m_ui->tlsRadio->setEnabled(testResult.contains(Transport::EnumEncryption::TLS));
    m_ui->noRadio->setEnabled(testResult.contains(Transport::EnumEncryption::None));

    QString text;
    if (testResult.contains(Transport::EnumEncryption::TLS)) {
        m_ui->tlsRadio->setChecked(true);
        text = i18n("<qt><b>STARTTLS is supported and recommended.</b></qt>");
    } else if (testResult.contains(Transport::EnumEncryption::SSL)) {
        m_ui->sslRadio->setChecked(true);
        text = i18n("<qt><b>SSL/TLS is supported and recommended.</b></qt>");
    } else if (testResult.contains(Transport::EnumEncryption::None)) {
        m_ui->noRadio->setChecked(true);
        text = i18n(
            "<qt><b>No security is supported. It is not "
            "recommended to connect to this server.</b></qt>");
    } else {
        text = i18n("<qt><b>It is not possible to use this server.</b></qt>");
    }
    m_ui->testInfo->setText(text);

    m_ui->testButton->setEnabled(true);
    m_ui->advancedTab->setEnabled(true);
    m_ui->authenticationCombo->setEnabled(true);
    slotEncryptionRadioChanged();
    slotSafetyChanged();
}

void SetupServer::slotTestChanged()
{
    delete m_serverTest;
    m_serverTest = nullptr;
    slotSafetyChanged();

    // do not use imapConnectionPossible, as the data is not yet saved.
    m_ui->testButton->setEnabled(true /* TODO && Global::connectionPossible() ||
                                                m_ui->imapServer->text() == "localhost"*/);
}

void SetupServer::slotEnableWidgets()
{
    const bool haveSieve = m_ui->managesieveCheck->isChecked();
    const bool reuseConfig = m_ui->sameConfigCheck->isChecked();

    m_ui->sameConfigCheck->setEnabled(haveSieve);
    m_ui->sievePortSpin->setEnabled(haveSieve);
    m_ui->alternateURL->setEnabled(haveSieve && !reuseConfig);
    m_ui->authentication->setEnabled(haveSieve && !reuseConfig);
}

void SetupServer::slotComplete()
{
    const bool ok = !m_ui->imapServer->text().isEmpty() && !m_ui->userName->text().isEmpty();
    mOkButton->setEnabled(ok);
}

void SetupServer::slotSafetyChanged()
{
    if (m_serverTest == nullptr) {
        return;
    }
    QVector<int> protocols;

    switch (m_ui->safeImapGroup->checkedId()) {
    case KIMAP::LoginJob::Unencrypted:
        qCDebug(IMAPRESOURCE_LOG) << "safeImapGroup: unencrypted";
        protocols = m_serverTest->normalProtocols();
        break;
    case KIMAP::LoginJob::SSLorTLS:
        protocols = m_serverTest->secureProtocols();
        qCDebug(IMAPRESOURCE_LOG) << "safeImapGroup: SSL";
        break;
    case KIMAP::LoginJob::STARTTLS:
        protocols = m_serverTest->tlsProtocols();
        qCDebug(IMAPRESOURCE_LOG) << "safeImapGroup: starttls";
        break;
    default:
        qFatal("Shouldn't happen");
    }

    m_ui->authenticationCombo->clear();
    addAuthenticationItem(m_ui->authenticationCombo, MailTransport::Transport::EnumAuthenticationType::CLEAR);
    for (int prot : std::as_const(protocols)) {
        addAuthenticationItem(m_ui->authenticationCombo, (MailTransport::Transport::EnumAuthenticationType)prot);
    }
    if (protocols.isEmpty()) {
        qCDebug(IMAPRESOURCE_LOG) << "no authmodes found";
    } else {
        setCurrentAuthMode(m_ui->authenticationCombo, (MailTransport::Transport::EnumAuthenticationType)protocols.first());
    }
}

void SetupServer::slotManageSubscriptions()
{
    qCDebug(IMAPRESOURCE_LOG) << "manage subscripts";
    ImapAccount account;

    account.setServer(m_ui->imapServer->text());
    account.setPort(m_ui->portSpin->value());

    account.setUserName(m_ui->userName->text());
    account.setSubscriptionEnabled(m_ui->subscriptionEnabled->isChecked());

    account.setEncryptionMode(static_cast<KIMAP::LoginJob::EncryptionMode>(m_ui->safeImapGroup->checkedId()));

    account.setAuthenticationMode(Settings::mapTransportAuthToKimap(getCurrentAuthMode(m_ui->authenticationCombo)));

    QPointer<SubscriptionDialog> subscriptions = new SubscriptionDialog(this);
    subscriptions->setWindowTitle(i18nc("@title:window", "Serverside Subscription"));
    subscriptions->setWindowIcon(QIcon::fromTheme(QStringLiteral("network-server")));
    subscriptions->connectAccount(account, m_ui->password->password());
    m_subscriptionsChanged = subscriptions->isSubscriptionChanged();

    subscriptions->exec();
    delete subscriptions;

    m_ui->subscriptionEnabled->setChecked(account.isSubscriptionEnabled());
}

void SetupServer::slotShowServerInfo()
{
    auto dialog = new ServerInfoDialog(m_parentResource, this);
    dialog->show();
}

void SetupServer::targetCollectionReceived(const Akonadi::Collection::List &collections)
{
    m_ui->folderRequester->setCollection(collections.first());
    mOldTrash = collections.first();
}

void SetupServer::localFolderRequestJobFinished(KJob *job)
{
    if (!job->error()) {
        Akonadi::Collection targetCollection = Akonadi::SpecialMailCollections::self()->defaultCollection(Akonadi::SpecialMailCollections::Trash);
        Q_ASSERT(targetCollection.isValid());
        m_ui->folderRequester->setCollection(targetCollection);
        mOldTrash = targetCollection;
    }
}

void SetupServer::populateDefaultAuthenticationOptions()
{
    populateDefaultAuthenticationOptions(m_ui->authenticationCombo);
    populateDefaultAuthenticationOptions(m_ui->authenticationAlternateCombo);
}

void SetupServer::populateDefaultAuthenticationOptions(QComboBox *combo)
{
    combo->clear();
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::CLEAR);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::LOGIN);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::PLAIN);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::CRAM_MD5);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::NTLM);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::GSSAPI);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::ANONYMOUS);
    addAuthenticationItem(combo, MailTransport::Transport::EnumAuthenticationType::XOAUTH2);
}

void SetupServer::slotServerChanged()
{
    populateDefaultAuthenticationOptions(m_ui->authenticationCombo);
}
