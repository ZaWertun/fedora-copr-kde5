/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QDebug>

#include "imapaccount.h"
#include "subscriptiondialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (app.arguments().size() < 5) {
        qWarning("Not enough parameters, expecting: <server> <port> <user> <password>");
        return 1;
    }

    const QString server = app.arguments().at(1);
    const int port = app.arguments().at(2).toInt();
    const QString user = app.arguments().at(3);
    const QString password = app.arguments().at(4);

    qDebug() << "Querying:" << server << port << user << password;
    qDebug();

    ImapAccount account;
    account.setServer(server);
    account.setPort(port);
    account.setUserName(user);

    auto dialog = new SubscriptionDialog();
    dialog->connectAccount(account, password);

    dialog->show();

    int retcode = app.exec();

    qDebug() << "Subscription changed?" << dialog->isSubscriptionChanged();

    return retcode;
}
