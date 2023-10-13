/*
    SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newmailnotificationhistorymanager.h"
#include <QDateTime>

NewMailNotificationHistoryManager::NewMailNotificationHistoryManager(QObject *parent)
    : QObject{parent}
{
}

NewMailNotificationHistoryManager::~NewMailNotificationHistoryManager() = default;

NewMailNotificationHistoryManager *NewMailNotificationHistoryManager::self()
{
    static NewMailNotificationHistoryManager s_self;
    return &s_self;
}

QStringList NewMailNotificationHistoryManager::history() const
{
    return mHistory;
}

void NewMailNotificationHistoryManager::addHistory(QString str)
{
    if (!mHistory.isEmpty()) {
        mHistory += QStringLiteral("\n");
    }
    QString newStr = QStringLiteral("============ %1 ============").arg(QDateTime::currentDateTime().toString());
    if (!str.startsWith(QLatin1Char('\n')) && !str.startsWith(QStringLiteral("<br>"))) {
        newStr += QLatin1Char('\n');
    }
    str.replace(QStringLiteral("<br>"), QStringLiteral("\n"));
    str.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    str.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    newStr += str;
    mHistory += newStr;
    Q_EMIT historyAdded(newStr);
}

void NewMailNotificationHistoryManager::setHistory(const QStringList &newHistory)
{
    mHistory = newHistory;
}

void NewMailNotificationHistoryManager::clear()
{
    mHistory.clear();
}
