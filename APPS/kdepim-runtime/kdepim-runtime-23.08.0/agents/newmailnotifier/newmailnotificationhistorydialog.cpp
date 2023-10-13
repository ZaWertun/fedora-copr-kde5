/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotificationhistorydialog.h"
#include "newmailnotificationhistorywidget.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWindow>

namespace
{
static const char myConfigNewMailHistoryNotificationDialogGroupName[] = "NewMailHistoryNotificationDialog";
}
NewMailNotificationHistoryDialog::NewMailNotificationHistoryDialog(QWidget *parent)
    : QDialog(parent)
    , mNewHistoryNotificationWidget(new NewMailNotificationHistoryWidget(this))
{
    setWindowTitle(i18nc("@title:window", "Notification History"));
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName(QStringLiteral("mainLayout"));

    mNewHistoryNotificationWidget->setObjectName(QStringLiteral("mNewHistoryNotificationWidget"));
    mainLayout->addWidget(mNewHistoryNotificationWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewMailNotificationHistoryDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewMailNotificationHistoryDialog::reject);
    readConfig();
}

NewMailNotificationHistoryDialog::~NewMailNotificationHistoryDialog()
{
    writeConfig();
}

void NewMailNotificationHistoryDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(800, 300));
    KConfigGroup group(KSharedConfig::openStateConfig(), myConfigNewMailHistoryNotificationDialogGroupName);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void NewMailNotificationHistoryDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), myConfigNewMailHistoryNotificationDialogGroupName);
    KWindowConfig::saveWindowSize(windowHandle(), group);
}
