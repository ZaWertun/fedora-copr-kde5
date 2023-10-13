/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "expungecollectiontask.h"
#include <QTest>
class TestExpungeCollectionTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldDeleteMailBox_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");

        Akonadi::Collection collection;
        QList<QByteArray> scenario;
        QStringList callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done";

        callNames.clear();
        callNames << QStringLiteral("taskDone");

        QTest::newRow("normal case") << collection << scenario << callNames;

        // We keep the same collection

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 NO select failed";

        callNames.clear();
        callNames << QStringLiteral("cancelTask");

        QTest::newRow("select failed") << collection << scenario << callNames;

        // We keep the same collection

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 NO expunge failed";

        callNames.clear();
        callNames << QStringLiteral("cancelTask");

        QTest::newRow("expunge failed") << collection << scenario << callNames;

        // We keep the same collection

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK [READ-ONLY] select done";

        callNames.clear();
        callNames << QStringLiteral("taskDone");

        QTest::newRow("read-only mailbox") << collection << scenario << callNames;
    }

    void shouldDeleteMailBox()
    {
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setCollection(collection);
        auto task = new ExpungeCollectionTask(state);
        task->start(&pool);

        QTRY_COMPARE(state->calls().count(), callNames.size());
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(state->calls().at(i).first);
            QVariant parameter = state->calls().at(i).second;

            if (command == QLatin1String("cancelTask") && callNames[i] != QLatin1String("cancelTask")) {
                qDebug() << "Got a cancel:" << parameter.toString();
            }

            QCOMPARE(command, callNames[i]);

            if (command == QLatin1String("cancelTask")) {
                QVERIFY(!parameter.toString().isEmpty());
            }
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_GUILESS_MAIN(TestExpungeCollectionTask)

#include "testexpungecollectiontask.moc"
