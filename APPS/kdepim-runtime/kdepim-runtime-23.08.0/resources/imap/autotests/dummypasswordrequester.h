/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "passwordrequesterinterface.h"

class DummyPasswordRequester : public PasswordRequesterInterface
{
    Q_OBJECT
public:
    DummyPasswordRequester(QObject *parent = nullptr);

    QString password() const;
    void setPassword(const QString &password);

    void setScenario(const QList<RequestType> &expectedCalls, const QList<ResultType> &results);
    void setDelays(const QList<int> &delays);

public:
    void requestPassword(RequestType request = StandardRequest, const QString &serverError = QString()) override;

private Q_SLOTS:
    void emitResult();

private:
    QString m_password;
    QList<RequestType> m_expectedCalls;
    QList<ResultType> m_results;
    QList<int> m_delays;
};
