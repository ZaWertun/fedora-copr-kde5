/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dummypasswordrequester.h"

#include <QTimer>

#include <QTest>

DummyPasswordRequester::DummyPasswordRequester(QObject *parent)
    : PasswordRequesterInterface(parent)
{
    m_expectedCalls.reserve(10);
    m_results.reserve(10);
    for (int i = 0; i < 10; ++i) {
        m_expectedCalls << StandardRequest;
        m_results << PasswordRetrieved;
    }
}

QString DummyPasswordRequester::password() const
{
    return m_password;
}

void DummyPasswordRequester::setPassword(const QString &password)
{
    m_password = password;
}

void DummyPasswordRequester::setScenario(const QList<RequestType> &expectedCalls, const QList<ResultType> &results)
{
    Q_ASSERT(expectedCalls.size() == results.size());

    m_expectedCalls = expectedCalls;
    m_results = results;
}

void DummyPasswordRequester::setDelays(const QList<int> &delays)
{
    m_delays = delays;
}

void DummyPasswordRequester::requestPassword(RequestType request, const QString & /*serverError*/)
{
    QVERIFY2(!m_expectedCalls.isEmpty(), QStringLiteral("Got unexpected call: %1").arg(request).toUtf8().constData());
    QCOMPARE((int)request, (int)m_expectedCalls.takeFirst());

    int delay = 20;
    if (!m_delays.isEmpty()) {
        delay = m_delays.takeFirst();
    }

    QTimer::singleShot(delay, this, &DummyPasswordRequester::emitResult);
}

void DummyPasswordRequester::emitResult()
{
    ResultType result = m_results.takeFirst();

    if (result == PasswordRetrieved) {
        Q_EMIT done(result, m_password);
    } else {
        Q_EMIT done(result);
    }
}
