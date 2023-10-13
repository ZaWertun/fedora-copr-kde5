/*
    SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapresource.h"

#include "sessionpool.h"
#include "sessionuiproxy.h"
#include "settings.h"
#include "setupserver.h"
#include <config-imap.h>

#ifdef WITH_GMAIL_XOAUTH2
#include "passwordrequester.h"
#else
#include "settingspasswordrequester.h"
#endif

#include <QIcon>

#include <KLocalizedString>
#include <KWindowSystem>

ImapResource::ImapResource(const QString &id)
    : ImapResourceBase(id)
{
#ifdef WITH_GMAIL_XOAUTH2
    m_pool->setPasswordRequester(new PasswordRequester(this, m_pool));
#else
    m_pool->setPasswordRequester(new SettingsPasswordRequester(this, m_pool));
#endif
    m_pool->setSessionUiProxy(SessionUiProxy::Ptr(new SessionUiProxy));
    m_pool->setClientId(clientId());

    settings(); // make sure the D-Bus settings interface is up
}

ImapResource::~ImapResource() = default;

QString ImapResource::defaultName() const
{
    return i18n("IMAP Account");
}

QByteArray ImapResource::clientId() const
{
    return QByteArrayLiteral("Kontact IMAP Resource");
}

QDialog *ImapResource::createConfigureDialog(WId windowId)
{
    auto dlg = new SetupServer(this, windowId);
    dlg->setAttribute(Qt::WA_NativeWindow, true);
    KWindowSystem::setMainWindow(dlg->windowHandle(), windowId);
    dlg->setWindowTitle(i18nc("@title:window", "IMAP Account Settings"));
    dlg->setWindowIcon(QIcon::fromTheme(QStringLiteral("network-server")));
    connect(dlg, &SetupServer::finished, this, &ImapResource::onConfigurationDone);
    return dlg;
}

void ImapResource::onConfigurationDone(int result)
{
    auto dlg = qobject_cast<SetupServer *>(sender());
    if (result) {
        if (dlg->shouldClearCache()) {
            clearCache();
        }
        settings()->save();
    }
    dlg->deleteLater();
}
