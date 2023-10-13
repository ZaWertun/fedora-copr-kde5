/*
   SPDX-FileCopyrightText: 2021-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newmailnotifierreplymessagejob.h"
#include "newmailnotifier_debug.h"
#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>

NewMailNotifierReplyMessageJob::NewMailNotifierReplyMessageJob(Akonadi::Item::Id id, QObject *parent)
    : KJob(parent)
    , mId(id)
{
}

NewMailNotifierReplyMessageJob::~NewMailNotifierReplyMessageJob() = default;

void NewMailNotifierReplyMessageJob::start()
{
    if (mId < 0) {
        emitResult();
        return;
    }
    const QString kmailInterface = QStringLiteral("org.kde.kmail");
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(kmailInterface);
    if (!reply.isValid() || !reply.value()) {
        // Program is not already running, so start it
        QString errmsg;
        if (!QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.kmail2")).isValid()) {
            qCDebug(NEWMAILNOTIFIER_LOG) << " Can not start kmail";
            setError(UserDefinedError);
            setErrorText(i18n("Unable to start KMail application."));
            emitResult();
            return;
        }
    }
    QDBusInterface kmail(kmailInterface, QStringLiteral("/KMail"), QStringLiteral("org.kde.kmail.kmail"));
    if (kmail.isValid()) {
        kmail.call(QStringLiteral("replyMail"), mId, mReplyToAll);
    } else {
        qCWarning(NEWMAILNOTIFIER_LOG) << "Impossible to access to DBus interface";
    }
    emitResult();
}

bool NewMailNotifierReplyMessageJob::replyToAll() const
{
    return mReplyToAll;
}

void NewMailNotifierReplyMessageJob::setReplyToAll(bool newReplyToAll)
{
    mReplyToAll = newReplyToAll;
}
