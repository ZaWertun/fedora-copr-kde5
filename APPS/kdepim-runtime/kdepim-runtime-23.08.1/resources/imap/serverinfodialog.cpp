/*
  SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "serverinfodialog.h"
#include "imapresource.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

namespace
{
static const char myServerInfoDialogConfigGroupName[] = "ServerInfoDialog";
}
ServerInfoDialog::ServerInfoDialog(ImapResourceBase *parentResource, QWidget *parent)
    : QDialog(parent)
    , mTextBrowser(new ServerInfoTextBrowser(this))
{
    setWindowTitle(i18nc("@title:window Dialog title for dialog showing information about a server", "Server Info"));
    auto mainLayout = new QVBoxLayout(this);
    setAttribute(Qt::WA_DeleteOnClose);

    mTextBrowser->setPlainText(parentResource->serverCapabilities().join(QLatin1Char('\n')));
    mainLayout->addWidget(mTextBrowser);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ServerInfoDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ServerInfoDialog::reject);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

ServerInfoDialog::~ServerInfoDialog()
{
    writeConfig();
}

void ServerInfoDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), myServerInfoDialogConfigGroupName);
    KWindowConfig::saveWindowSize(windowHandle(), group);
}

void ServerInfoDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(500, 300));
    KConfigGroup group(KSharedConfig::openStateConfig(), myServerInfoDialogConfigGroupName);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

ServerInfoTextBrowser::ServerInfoTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
}

ServerInfoTextBrowser::~ServerInfoTextBrowser() = default;

void ServerInfoTextBrowser::paintEvent(QPaintEvent *event)
{
    if (document()->isEmpty()) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        const QPalette palette = viewport()->palette();
        QColor color = palette.text().color();
        color.setAlpha(128);
        p.setPen(color);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("Resource not synchronized yet."));
    } else {
        QTextBrowser::paintEvent(event);
    }
}
