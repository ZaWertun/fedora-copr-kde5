/*
    SPDX-FileCopyrightText: 2010 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "urlconfigurationdialog.h"

#include <KDAV/DavCollectionModifyJob>
#include <KDAV/DavCollectionsFetchJob>

#include <KAuthorized>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include <KWindowConfig>
#include <QButtonGroup>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWindow>

UrlConfigurationDialog::UrlConfigurationDialog(QWidget *parent)
    : QDialog(parent)
    , mModel(new QStandardItemModel(this))
    , mRemoteProtocolGroup(new QButtonGroup(this))
{
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    mUi.setupUi(mainWidget);
    mUi.credentialsGroup->setVisible(false);
    mUi.password->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UrlConfigurationDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &UrlConfigurationDialog::reject);
    mainLayout->addWidget(buttonBox);

    initModel();

    mUi.discoveredUrls->setModel(mModel);
    mUi.discoveredUrls->setRootIsDecorated(false);
    connect(mModel, &QStandardItemModel::dataChanged, this, &UrlConfigurationDialog::onModelDataChanged);

    mRemoteProtocolGroup->setExclusive(true);
    mRemoteProtocolGroup->addButton(mUi.caldav, KDAV::CalDav);
    mRemoteProtocolGroup->addButton(mUi.carddav, KDAV::CardDav);
    mRemoteProtocolGroup->addButton(mUi.groupdav, KDAV::GroupDav);
    connect(mRemoteProtocolGroup, &QButtonGroup::idToggled, this, &UrlConfigurationDialog::onConfigChanged);
    connect(mUi.remoteUrl, &QLineEdit::textChanged, this, &UrlConfigurationDialog::onConfigChanged);
    connect(mUi.useDefaultCreds, &QRadioButton::toggled, this, &UrlConfigurationDialog::onConfigChanged);
    connect(mUi.username, &QLineEdit::textChanged, this, &UrlConfigurationDialog::onConfigChanged);
    connect(mUi.password, &KPasswordLineEdit::passwordChanged, this, &UrlConfigurationDialog::onConfigChanged);

    connect(mUi.fetchButton, &QPushButton::clicked, this, &UrlConfigurationDialog::onFetchButtonClicked);
    connect(mOkButton, &QPushButton::clicked, this, &UrlConfigurationDialog::onOkButtonClicked);

    checkUserInput();
    readConfig();
}

UrlConfigurationDialog::~UrlConfigurationDialog()
{
    writeConfig();
}

void UrlConfigurationDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(300, 200));
    KConfigGroup group(KSharedConfig::openStateConfig(), "UrlConfigurationDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void UrlConfigurationDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openStateConfig(), "UrlConfigurationDialog");
    KWindowConfig::saveWindowSize(windowHandle(), grp);
    grp.sync();
}

KDAV::Protocol UrlConfigurationDialog::protocol() const
{
    return KDAV::Protocol(mRemoteProtocolGroup->id(mRemoteProtocolGroup->checkedButton()));
}

void UrlConfigurationDialog::setProtocol(KDAV::Protocol protocol)
{
    mRemoteProtocolGroup->button(protocol)->setChecked(true);
}

QString UrlConfigurationDialog::remoteUrl() const
{
    return mUi.remoteUrl->text();
}

void UrlConfigurationDialog::setRemoteUrl(const QString &url)
{
    mUi.remoteUrl->setText(url);
}

bool UrlConfigurationDialog::useDefaultCredentials() const
{
    return mUi.useDefaultCreds->isChecked();
}

void UrlConfigurationDialog::setUseDefaultCredentials(bool defaultCreds)
{
    if (defaultCreds) {
        mUi.useDefaultCreds->setChecked(true);
    } else {
        mUi.useSpecificCreds->setChecked(true);
    }
}

QString UrlConfigurationDialog::username() const
{
    if (mUi.useDefaultCreds->isChecked()) {
        return mDefaultUsername;
    } else {
        return mUi.username->text();
    }
}

void UrlConfigurationDialog::setDefaultUsername(const QString &userName)
{
    mDefaultUsername = userName;
}

void UrlConfigurationDialog::setUsername(const QString &userName)
{
    mUi.username->setText(userName);
}

QString UrlConfigurationDialog::password() const
{
    if (mUi.useDefaultCreds->isChecked()) {
        return mDefaultPassword;
    } else {
        return mUi.password->password();
    }
}

void UrlConfigurationDialog::setDefaultPassword(const QString &password)
{
    mDefaultPassword = password;
}

void UrlConfigurationDialog::setPassword(const QString &password)
{
    mUi.password->setPassword(password);
}

void UrlConfigurationDialog::onConfigChanged()
{
    initModel();
    mUi.fetchButton->setEnabled(false);
    mOkButton->setEnabled(false);
    checkUserInput();
}

void UrlConfigurationDialog::checkUserInput()
{
    if (!mUi.remoteUrl->text().trimmed().isEmpty() && checkUserAuthInput()) {
        mUi.fetchButton->setEnabled(true);
        if (mModel->rowCount() > 0) {
            mOkButton->setEnabled(true);
        }
    } else {
        mUi.fetchButton->setEnabled(false);
        mOkButton->setEnabled(false);
    }
}

void UrlConfigurationDialog::onFetchButtonClicked()
{
    mUi.discoveredUrls->setEnabled(false);
    initModel();

    if (!remoteUrl().endsWith(QLatin1Char('/'))) {
        setRemoteUrl(remoteUrl() + QLatin1Char('/'));
    }

    if (!remoteUrl().startsWith(QLatin1String("https://")) && !remoteUrl().startsWith(QLatin1String("http://"))) {
        setRemoteUrl(QStringLiteral("https://") + remoteUrl());
    }

    QUrl url(mUi.remoteUrl->text());
    if (mUi.useDefaultCreds->isChecked()) {
        url.setUserName(mDefaultUsername);
        url.setPassword(mDefaultPassword);
    } else {
        url.setUserName(username());
        url.setPassword(password());
    }

    KDAV::DavUrl davUrl(url, protocol());
    auto job = new KDAV::DavCollectionsFetchJob(davUrl);
    connect(job, &KDAV::DavCollectionsFetchJob::result, this, &UrlConfigurationDialog::onCollectionsFetchDone);
    job->start();
}

void UrlConfigurationDialog::onOkButtonClicked()
{
    if (!remoteUrl().endsWith(QLatin1Char('/'))) {
        setRemoteUrl(remoteUrl() + QLatin1Char('/'));
    }
}

void UrlConfigurationDialog::onCollectionsFetchDone(KJob *job)
{
    mUi.discoveredUrls->setEnabled(true);

    if (job->error()) {
        KMessageBox::error(this, job->errorText());
        return;
    }

    auto davJob = qobject_cast<KDAV::DavCollectionsFetchJob *>(job);

    const KDAV::DavCollection::List collections = davJob->collections();

    for (const KDAV::DavCollection &collection : collections) {
        addModelRow(collection.displayName(), collection.url().toDisplayString());
    }

    checkUserInput();
}

void UrlConfigurationDialog::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    // Actually only the display name can be changed, so no stricts checks are required
    const QString newName = topLeft.data().toString();
    const QString url = topLeft.sibling(topLeft.row(), 1).data().toString();

    QUrl fullUrl(url);
    fullUrl.setUserInfo(QString());

    const KDAV::DavUrl davUrl(fullUrl, protocol());
    auto job = new KDAV::DavCollectionModifyJob(davUrl);
    job->setProperty(QStringLiteral("displayname"), newName);
    connect(job, &KDAV::DavCollectionModifyJob::result, this, &UrlConfigurationDialog::onChangeDisplayNameFinished);
    job->start();
    mUi.discoveredUrls->setEnabled(false);
}

void UrlConfigurationDialog::onChangeDisplayNameFinished(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorText());
    }

    onFetchButtonClicked();
}

void UrlConfigurationDialog::initModel()
{
    mModel->clear();
    const QStringList headers = {i18n("Display Name"), i18n("URL")};
    mModel->setHorizontalHeaderLabels(headers);
}

bool UrlConfigurationDialog::checkUserAuthInput()
{
    return mUi.useDefaultCreds->isChecked() || !(mUi.username->text().isEmpty() || mUi.password->password().isEmpty());
}

void UrlConfigurationDialog::addModelRow(const QString &displayName, const QString &url)
{
    QStandardItem *rootItem = mModel->invisibleRootItem();

    QList<QStandardItem *> items;

    auto displayNameStandardItem = new QStandardItem(displayName);
    displayNameStandardItem->setEditable(true);
    items << displayNameStandardItem;

    auto urlStandardItem = new QStandardItem(url);
    urlStandardItem->setEditable(false);
    items << urlStandardItem;

    rootItem->appendRow(items);
}
