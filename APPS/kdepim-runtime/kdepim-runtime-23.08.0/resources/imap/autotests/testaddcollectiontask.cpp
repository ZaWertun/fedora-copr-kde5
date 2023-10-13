/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "addcollectiontask.h"
#include <Akonadi/CollectionAnnotationsAttribute>
#include <QDebug>
#include <QTest>
class TestAddCollectionTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldCreateAndSubscribe_data()
    {
        QTest::addColumn<Akonadi::Collection>("parentCollection");
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<QString>("collectionName");
        QTest::addColumn<QString>("remoteId");

        Akonadi::Collection parentCollection;
        Akonadi::Collection collection;
        QList<QByteArray> scenario;
        QStringList callNames;

        parentCollection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection = Akonadi::Collection(4);
        collection.setName(QStringLiteral("Bar"));
        collection.setParentCollection(parentCollection);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 CREATE \"INBOX/Foo/Bar\""
                 << "S: A000003 OK create done"
                 << "C: A000004 SUBSCRIBE \"INBOX/Foo/Bar\""
                 << "S: A000004 OK subscribe done";

        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted") << QStringLiteral("synchronizeCollectionTree");

        QTest::newRow("trivial case") << parentCollection << collection << scenario << callNames << collection.name() << "/Bar";

        parentCollection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection = Akonadi::Collection(4);
        collection.setName(QStringLiteral("Bar/Baz"));
        collection.setParentCollection(parentCollection);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 CREATE \"INBOX/Foo/BarBaz\""
                 << "S: A000003 OK create done"
                 << "C: A000004 SUBSCRIBE \"INBOX/Foo/BarBaz\""
                 << "S: A000004 OK subscribe done";

        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted") << QStringLiteral("synchronizeCollectionTree");

        QTest::newRow("folder with invalid separator") << parentCollection << collection << scenario << callNames << "BarBaz"
                                                       << "/BarBaz";

        parentCollection = createCollectionChain(QStringLiteral(".INBOX"));
        collection = Akonadi::Collection(3);
        collection.setName(QStringLiteral("Foo"));
        collection.setParentCollection(parentCollection);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 CREATE \"INBOX.Foo\""
                 << "S: A000003 OK create done"
                 << "C: A000004 SUBSCRIBE \"INBOX.Foo\""
                 << "S: A000004 OK subscribe done";
        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted") << QStringLiteral("synchronizeCollectionTree");

        QTest::newRow("folder with non-standard separator") << parentCollection << collection << scenario << callNames << "Foo"
                                                            << ".Foo";

        parentCollection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection = Akonadi::Collection(4);
        collection.setName(QStringLiteral("Bar"));
        collection.setParentCollection(parentCollection);
        auto attr = collection.attribute<Akonadi::CollectionAnnotationsAttribute>(Akonadi::Collection::AddIfMissing);
        QMap<QByteArray, QByteArray> annotations;
        annotations.insert("/shared/vendor/foobar/foo", "value");
        attr->setAnnotations(annotations);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 CREATE \"INBOX/Foo/Bar\""
                 << "S: A000003 OK create done"
                 << "C: A000004 SUBSCRIBE \"INBOX/Foo/Bar\""
                 << "S: A000004 OK subscribe done"
                 << R"(C: A000005 SETMETADATA "INBOX/Foo/Bar" ("/shared/vendor/foobar/foo" "value"))"
                 << "S: A000005 OK SETMETADATA complete";

        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted");

        QTest::newRow("folder with annotations") << parentCollection << collection << scenario << callNames << collection.name() << "/Bar";
    }

    void shouldCreateAndSubscribe()
    {
        QFETCH(Akonadi::Collection, parentCollection);
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(QString, collectionName);
        QFETCH(QString, remoteId);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setParentCollection(parentCollection);
        state->setCollection(collection);
        if (collection.hasAttribute<Akonadi::CollectionAnnotationsAttribute>()) {
            state->setServerCapabilities(QStringList() << QStringLiteral("METADATA"));
        }
        auto task = new AddCollectionTask(state);
        task->start(&pool);

        QTRY_COMPARE(state->calls().count(), callNames.size());
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(state->calls().at(i).first);
            QVariant parameter = state->calls().at(i).second;

            if (command == QLatin1String("cancelTask") && callNames[i] != QLatin1String("cancelTask")) {
                qDebug() << "Got a cancel:" << parameter.toString();
            }

            if (command == QLatin1String("collectionChangeCommitted")) {
                QCOMPARE(parameter.value<Akonadi::Collection>().name(), collectionName);
                QCOMPARE(parameter.value<Akonadi::Collection>().remoteId().right(collectionName.length()), collectionName);
                QCOMPARE(parameter.value<Akonadi::Collection>().remoteId(), remoteId);
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

QTEST_GUILESS_MAIN(TestAddCollectionTask)

#include "testaddcollectiontask.moc"
