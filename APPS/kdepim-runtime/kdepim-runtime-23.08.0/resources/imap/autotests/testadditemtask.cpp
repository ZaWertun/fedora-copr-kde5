/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "additemtask.h"

#include "uidnextattribute.h"

#include <KMime/Message>

#include <QTest>

class TestAddItemTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldAppendMessage_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");

        Akonadi::Collection collection;
        Akonadi::Item item;
        QString messageContent;
        QList<QByteArray> scenario;
        QStringList callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        auto uidNext = new UidNextAttribute;
        uidNext->setUidNext(63);
        collection.addAttribute(uidNext);

        item = Akonadi::Item(2);
        item.setParentCollection(collection);

        KMime::Message::Ptr message(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 APPEND \"INBOX/Foo\" {55}\r\n" + message->encodedContent(true)
                 << "S: A000003 OK append done [ APPENDUID 1239890035 66 ]";

        callNames.clear();
        callNames << QStringLiteral("itemChangeCommitted");

        QTest::newRow("trivial case") << item << collection << scenario << callNames;

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\nMessage-ID: <42.4242.foo@bar.org>\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 APPEND \"INBOX/Foo\" {90}\r\n" + message->encodedContent(true) << "S: A000003 OK append done"
                 << "C: A000004 SELECT \"INBOX/Foo\""
                 << "S: A000004 OK select done"
                 << "C: A000005 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 66"
                 << "S: A000005 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemChangeCommitted");

        QTest::newRow("no APPENDUID, message contained Message-ID") << item << collection << scenario << callNames;

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 APPEND \"INBOX/Foo\" {90}\r\n" + message->encodedContent(true) << "S: A000003 OK append done"
                 << "C: A000004 SELECT \"INBOX/Foo\""
                 << "S: A000004 OK select done"
                 << "C: A000005 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 65 66"
                 << "S: A000005 OK search done";
        callNames.clear();
        callNames << QStringLiteral("itemChangeCommitted");
        QTest::newRow("no APPENDUID, message contained non-unique Message-ID") << item << collection << scenario << callNames;

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 APPEND \"INBOX/Foo\" {55}\r\n" + message->encodedContent(true) << "S: A000003 OK append done"
                 << "C: A000004 SELECT \"INBOX/Foo\""
                 << "S: A000004 OK select done"
                 << "C: A000005 UID SEARCH NEW UID 63:*"
                 << "S: * SEARCH 66"
                 << "S: A000005 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemChangeCommitted");

        QTest::newRow("no APPENDUID, message didn't contain Message-ID") << item << collection << scenario << callNames;
    }

    void shouldAppendMessage()
    {
        QFETCH(Akonadi::Item, item);
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
        state->setItem(item);
        auto task = new AddItemTask(state);
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

QTEST_GUILESS_MAIN(TestAddItemTask)

#include "testadditemtask.moc"
