/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"
#include "resourcetask.h"
#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(ResourceTask::ActionIfNoSession)

class DummyResourceTask : public ResourceTask
{
public:
    explicit DummyResourceTask(ActionIfNoSession action, ResourceStateInterface::Ptr resource, QObject *parent = nullptr)
        : ResourceTask(action, resource, parent)
    {
    }

    void doStart(KIMAP::Session * /*session*/) override
    {
        cancelTask(QStringLiteral("Dummy task"));
    }
};

class TestResourceTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldRequestSession_data()
    {
        QTest::addColumn<DummyResourceState::Ptr>("state");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<bool>("shouldConnect");
        QTest::addColumn<bool>("shouldRequestSession");
        QTest::addColumn<ResourceTask::ActionIfNoSession>("actionIfNoSession");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<QVariant>("firstCallParameter");

        DummyResourceState::Ptr state;
        QList<QByteArray> scenario;
        QStringList callNames;

        state = DummyResourceState::Ptr(new DummyResourceState);
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                 << "S: A000002 OK Completed";
        callNames.clear();
        callNames << QStringLiteral("cancelTask");
        QTest::newRow("normal case") << state << scenario << true << false << ResourceTask::DeferIfNoSession << callNames
                                     << QVariant(QLatin1String("Dummy task"));

        state = DummyResourceState::Ptr(new DummyResourceState);
        callNames.clear();
        callNames << QStringLiteral("deferTask");
        QTest::newRow("all sessions allocated (defer)") << state << scenario << true << true << ResourceTask::DeferIfNoSession << callNames << QVariant();

        state = DummyResourceState::Ptr(new DummyResourceState);
        callNames.clear();
        callNames << QStringLiteral("cancelTask");
        QTest::newRow("all sessions allocated (cancel)") << state << scenario << true << true << ResourceTask::CancelIfNoSession << callNames << QVariant();

        state = DummyResourceState::Ptr(new DummyResourceState);
        scenario.clear();
        callNames.clear();
        callNames << QStringLiteral("deferTask") << QStringLiteral("scheduleConnectionAttempt");
        QTest::newRow("disconnected pool (defer)") << state << scenario << false << false << ResourceTask::DeferIfNoSession << callNames << QVariant();

        state = DummyResourceState::Ptr(new DummyResourceState);
        scenario.clear();
        callNames.clear();
        callNames << QStringLiteral("cancelTask") << QStringLiteral("scheduleConnectionAttempt");
        QTest::newRow("disconnected pool (cancel)") << state << scenario << false << false << ResourceTask::CancelIfNoSession << callNames << QVariant();
    }

    void shouldRequestSession()
    {
        QFETCH(DummyResourceState::Ptr, state);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(bool, shouldConnect);
        QFETCH(bool, shouldRequestSession);
        QFETCH(ResourceTask::ActionIfNoSession, actionIfNoSession);
        QFETCH(QStringList, callNames);
        QFETCH(QVariant, firstCallParameter);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        if (shouldConnect) {
            QSignalSpy poolSpy(&pool, &SessionPool::connectDone);

            pool.setPasswordRequester(createDefaultRequester());
            QVERIFY(pool.connect(createDefaultAccount()));

            QTRY_COMPARE(poolSpy.count(), 1);
            QCOMPARE(poolSpy.at(0).at(0).toInt(), (int)SessionPool::NoError);
        }

        if (shouldRequestSession) {
            QSignalSpy requestSpy(&pool, &SessionPool::sessionRequestDone);
            pool.requestSession();
            QTRY_COMPARE(requestSpy.count(), 1);
        }

        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        auto task = new DummyResourceTask(actionIfNoSession, state);
        task->start(&pool);

        if (shouldConnect) {
            QTRY_COMPARE(sessionSpy.count(), 1);
        } else {
            // We want to ensure the signal isn't emitted, so we have to wait
            QTest::qWait(500);
            QCOMPARE(sessionSpy.count(), 0);
        }

        QCOMPARE(state->calls().count(), callNames.size());
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(state->calls().at(i).first);
            QCOMPARE(command, callNames[i]);
        }

        if (firstCallParameter.toString() == QLatin1String("Dummy task")) {
            QCOMPARE(state->calls().first().second, firstCallParameter);
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_GUILESS_MAIN(TestResourceTask)

#include "testresourcetask.moc"
