/*
 *   SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>
 *   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>
 *   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

// Local includes
#include "accountwidget.h"
#include "settings.h"
#include "settingsadaptor.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ResourceSettings>
#include <Akonadi/SpecialMailCollections>
#include <Akonadi/SpecialMailCollectionsRequestJob>
#include <Libkdepim/LineEditCatchReturnKey>
#include <MailTransport/ServerTest>

#include "pop3resource_debug.h"
#include <KAuthorized>
#include <KMessageBox>
#include <KUser>

#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace MailTransport;
using namespace Akonadi;
using namespace QKeychain;
namespace
{
class BusyCursorHelper : public QObject
{
public:
    inline BusyCursorHelper(QObject *parent)
        : QObject(parent)
    {
#ifndef QT_NO_CURSOR
        qApp->setOverrideCursor(Qt::BusyCursor);
#endif
    }

    inline ~BusyCursorHelper() override
    {
#ifndef QT_NO_CURSOR
        qApp->restoreOverrideCursor();
#endif
    }
};
}

AccountWidget::AccountWidget(Settings &settings, const QString &identifier, QWidget *parent)
    : QWidget(parent)
    , mValidator(this)
    , mIdentifier(identifier)
    , mSettings(settings)
{
    mValidator.setRegularExpression(QRegularExpression(QStringLiteral("[A-Za-z0-9_:.-]*")));
    setupWidgets();
}

AccountWidget::~AccountWidget()
{
    delete mServerTest;
    mServerTest = nullptr;
}

void AccountWidget::setupWidgets()
{
    auto mainLayout = new QVBoxLayout(this);

    auto page = new QWidget(this);
    mainLayout->addWidget(page);

    setupUi(page);

    passwordEdit->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));

    // only letters, digits, '-', '.', ':' (IPv6) and '_' (for Windows
    // compatibility) are allowed
    hostEdit->setValidator(&mValidator);
    intervalSpin->setSuffix(ki18np(" minute", " minutes"));

    intervalSpin->setRange(ResourceSettings::self()->minimumCheckInterval(), 10000);
    intervalSpin->setSingleStep(1);
    new KPIM::LineEditCatchReturnKey(nameEdit, this);
    new KPIM::LineEditCatchReturnKey(hostEdit, this);
    new KPIM::LineEditCatchReturnKey(loginEdit, this);
    new KPIM::LineEditCatchReturnKey(precommand, this);

    connect(leaveOnServerCheck, &QCheckBox::clicked, this, &AccountWidget::slotLeaveOnServerClicked);
    connect(leaveOnServerDaysCheck, &QCheckBox::toggled, this, &AccountWidget::slotEnableLeaveOnServerDays);
    connect(leaveOnServerDaysSpin, &QSpinBox::valueChanged, this, &AccountWidget::slotLeaveOnServerDaysChanged);
    connect(leaveOnServerCountCheck, &QCheckBox::toggled, this, &AccountWidget::slotEnableLeaveOnServerCount);
    connect(leaveOnServerCountSpin, &QSpinBox::valueChanged, this, &AccountWidget::slotLeaveOnServerCountChanged);
    connect(leaveOnServerSizeCheck, &QCheckBox::toggled, this, &AccountWidget::slotEnableLeaveOnServerSize);

    connect(checkCapabilities, &QPushButton::clicked, this, &AccountWidget::slotCheckPopCapabilities);
    encryptionButtonGroup = new QButtonGroup(page);
    encryptionButtonGroup->addButton(encryptionNone, Transport::EnumEncryption::None);
    encryptionButtonGroup->addButton(encryptionSSL, Transport::EnumEncryption::SSL);
    encryptionButtonGroup->addButton(encryptionTLS, Transport::EnumEncryption::TLS);

    connect(encryptionButtonGroup, &QButtonGroup::buttonClicked, this, &AccountWidget::slotPopEncryptionChanged);
    connect(intervalCheck, &QCheckBox::toggled, this, &AccountWidget::slotEnablePopInterval);

    populateDefaultAuthenticationOptions();

    folderRequester->setMimeTypeFilter(QStringList() << QStringLiteral("message/rfc822"));
    folderRequester->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    folderRequester->changeCollectionDialogOptions(Akonadi::CollectionDialog::AllowToCreateNewChildCollection);

    connect(usePipeliningCheck, &QCheckBox::clicked, this, &AccountWidget::slotPipeliningClicked);
}

void AccountWidget::loadSettings()
{
    if (mSettings.name().isEmpty()) {
        nameEdit->setText(i18n("POP3 Account"));
    } else {
        nameEdit->setText(mSettings.name());
    }

    nameEdit->setFocus();
    loginEdit->setText(!mSettings.login().isEmpty() ? mSettings.login() : KUser().loginName());

    hostEdit->setText(mSettings.host());
    portEdit->setValue(mSettings.port());
    precommand->setText(mSettings.precommand());
    usePipeliningCheck->setChecked(mSettings.pipelining());
    leaveOnServerCheck->setChecked(mSettings.leaveOnServer());
    leaveOnServerDaysCheck->setEnabled(mSettings.leaveOnServer());
    leaveOnServerDaysCheck->setChecked(mSettings.leaveOnServerDays() >= 1);
    leaveOnServerDaysSpin->setValue(mSettings.leaveOnServerDays() >= 1 ? mSettings.leaveOnServerDays() : 7);
    leaveOnServerCountCheck->setEnabled(mSettings.leaveOnServer());
    leaveOnServerCountCheck->setChecked(mSettings.leaveOnServerCount() >= 1);
    leaveOnServerCountSpin->setValue(mSettings.leaveOnServerCount() >= 1 ? mSettings.leaveOnServerCount() : 100);
    leaveOnServerSizeCheck->setEnabled(mSettings.leaveOnServer());
    leaveOnServerSizeCheck->setChecked(mSettings.leaveOnServerSize() >= 1);
    leaveOnServerSizeSpin->setValue(mSettings.leaveOnServerSize() >= 1 ? mSettings.leaveOnServerSize() : 10);
    intervalCheck->setChecked(mSettings.intervalCheckEnabled());
    intervalSpin->setValue(mSettings.intervalCheckInterval());
    intervalSpin->setEnabled(mSettings.intervalCheckEnabled());

    const int authenticationMethod = mSettings.authenticationMethod();
    authCombo->setCurrentIndex(authCombo->findData(authenticationMethod));
    encryptionNone->setChecked(!mSettings.useSSL() && !mSettings.useTLS());
    encryptionSSL->setChecked(mSettings.useSSL());
    encryptionTLS->setChecked(mSettings.useTLS());
    proxyCheck->setChecked(mSettings.useProxy());

    slotEnableLeaveOnServerDays(leaveOnServerDaysCheck->isEnabled() ? mSettings.leaveOnServerDays() >= 1 : false);
    slotEnableLeaveOnServerCount(leaveOnServerCountCheck->isEnabled() ? mSettings.leaveOnServerCount() >= 1 : false);
    slotEnableLeaveOnServerSize(leaveOnServerSizeCheck->isEnabled() ? mSettings.leaveOnServerSize() >= 1 : false);

    // We need to fetch the collection, as the CollectionRequester needs the name
    // of it to work correctly
    Collection targetCollection(mSettings.targetCollection());
    if (targetCollection.isValid()) {
        auto fetchJob = new CollectionFetchJob(targetCollection, CollectionFetchJob::Base, this);
        connect(fetchJob, &CollectionFetchJob::collectionsReceived, this, &AccountWidget::targetCollectionReceived);
    } else {
        // FIXME: This is a bit duplicated from POP3Resource...

        // No target collection set in the config? Try requesting a default inbox
        auto requestJob = new SpecialMailCollectionsRequestJob(this);
        requestJob->requestDefaultCollection(SpecialMailCollections::Inbox);
        requestJob->start();
        connect(requestJob, &SpecialMailCollectionsRequestJob::result, this, &AccountWidget::localFolderRequestJobFinished);
    }

    passwordEdit->setEnabled(false);
    passwordLabel->setEnabled(false);
    auto readJob = new ReadPasswordJob(QStringLiteral("pop3"), this);
    connect(readJob, &QKeychain::Job::finished, this, &AccountWidget::walletOpenedForLoading);
    readJob->setKey(mIdentifier);
    readJob->start();
}

void AccountWidget::walletOpenedForLoading(QKeychain::Job *baseJob)
{
    auto job = qobject_cast<ReadPasswordJob *>(baseJob);
    Q_ASSERT(job);
    if (job->error()) {
        qCWarning(POP3RESOURCE_LOG) << "Failed to open wallet for loading the password." << job->errorString();
        passwordEdit->lineEdit()->setPlaceholderText(i18n("Unable to open wallet"));
    } else {
        passwordEdit->setPassword(job->textData());
        passwordEdit->setEnabled(true);
        passwordLabel->setEnabled(true);
    }
}

void AccountWidget::walletOpenedForSaving()
{
    if (passwordEdit->password().isEmpty()) {
        auto deleteJob = new DeletePasswordJob(QStringLiteral("pop3"));
        deleteJob->setKey(mIdentifier);
        deleteJob->start();
    } else {
        auto writeJob = new WritePasswordJob(QStringLiteral("pop3"));
        connect(writeJob, &QKeychain::Job::finished, this, [](QKeychain::Job *baseJob) {
            if (baseJob->error()) {
                qCWarning(POP3RESOURCE_LOG) << "Error writing password using QKeychain:" << baseJob->errorString();
            }
        });
        writeJob->setKey(mIdentifier);
        writeJob->setTextData(passwordEdit->password());
        writeJob->start();
    }
}

void AccountWidget::slotLeaveOnServerClicked()
{
    const bool state = leaveOnServerCheck->isChecked();
    leaveOnServerDaysCheck->setEnabled(state);
    leaveOnServerCountCheck->setEnabled(state);
    leaveOnServerSizeCheck->setEnabled(state);
    if (state) {
        if (leaveOnServerDaysCheck->isChecked()) {
            slotEnableLeaveOnServerDays(state);
        }
        if (leaveOnServerCountCheck->isChecked()) {
            slotEnableLeaveOnServerCount(state);
        }
        if (leaveOnServerSizeCheck->isChecked()) {
            slotEnableLeaveOnServerSize(state);
        }
    } else {
        slotEnableLeaveOnServerDays(state);
        slotEnableLeaveOnServerCount(state);
        slotEnableLeaveOnServerSize(state);
    }
    if (mServerTest && !mServerTest->capabilities().contains(ServerTest::UIDL) && leaveOnServerCheck->isChecked()) {
        KMessageBox::information(topLevelWidget(),
                                 i18n("The server does not seem to support unique "
                                      "message numbers, but this is a "
                                      "requirement for leaving messages on the "
                                      "server.\n"
                                      "Since some servers do not correctly "
                                      "announce their capabilities you still "
                                      "have the possibility to turn leaving "
                                      "fetched messages on the server on."));
    }
}

void AccountWidget::slotPipeliningClicked()
{
    if (usePipeliningCheck->isChecked()) {
        KMessageBox::information(topLevelWidget(),
                                 i18n("Please note that this feature can cause some POP3 servers "
                                      "that do not support pipelining to send corrupted mail;\n"
                                      "this is configurable, though, because some servers support pipelining\n"
                                      "but do not announce their capabilities. To check whether your POP3 server\n"
                                      "announces pipelining support use the \"Auto Detect\""
                                      " button at the bottom of the dialog;\n"
                                      "if your server does not announce it, but you want more speed, then\n"
                                      "you should do some testing first by sending yourself a batch\n"
                                      "of mail and downloading it."),
                                 QString(),
                                 QStringLiteral("pipelining"));
    }
}

void AccountWidget::slotPopEncryptionChanged(QAbstractButton *button)
{
    if (button) {
        const int id = encryptionButtonGroup->id(button);
        qCDebug(POP3RESOURCE_LOG) << "setting port";
        // adjust port
        if (id == Transport::EnumEncryption::SSL || portEdit->value() == 995) {
            portEdit->setValue((id == Transport::EnumEncryption::SSL) ? 995 : 110);
        }

        qCDebug(POP3RESOURCE_LOG) << "port set ";
        enablePopFeatures(); // removes invalid auth options from the combobox
    }
}

void AccountWidget::slotCheckPopCapabilities()
{
    if (hostEdit->text().isEmpty()) {
        KMessageBox::error(this,
                           i18n("Please specify a server and port on "
                                "the General tab first."));
        return;
    }
    delete mServerTest;
    mServerTest = new ServerTest(this);
    auto busyCursorHelper = new BusyCursorHelper(mServerTest);
    mServerTest->setProgressBar(checkCapabilitiesProgress);
    Q_EMIT okEnabled(false);
    checkCapabilitiesStack->setCurrentIndex(1);
    Transport::EnumEncryption encryptionType;
    if (encryptionSSL->isChecked()) {
        encryptionType = Transport::EnumEncryption::SSL;
    } else {
        encryptionType = Transport::EnumEncryption::None;
    }
    mServerTest->setPort(encryptionType, portEdit->value());
    mServerTest->setServer(hostEdit->text());
    mServerTest->setProtocol(QStringLiteral("pop"));
    connect(mServerTest, &MailTransport::ServerTest::finished, this, &AccountWidget::slotPopCapabilities);
    connect(mServerTest, &MailTransport::ServerTest::finished, busyCursorHelper, &BusyCursorHelper::deleteLater);

    mServerTest->start();
    mServerTestFailed = false;
}

void AccountWidget::slotPopCapabilities(const QVector<int> &encryptionTypes)
{
    checkCapabilitiesStack->setCurrentIndex(0);
    Q_EMIT okEnabled(true);

    // if both fail, popup a dialog
    if (!mServerTest->isNormalPossible() && !mServerTest->isSecurePossible()) {
        KMessageBox::error(this, i18n("Unable to connect to the server, please verify the server address."));
    }

    // If the servertest did not find any usable authentication modes, assume the
    // connection failed and don't disable any of the radioboxes.
    if (encryptionTypes.isEmpty()) {
        mServerTestFailed = true;
        return;
    }

    encryptionNone->setEnabled(encryptionTypes.contains(Transport::EnumEncryption::None));
    encryptionSSL->setEnabled(encryptionTypes.contains(Transport::EnumEncryption::SSL));
    encryptionTLS->setEnabled(encryptionTypes.contains(Transport::EnumEncryption::TLS));

    usePipeliningCheck->setChecked(mServerTest->capabilities().contains(ServerTest::Pipelining));

    checkHighest(encryptionButtonGroup);
}

void AccountWidget::enablePopFeatures()
{
    if (!mServerTest || mServerTestFailed) {
        return;
    }

    QVector<int> supportedAuths;
    if (encryptionButtonGroup->checkedId() == Transport::EnumEncryption::None) {
        supportedAuths = mServerTest->normalProtocols();
    }
    if (encryptionButtonGroup->checkedId() == Transport::EnumEncryption::SSL) {
        supportedAuths = mServerTest->secureProtocols();
    }
    if (encryptionButtonGroup->checkedId() == Transport::EnumEncryption::TLS) {
        supportedAuths = mServerTest->tlsProtocols();
    }

    authCombo->clear();
    for (int prot : std::as_const(supportedAuths)) {
        authCombo->addItem(Transport::authenticationTypeString(prot), prot);
    }

    if (mServerTest && !mServerTest->capabilities().contains(ServerTest::Pipelining) && usePipeliningCheck->isChecked()) {
        usePipeliningCheck->setChecked(false);
        KMessageBox::information(topLevelWidget(),
                                 i18n("The server does not seem to support "
                                      "pipelining; therefore, this option has "
                                      "been disabled.\n"
                                      "Since some servers do not correctly "
                                      "announce their capabilities you still "
                                      "have the possibility to turn pipelining "
                                      "on. But please note that this feature can "
                                      "cause some POP servers that do not "
                                      "support pipelining to send corrupt "
                                      "messages. So before using this feature "
                                      "with important mail you should first "
                                      "test it by sending yourself a larger "
                                      "number of test messages which you all "
                                      "download in one go from the POP "
                                      "server."));
    }

    if (mServerTest && !mServerTest->capabilities().contains(ServerTest::UIDL) && leaveOnServerCheck->isChecked()) {
        leaveOnServerCheck->setChecked(false);
        KMessageBox::information(topLevelWidget(),
                                 i18n("The server does not seem to support unique "
                                      "message numbers, but this is a "
                                      "requirement for leaving messages on the "
                                      "server; therefore, this option has been "
                                      "disabled.\n"
                                      "Since some servers do not correctly "
                                      "announce their capabilities you still "
                                      "have the possibility to turn leaving "
                                      "fetched messages on the server on."));
    }
}

static void addAuthenticationItem(QComboBox *combo, int authenticationType)
{
    combo->addItem(Transport::authenticationTypeString(authenticationType), QVariant(authenticationType));
}

void AccountWidget::populateDefaultAuthenticationOptions()
{
    authCombo->clear();
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::CLEAR);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::LOGIN);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::PLAIN);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::CRAM_MD5);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::DIGEST_MD5);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::NTLM);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::GSSAPI);
    addAuthenticationItem(authCombo, Transport::EnumAuthenticationType::APOP);
}

void AccountWidget::slotLeaveOnServerDaysChanged(int value)
{
    leaveOnServerDaysSpin->setSuffix(i18np(" day", " days", value));
}

void AccountWidget::slotLeaveOnServerCountChanged(int value)
{
    leaveOnServerCountSpin->setSuffix(i18np(" message", " messages", value));
}

void AccountWidget::checkHighest(QButtonGroup *btnGroup)
{
    QListIterator<QAbstractButton *> it(btnGroup->buttons());
    it.toBack();
    while (it.hasPrevious()) {
        QAbstractButton *btn = it.previous();
        if (btn && btn->isEnabled()) {
            btn->animateClick();
            return;
        }
    }
}

void AccountWidget::slotAccepted()
{
    saveSettings();
    // Don't call accept() yet, saveSettings() triggers an asynchronous wallet operation,
    // which will call accept() when it is finished
}

void AccountWidget::saveSettings()
{
    mSettings.setName(nameEdit->text());
    mSettings.setIntervalCheckEnabled(intervalCheck->checkState() == Qt::Checked);
    mSettings.setIntervalCheckInterval(intervalSpin->value());
    mSettings.setHost(hostEdit->text().trimmed());
    mSettings.setPort(portEdit->value());
    mSettings.setLogin(loginEdit->text().trimmed());
    mSettings.setPrecommand(precommand->text());
    mSettings.setUseSSL(encryptionSSL->isChecked());
    mSettings.setUseTLS(encryptionTLS->isChecked());
    mSettings.setAuthenticationMethod(authCombo->itemData(authCombo->currentIndex()).toInt());
    mSettings.setUseProxy(proxyCheck->isChecked());
    mSettings.setPipelining(usePipeliningCheck->isChecked());
    mSettings.setLeaveOnServer(leaveOnServerCheck->isChecked());
    mSettings.setLeaveOnServerDays(leaveOnServerCheck->isChecked() ? (leaveOnServerDaysCheck->isChecked() ? leaveOnServerDaysSpin->value() : -1) : 0);
    mSettings.setLeaveOnServerCount(leaveOnServerCheck->isChecked() ? (leaveOnServerCountCheck->isChecked() ? leaveOnServerCountSpin->value() : -1) : 0);
    mSettings.setLeaveOnServerSize(leaveOnServerCheck->isChecked() ? (leaveOnServerSizeCheck->isChecked() ? leaveOnServerSizeSpin->value() : -1) : 0);
    mSettings.setTargetCollection(folderRequester->collection().id());
    mSettings.save();

    // Now, either save the password or delete it from the wallet. For both, we need
    // to open it.
    const bool userChangedPassword = mInitalPassword != passwordEdit->password();
    const bool userWantsToDeletePassword = passwordEdit->password().isEmpty() && userChangedPassword;
    // Move to async
    if ((!passwordEdit->password().isEmpty() && userChangedPassword) || userWantsToDeletePassword) {
        walletOpenedForSaving();
    }
}

void AccountWidget::slotEnableLeaveOnServerDays(bool state)
{
    if (state && !leaveOnServerDaysCheck->isEnabled()) {
        return;
    }
    leaveOnServerDaysSpin->setEnabled(state);
}

void AccountWidget::slotEnableLeaveOnServerCount(bool state)
{
    if (state && !leaveOnServerCountCheck->isEnabled()) {
        return;
    }
    leaveOnServerCountSpin->setEnabled(state);
}

void AccountWidget::slotEnableLeaveOnServerSize(bool state)
{
    if (state && !leaveOnServerSizeCheck->isEnabled()) {
        return;
    }
    leaveOnServerSizeSpin->setEnabled(state);
}

void AccountWidget::slotEnablePopInterval(bool state)
{
    intervalSpin->setEnabled(state);
    intervalLabel->setEnabled(state);
}

void AccountWidget::targetCollectionReceived(Akonadi::Collection::List collections)
{
    folderRequester->setCollection(collections.first());
}

void AccountWidget::localFolderRequestJobFinished(KJob *job)
{
    if (!job->error()) {
        Collection targetCollection = SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::Inbox);
        Q_ASSERT(targetCollection.isValid());
        folderRequester->setCollection(targetCollection);
    }
}
