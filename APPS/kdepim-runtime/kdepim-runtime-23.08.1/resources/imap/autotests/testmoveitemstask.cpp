/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "moveitemstask.h"
#include "uidnextattribute.h"

#include <KMime/Message>

#include <QTest>

Q_DECLARE_METATYPE(QSet<QByteArray>)

class TestMoveItemsTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldCopyAndDeleteMessage_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Akonadi::Collection>("source");
        QTest::addColumn<Akonadi::Collection>("target");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");

        Akonadi::Item item;
        Akonadi::Collection inbox;
        Akonadi::Collection source;
        Akonadi::Collection target;
        QList<QByteArray> scenario;
        QStringList callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        KMime::Message::Ptr message(new KMime::Message);

        QString messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        inbox = createCollectionChain(QStringLiteral("/INBOX"));
        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        target = Akonadi::Collection(4);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID COPY 5 \"INBOX/Bar\""
                 << "S: A000004 OK copy [ COPYUID 1239890035 5 65 ]"
                 << "C: A000005 UID STORE 5 +FLAGS (\\Deleted)"
                 << "S: A000005 OK store done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted");

        QTest::newRow("moving mail") << item << source << target << scenario << callNames;

        // Same item and collections
        // The scenario changes though

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID COPY 5 \"INBOX/Bar\""
                 << "S: A000004 OK copy [ COPYUID 1239890035 5 65 ]"
                 << "C: A000005 UID STORE 5 +FLAGS (\\Deleted)"
                 << "S: A000005 NO store failed";

        callNames.clear();
        callNames << QStringLiteral("emitWarning") << QStringLiteral("itemsChangesCommitted");

        QTest::newRow("moving mail, store fails") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\nMessage-ID: <42.4242.foo@bar.org>\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        source.addAttribute(new UidNextAttribute(42));
        target = Akonadi::Collection(3);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);
        target.addAttribute(new UidNextAttribute(65));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID COPY 5 \"INBOX/Bar\""
                 << "S: A000004 OK copy done"
                 << "C: A000005 UID STORE 5 +FLAGS (\\Deleted)"
                 << "S: A000005 OK store done"
                 << "C: A000006 SELECT \"INBOX/Bar\""
                 << "S: A000006 OK select done"
                 << "C: A000007 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 65"
                 << "S: A000007 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message had Message-ID") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        source.addAttribute(new UidNextAttribute(42));
        target = Akonadi::Collection(4);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);
        target.addAttribute(new UidNextAttribute(65));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID COPY 5 \"INBOX/Bar\""
                 << "S: A000004 OK copy done"
                 << "C: A000005 UID STORE 5 +FLAGS (\\Deleted)"
                 << "S: A000005 OK store done"
                 << "C: A000006 SELECT \"INBOX/Bar\""
                 << "S: A000006 OK select done"
                 << "C: A000007 UID SEARCH NEW UID 65:*"
                 << "S: * SEARCH 65"
                 << "S: A000007 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message didn't have Message-ID") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));
        message = KMime::Message::Ptr(new KMime::Message);
        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\nMessage-ID: <42.4242.foo@bar.org>\n\nSpeechless...");
        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID COPY 5 \"INBOX/Bar\""
                 << "S: A000004 OK copy done"
                 << "C: A000005 UID STORE 5 +FLAGS (\\Deleted)"
                 << "S: A000005 OK store done"
                 << "C: A000006 SELECT \"INBOX/Bar\""
                 << "S: A000006 OK select done"
                 << "C: A000007 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 61 65"
                 << "S: A000007 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message didn't have unique Message-ID, but last one matches old uidnext")
            << item << source << target << scenario << callNames;
    }

    void shouldCopyAndDeleteMessage()
    {
        QFETCH(Akonadi::Item, item);
        QFETCH(Akonadi::Collection, source);
        QFETCH(Akonadi::Collection, target);
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
        state->setItem(item);
        state->setSourceCollection(source);
        state->setTargetCollection(target);
        auto task = new MoveItemsTask(state);
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

    void shouldMoveMessage_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Akonadi::Collection>("source");
        QTest::addColumn<Akonadi::Collection>("target");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");

        Akonadi::Item item;
        Akonadi::Collection inbox;
        Akonadi::Collection source;
        Akonadi::Collection target;
        QList<QByteArray> scenario;
        QStringList callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        KMime::Message::Ptr message(new KMime::Message);

        QString messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        inbox = createCollectionChain(QStringLiteral("/INBOX"));
        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        target = Akonadi::Collection(4);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);

        scenario.clear();
        scenario << defaultPoolConnectionScenario({"MOVE"}) << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID MOVE 5 \"INBOX/Bar\""
                 << "S: A000004 OK move [ COPYUID 1239890035 5 65 ]";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted");

        QTest::newRow("moving mail") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\nMessage-ID: <42.4242.foo@bar.org>\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        source.addAttribute(new UidNextAttribute(42));
        target = Akonadi::Collection(3);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);
        target.addAttribute(new UidNextAttribute(65));

        scenario.clear();
        scenario << defaultPoolConnectionScenario({"MOVE"}) << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID MOVE 5 \"INBOX/Bar\""
                 << "S: A000004 OK MOVE done"
                 << "C: A000005 SELECT \"INBOX/Bar\""
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 65"
                 << "S: A000006 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message had Message-ID") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));

        message = KMime::Message::Ptr(new KMime::Message);

        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\n\nSpeechless...");

        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        source = Akonadi::Collection(3);
        source.setRemoteId(QStringLiteral("/Foo"));
        source.setParentCollection(inbox);
        source.addAttribute(new UidNextAttribute(42));
        target = Akonadi::Collection(4);
        target.setRemoteId(QStringLiteral("/Bar"));
        target.setParentCollection(inbox);
        target.addAttribute(new UidNextAttribute(65));

        scenario.clear();
        scenario << defaultPoolConnectionScenario({"MOVE"}) << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID MOVE 5 \"INBOX/Bar\""
                 << "S: A000004 OK MOVE done"
                 << "C: A000005 SELECT \"INBOX/Bar\""
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH NEW UID 65:*"
                 << "S: * SEARCH 65"
                 << "S: A000006 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message didn't have Message-ID") << item << source << target << scenario << callNames;

        item = Akonadi::Item(1);
        item.setRemoteId(QStringLiteral("5"));
        message = KMime::Message::Ptr(new KMime::Message);
        messageContent = QStringLiteral("From: ervin\nTo: someone\nSubject: foo\nMessage-ID: <42.4242.foo@bar.org>\n\nSpeechless...");
        message->setContent(messageContent.toUtf8());
        message->parse();
        item.setPayload(message);

        scenario.clear();
        scenario << defaultPoolConnectionScenario({"MOVE"}) << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 UID MOVE 5 \"INBOX/Bar\""
                 << "S: A000004 OK MOVE done"
                 << "C: A000005 SELECT \"INBOX/Bar\""
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH HEADER Message-ID \"<42.4242.foo@bar.org>\""
                 << "S: * SEARCH 61 65"
                 << "S: A000006 OK search done";

        callNames.clear();
        callNames << QStringLiteral("itemsChangesCommitted") << QStringLiteral("applyCollectionChanges");

        QTest::newRow("moving mail, no COPYUID, message didn't have unique Message-ID, but last one matches old uidnext")
            << item << source << target << scenario << callNames;
    }

    void shouldMoveMessage()
    {
        QFETCH(Akonadi::Item, item);
        QFETCH(Akonadi::Collection, source);
        QFETCH(Akonadi::Collection, target);
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
        state->setItem(item);
        state->setSourceCollection(source);
        state->setTargetCollection(target);
        state->setServerCapabilities({QStringLiteral("MOVE")});
        auto task = new MoveItemsTask(state);
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

QTEST_GUILESS_MAIN(TestMoveItemsTask)

#include "testmoveitemstask.moc"
