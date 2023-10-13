/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "passwordrequester.h"
#include "imapresourcebase.h"
#include "settings.h"
#include <config-imap.h>

#include "gmailpasswordrequester.h"
#include "settingspasswordrequester.h"

PasswordRequester::PasswordRequester(ImapResourceBase *resource, QObject *parent)
    : PasswordRequesterInterface(parent)
    , mResource(resource)
{
}

PasswordRequester::~PasswordRequester() = default;

PasswordRequesterInterface *PasswordRequester::requesterImpl()
{
    const bool isXOAuth = mResource->settings()->authentication() == MailTransport::Transport::EnumAuthenticationType::XOAUTH2;
    if (!mImpl || (isXOAuth != !!qobject_cast<GmailPasswordRequester *>(mImpl))) {
        if (mImpl) {
            mImpl->disconnect(this);
            mImpl->deleteLater();
        }
        if (isXOAuth) {
            mImpl = new GmailPasswordRequester(mResource, this);
        } else {
            mImpl = new SettingsPasswordRequester(mResource, this);
        }
        connect(mImpl, &PasswordRequesterInterface::done, this, &PasswordRequesterInterface::done);
    }

    return mImpl;
}

void PasswordRequester::cancelPasswordRequests()
{
    requesterImpl()->cancelPasswordRequests();
}

void PasswordRequester::requestPassword(RequestType request, const QString &serverError)
{
    requesterImpl()->requestPassword(request, serverError);
}
