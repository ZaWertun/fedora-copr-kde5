/*
 * SPDX-FileCopyrightText: 2020 Shashwat Jolly <shashwat.jolly@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "setupwizard.h"

#include <KAuthorized>
#include <KLocalizedString>
#include <KPasswordLineEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "etesync_debug.h"
#include "loginjob.h"

using namespace EteSyncAPI;

SetupWizard::SetupWizard(EteSyncClientState *clientState, QWidget *parent)
    : QWizard(parent)
    , mClientState(clientState)
{
    setWindowTitle(i18nc("@title:window", "EteSync configuration wizard"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("akonadi-etesync")));
    setPage(W_LoginPage, new LoginPage);

    disconnect(button(QWizard::NextButton), SIGNAL(clicked()), this, SLOT(next()));
    connect(button(QWizard::NextButton), SIGNAL(clicked()), this, SLOT(manualNext()));
    disconnect(button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(accept()));
    connect(button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(manualNext()));
}

void SetupWizard::manualNext()
{
    qCDebug(ETESYNC_LOG) << "Manual next";
    if (currentId() == W_LoginPage) {
        static_cast<LoginPage *>(page(W_LoginPage))->showProgressBar();
        const QString username = field(QStringLiteral("credentialsUserName")).toString();
        const QString password = field(QStringLiteral("credentialsPassword")).toString();
        const QString advancedServerUrl = field(QStringLiteral("credentialsServerUrl")).toString();
        const QString serverUrl = advancedServerUrl.isEmpty() ? QString::fromUtf8(etebase_get_default_server_url()) : advancedServerUrl;
        auto job = new LoginJob(mClientState, serverUrl, username, password, this);
        connect(job, &LoginJob::finished, this, [this](KJob *job) {
            qCDebug(ETESYNC_LOG) << "Login finished";
            static_cast<LoginPage *>(page(W_LoginPage))->setLoginResult(static_cast<LoginJob *>(job)->getLoginResult());
            static_cast<LoginPage *>(page(W_LoginPage))->setAccountStatusResult(static_cast<LoginJob *>(job)->getAccountStatusResult());
            static_cast<LoginPage *>(page(W_LoginPage))->setErrorCode(job->error());
            static_cast<LoginPage *>(page(W_LoginPage))->setErrorMessage(job->errorText());
            static_cast<LoginPage *>(page(W_LoginPage))->hideProgressBar();
            nextId() == -1 ? QWizard::accept() : QWizard::next();
        });
        job->start();
        return;
    }
    nextId() == -1 ? QWizard::accept() : QWizard::next();
}

LoginPage::LoginPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18n("Login Credentials"));
    setSubTitle(i18n("Enter your credentials to login to the EteSync server"));

    QFormLayout *layout = new QFormLayout(this);

    mLoginLabel = new QLabel(this);
    mLoginLabel->setWordWrap(true);
    layout->addWidget(mLoginLabel);

    mUserName = new QLineEdit(this);
    layout->addRow(i18n("Username:"), mUserName);
    registerField(QStringLiteral("credentialsUserName*"), mUserName);

    mPassword = new KPasswordLineEdit(this);
    mPassword->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
    layout->addRow(i18n("Password:"), mPassword);
    registerField(QStringLiteral("credentialsPassword*"), mPassword, "password", SIGNAL(passwordChanged(QString)));

    mAdvancedSettings = new QCheckBox(i18n("Advanced settings"), this);
    layout->addWidget(mAdvancedSettings);

    mServerUrl = new QLineEdit(this);
    mServerUrl->setVisible(false);
    layout->addRow(i18n("Server:"), mServerUrl);
    registerField(QStringLiteral("credentialsServerUrl"), mServerUrl);

    layout->labelForField(mServerUrl)->setVisible(false);

    mProgressBar = new QProgressBar(this);
    mProgressBar->setVisible(false);
    mProgressBar->setRange(0, 0);
    layout->addWidget(mProgressBar);

    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    mPasswordInfo = new QLabel(this);
    mPasswordInfo->setText(i18n("🛈 The password field is empty because EteSync does not store it for security reasons."));
    mPasswordInfo->setWordWrap(true);
    layout->addRow(mPasswordInfo);
    mPasswordInfo->setVisible(false);

    connect(mAdvancedSettings, SIGNAL(toggled(bool)), mServerUrl, SLOT(setVisible(bool)));
    connect(mAdvancedSettings, SIGNAL(toggled(bool)), layout->labelForField(mServerUrl), SLOT(setVisible(bool)));
}

void LoginPage::initializePage()
{
    mIsInitialized = static_cast<SetupWizard *>(wizard())->mClientState->isInitialized();
    qCDebug(ETESYNC_LOG) << "Login page - isInitialized" << mIsInitialized;
    if (mIsInitialized) {
        mAdvancedSettings->setVisible(false);
        setField(QStringLiteral("credentialsServerUrl"), static_cast<SetupWizard *>(wizard())->mClientState->serverUrl());
        const QString username = static_cast<SetupWizard *>(wizard())->mClientState->username();
        mUserName->setText(username);
        mPasswordInfo->setVisible(true);
    }
}

int LoginPage::nextId() const
{
    return -1;
}

bool LoginPage::validatePage()
{
    if (!mLoginResult) {
        qCDebug(ETESYNC_LOG) << "loginResult error" << mErrorCode;
        if (mErrorCode == EtebaseErrorCode::ETEBASE_ERROR_CODE_URL_PARSE) {
            mLoginLabel->setText(i18n("Please ensure that the server URL is correct. The URL should start with http:// or https://."));
        } else if (mErrorCode == EtebaseErrorCode::ETEBASE_ERROR_CODE_CONNECTION) {
            mLoginLabel->setText(i18n("Could not connect to the server. Please ensure that the server URL is correct."));
        } else if (mErrorCode == EtebaseErrorCode::ETEBASE_ERROR_CODE_NOT_FOUND) {
            mLoginLabel->setText(i18n("EteSync server not found. Please ensure that the server URL is correct."));
        } else {
            mLoginLabel->setText(i18n(charArrFromQString(mErrorMessage)));
        }
        return false;
    }
    if (!mAccountStatusResult) {
        qCDebug(ETESYNC_LOG) << "accountStatus error" << mErrorCode;
        mLoginLabel->setText(i18n(charArrFromQString(mErrorMessage)));
        return false;
    }
    return true;
}
