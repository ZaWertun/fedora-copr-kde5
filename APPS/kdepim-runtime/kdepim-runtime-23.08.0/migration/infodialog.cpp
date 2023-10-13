/*
    SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "infodialog.h"

#include "migration_debug.h"
#include <QIcon>

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

enum {
    // The max value of the scrollbar. Don't change this without making the kmail
    // migrator use this. It still uses hardcoded "100".
    MAX_PROGRESS = 100
};

bool InfoDialog::mError = false;

InfoDialog::InfoDialog(bool closeWhenDone)
    : mButtonBox(new QDialogButtonBox(QDialogButtonBox::Close, this))
    , mList(new QListWidget(this))
    , mStatusLabel(new QLabel(this))
    , mProgressBar(new QProgressBar(this))
    , mCloseWhenDone(closeWhenDone)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto mainLayout = new QVBoxLayout(this);

    mList->setMinimumWidth(640);
    mainLayout->addWidget(mList);

    auto statusLayout = new QHBoxLayout;
    mainLayout->addLayout(statusLayout);

    mStatusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    statusLayout->addWidget(mStatusLabel);

    mProgressBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mProgressBar->setMinimumWidth(200);
    statusLayout->addWidget(mProgressBar);

    connect(mButtonBox, &QDialogButtonBox::accepted, this, &InfoDialog::accept);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &InfoDialog::reject);
    mButtonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    mainLayout->addWidget(mButtonBox);
}

InfoDialog::~InfoDialog() = default;

static KMigratorBase::MessageType convertType(MigratorBase::MessageType type)
{
    switch (type) {
    case MigratorBase::Success:
        return KMigratorBase::Success;
    case MigratorBase::Error:
        return KMigratorBase::Error;
    case MigratorBase::Skip:
        return KMigratorBase::Skip;
    case MigratorBase::Warning:
        return KMigratorBase::Warning;
    case MigratorBase::Info:
        return KMigratorBase::Info;
    }
    return KMigratorBase::Info;
}

void InfoDialog::message(MigratorBase::MessageType type, const QString &msg)
{
    message(convertType(type), msg);
}

void InfoDialog::message(KMigratorBase::MessageType type, const QString &msg)
{
    bool autoScroll = mAutoScrollList;

    auto item = new QListWidgetItem(msg, mList);
    switch (type) {
    case KMigratorBase::Success:
        item->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
        mChange = true;
        qCDebug(MIGRATION_LOG) << msg;
        break;
    case KMigratorBase::Skip:
        item->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
        qCDebug(MIGRATION_LOG) << msg;
        break;
    case KMigratorBase::Info:
        item->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
        qCDebug(MIGRATION_LOG) << msg;
        break;
    case KMigratorBase::Warning:
        item->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));
        qCDebug(MIGRATION_LOG) << msg;
        break;
    case KMigratorBase::Error: {
        item->setIcon(QIcon::fromTheme(QStringLiteral("dialog-error")));
        QFont currentFont = font();
        currentFont.setBold(true);
        item->setFont(currentFont);
        mError = true;
        qCCritical(MIGRATION_LOG) << msg;
        break;
    }
    default:
        qCCritical(MIGRATION_LOG) << "WTF?";
    }

    mAutoScrollList = autoScroll;

    if (autoScroll) {
        mList->scrollToItem(item);
    }
}

void InfoDialog::migratorAdded()
{
    ++mMigratorCount;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void InfoDialog::migratorDone()
{
    QApplication::restoreOverrideCursor();

    --mMigratorCount;
    if (mMigratorCount == 0) {
        mButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
        status(QString());
        if (mCloseWhenDone && !hasError() && !hasChange()) {
            accept();
        }
    }
}

void InfoDialog::status(const QString &msg)
{
    mStatusLabel->setText(msg);
    if (msg.isEmpty()) {
        progress(0, MAX_PROGRESS, MAX_PROGRESS);
        mProgressBar->setFormat(QString());
    }
}

void InfoDialog::progress(int value)
{
    mProgressBar->setFormat(QStringLiteral("%p%"));
    mProgressBar->setValue(value);
}

void InfoDialog::progress(int min, int max, int value)
{
    mProgressBar->setFormat(QStringLiteral("%p%"));
    mProgressBar->setRange(min, max);
    mProgressBar->setValue(value);
}

void InfoDialog::scrollBarMoved(int value)
{
    mAutoScrollList = (value == mList->verticalScrollBar()->maximum());
}
