/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class PasswordRequesterInterface : public QObject
{
    Q_OBJECT
    Q_ENUMS(ResultType RequestType)

public:
    enum ResultType {
        PasswordRetrieved,
        ReconnectNeeded,
        UserRejected,
        EmptyPasswordEntered,
    };

    enum RequestType {
        StandardRequest,
        WrongPasswordRequest,
    };

protected:
    explicit PasswordRequesterInterface(QObject *parent = nullptr);

public:
    virtual void requestPassword(RequestType request = StandardRequest, const QString &serverError = QString()) = 0;
    virtual void cancelPasswordRequests();

Q_SIGNALS:
    void done(int resultType, const QString &password = QString());
};
