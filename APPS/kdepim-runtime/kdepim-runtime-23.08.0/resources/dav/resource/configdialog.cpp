/*
    SPDX-FileCopyrightText: 2009 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"
#include "searchdialog.h"
#include "settings.h"
#include "urlconfigurationdialog.h"
#include "utils.h"

#include <KDAV/ProtocolInfo>

#include <KConfigDialogManager>
#include <KLocalizedString>
#include <KMessageBox>

#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QPointer>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QVBoxLayout>
#include <QWindow>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , mModel(new QStandardItemModel(this))
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("folder-remote")));
    setWindowTitle(i18nc("@title:window", "DAV Resource Configuration"));
    auto mainLayout = new QVBoxLayout(this);
    auto mainWidget = new QWidget(this);
    mainLayout->addWidget(mainWidget);
    mUi.setupUi(mainWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigDialog::onCancelClicked);
    mainLayout->addWidget(buttonBox);

    const QStringList headers = {i18n("Protocol"), i18n("URL")};
    mModel->setHorizontalHeaderLabels(headers);

    mUi.configuredUrls->setModel(mModel);
    mUi.configuredUrls->setRootIsDecorated(false);

    const KDAV::DavUrl::List lstUrls = Settings::self()->configuredDavUrls();
    for (const KDAV::DavUrl &url : lstUrls) {
        QUrl displayUrl = url.url();
        displayUrl.setUserInfo(QString());
        addModelRow(Utils::translatedProtocolName(url.protocol()), displayUrl.toDisplayString());
    }

    mUi.syncRangeStartType->addItem(i18n("Days"), QVariant(QLatin1String("D")));
    mUi.syncRangeStartType->addItem(i18n("Months"), QVariant(QLatin1String("M")));
    mUi.syncRangeStartType->addItem(i18n("Years"), QVariant(QLatin1String("Y")));

    mManager = new KConfigDialogManager(this, Settings::self());
    mManager->updateWidgets();

    const int typeIndex = mUi.syncRangeStartType->findData(QVariant(Settings::self()->syncRangeStartType()));
    mUi.syncRangeStartType->setCurrentIndex(typeIndex);

    connect(mUi.kcfg_displayName, &QLineEdit::textChanged, this, &ConfigDialog::checkUserInput);
    connect(mUi.configuredUrls->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ConfigDialog::checkConfiguredUrlsButtonsState);
    connect(mUi.configuredUrls, &QAbstractItemView::doubleClicked, this, &ConfigDialog::onEditButtonClicked);

    connect(mUi.syncRangeStartType, &QComboBox::currentIndexChanged, this, &ConfigDialog::onSyncRangeStartTypeChanged);
    connect(mUi.addButton, &QPushButton::clicked, this, &ConfigDialog::onAddButtonClicked);
    connect(mUi.searchButton, &QPushButton::clicked, this, &ConfigDialog::onSearchButtonClicked);
    connect(mUi.removeButton, &QPushButton::clicked, this, &ConfigDialog::onRemoveButtonClicked);
    connect(mUi.editButton, &QPushButton::clicked, this, &ConfigDialog::onEditButtonClicked);

    connect(mOkButton, &QPushButton::clicked, this, &ConfigDialog::onOkClicked);

    checkUserInput();
    readConfig();
}

ConfigDialog::~ConfigDialog()
{
    writeConfig();
}

void ConfigDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(300, 200));
    KConfigGroup group(KSharedConfig::openStateConfig(), "ConfigDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void ConfigDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openStateConfig(), "ConfigDialog");
    KWindowConfig::saveWindowSize(windowHandle(), grp);
    grp.sync();
}

void ConfigDialog::setPassword(const QString &password)
{
    mUi.password->setPassword(password);
}

void ConfigDialog::onSyncRangeStartTypeChanged()
{
    Settings::self()->setSyncRangeStartType(mUi.syncRangeStartType->currentData().toString());
}

void ConfigDialog::checkUserInput()
{
    checkConfiguredUrlsButtonsState();

    if (!mUi.kcfg_displayName->text().trimmed().isEmpty() && !(mModel->invisibleRootItem()->rowCount() == 0)) {
        mOkButton->setEnabled(true);
    } else {
        mOkButton->setEnabled(false);
    }
}

void ConfigDialog::onAddButtonClicked()
{
    QPointer<UrlConfigurationDialog> dlg = new UrlConfigurationDialog(this);
    dlg->setDefaultUsername(mUi.kcfg_defaultUsername->text());
    dlg->setDefaultPassword(mUi.password->password());
    const int result = dlg->exec();

    if (result == QDialog::Accepted && !dlg.isNull()) {
        if (Settings::self()->urlConfiguration(KDAV::Protocol(dlg->protocol()), dlg->remoteUrl())) {
            KMessageBox::error(this,
                               i18n("Another configuration entry already uses the same URL/protocol couple.\n"
                                    "Please use a different URL"));
        } else {
            auto urlConfig = new Settings::UrlConfiguration();

            urlConfig->mUrl = dlg->remoteUrl();
            if (dlg->useDefaultCredentials()) {
                urlConfig->mUser = QStringLiteral("$default$");
            } else {
                urlConfig->mUser = dlg->username();
                urlConfig->mPassword = dlg->password();
            }
            urlConfig->mProtocol = dlg->protocol();

            Settings::self()->newUrlConfiguration(urlConfig);

            const QString protocolName = Utils::translatedProtocolName(dlg->protocol());

            addModelRow(protocolName, dlg->remoteUrl());
            mAddedUrls << QPair<QString, KDAV::Protocol>(dlg->remoteUrl(), KDAV::Protocol(dlg->protocol()));
            checkUserInput();
        }
    }

    delete dlg;
}

void ConfigDialog::onSearchButtonClicked()
{
    QPointer<SearchDialog> dlg = new SearchDialog(this);
    dlg->setUsername(mUi.kcfg_defaultUsername->text());
    dlg->setPassword(mUi.password->password());
    const int result = dlg->exec();

    if (result == QDialog::Accepted && !dlg.isNull()) {
        const QStringList results = dlg->selection();
        for (const QString &resultStr : results) {
            const QStringList split = resultStr.split(QLatin1Char('|'));
            KDAV::Protocol protocol = KDAV::ProtocolInfo::protocolByName(split.at(0));
            if (!Settings::self()->urlConfiguration(protocol, split.at(1))) {
                auto urlConfig = new Settings::UrlConfiguration();

                urlConfig->mUrl = split.at(1);
                if (dlg->useDefaultCredentials()) {
                    urlConfig->mUser = QStringLiteral("$default$");
                } else {
                    urlConfig->mUser = dlg->username();
                    urlConfig->mPassword = dlg->password();
                }
                urlConfig->mProtocol = protocol;

                Settings::self()->newUrlConfiguration(urlConfig);

                addModelRow(Utils::translatedProtocolName(protocol), split.at(1));
                mAddedUrls << QPair<QString, KDAV::Protocol>(split.at(1), protocol);
                checkUserInput();
            }
        }
    }

    delete dlg;
}

void ConfigDialog::onRemoveButtonClicked()
{
    const QModelIndexList indexes = mUi.configuredUrls->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    const QString proto = mModel->index(indexes.at(0).row(), 0).data().toString();
    const QString url = mModel->index(indexes.at(0).row(), 1).data().toString();

    mRemovedUrls << QPair<QString, KDAV::Protocol>(url, Utils::protocolByTranslatedName(proto));
    mModel->removeRow(indexes.at(0).row());

    checkUserInput();
}

void ConfigDialog::onEditButtonClicked()
{
    const QModelIndexList indexes = mUi.configuredUrls->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    const int row = indexes.at(0).row();
    const QString proto = mModel->index(row, 0).data().toString();
    const QString url = mModel->index(row, 1).data().toString();

    Settings::UrlConfiguration *urlConfig = Settings::self()->urlConfiguration(Utils::protocolByTranslatedName(proto), url);
    if (!urlConfig) {
        return;
    }

    QPointer<UrlConfigurationDialog> dlg = new UrlConfigurationDialog(this);
    dlg->setRemoteUrl(urlConfig->mUrl);
    dlg->setProtocol(KDAV::Protocol(urlConfig->mProtocol));

    if (urlConfig->mUser == QLatin1String("$default$")) {
        dlg->setUseDefaultCredentials(true);
    } else {
        dlg->setUseDefaultCredentials(false);
        dlg->setUsername(urlConfig->mUser);
        dlg->setPassword(urlConfig->mPassword);
    }
    dlg->setDefaultUsername(mUi.kcfg_defaultUsername->text());
    dlg->setDefaultPassword(mUi.password->password());

    const int result = dlg->exec();

    if (result == QDialog::Accepted && !dlg.isNull()) {
        Settings::self()->removeUrlConfiguration(Utils::protocolByTranslatedName(proto), url);
        auto urlConfigAccepted = new Settings::UrlConfiguration();
        urlConfigAccepted->mUrl = dlg->remoteUrl();
        if (dlg->useDefaultCredentials()) {
            urlConfigAccepted->mUser = QStringLiteral("$default$");
        } else {
            urlConfigAccepted->mUser = dlg->username();
            urlConfigAccepted->mPassword = dlg->password();
        }
        urlConfigAccepted->mProtocol = dlg->protocol();
        Settings::self()->newUrlConfiguration(urlConfigAccepted);

        mModel->removeRow(row);
        insertModelRow(row, Utils::translatedProtocolName(dlg->protocol()), dlg->remoteUrl());
    }
    delete dlg;
}

void ConfigDialog::onOkClicked()
{
    using UrlPair = QPair<QString, KDAV::Protocol>;
    for (const UrlPair &url : std::as_const(mRemovedUrls)) {
        Settings::self()->removeUrlConfiguration(url.second, url.first);
    }

    mManager->updateSettings();
    Settings::self()->setDefaultPassword(mUi.password->password());
    accept();
}

void ConfigDialog::onCancelClicked()
{
    mRemovedUrls.clear();

    using UrlPair = QPair<QString, KDAV::Protocol>;
    for (const UrlPair &url : std::as_const(mAddedUrls)) {
        Settings::self()->removeUrlConfiguration(url.second, url.first);
    }
    reject();
}

void ConfigDialog::checkConfiguredUrlsButtonsState()
{
    const bool enabled = mUi.configuredUrls->selectionModel()->hasSelection();

    mUi.removeButton->setEnabled(enabled);
    mUi.editButton->setEnabled(enabled);
}

void ConfigDialog::addModelRow(const QString &protocol, const QString &url)
{
    insertModelRow(-1, protocol, url);
}

void ConfigDialog::insertModelRow(int index, const QString &protocol, const QString &url)
{
    QStandardItem *rootItem = mModel->invisibleRootItem();
    QList<QStandardItem *> items;

    auto protocolStandardItem = new QStandardItem(protocol);
    protocolStandardItem->setEditable(false);
    items << protocolStandardItem;

    auto urlStandardItem = new QStandardItem(url);
    urlStandardItem->setEditable(false);
    items << urlStandardItem;

    if (index == -1) {
        rootItem->appendRow(items);
    } else {
        rootItem->insertRow(index, items);
    }
}
