/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "changecollectiontask.h"

#include "imapaclattribute.h"
#include <Akonadi/CollectionAnnotationsAttribute>
#include <QTest>
Q_DECLARE_METATYPE(QSet<QByteArray>)

class TestChangeCollectionTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldUpdateMetadataAclAndName_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QSet<QByteArray>>("parts");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<QString>("collectionName");
        QTest::addColumn<QStringList>("caps");

        Akonadi::Collection collection;
        QSet<QByteArray> parts;
        QList<QByteArray> scenario;
        QStringList callNames;
        QStringList caps;

        collection = createCollectionChain(QStringLiteral("/Foo"));
        collection.setName(QStringLiteral("Bar"));
        collection.setRights(Akonadi::Collection::AllRights);

        auto acls = new Akonadi::ImapAclAttribute;
        QMap<QByteArray, KIMAP::Acl::Rights> rights;
        // Old rights
        rights["test@kdab.com"] = KIMAP::Acl::rightsFromString("lrswipckxtda");
        rights["foo@kde.org"] = KIMAP::Acl::rightsFromString("lrswipcda");
        acls->setRights(rights);

        // New rights
        rights["test@kdab.com"] = KIMAP::Acl::rightsFromString("lrswipckxtda");
        rights["foo@kde.org"] = KIMAP::Acl::rightsFromString("lrswipcda");
        acls->setRights(rights);
        collection.addAttribute(acls);

        auto annotationsAttr = new Akonadi::CollectionAnnotationsAttribute;
        QMap<QByteArray, QByteArray> annotations;
        annotations["/vendor/kolab/folder-test"] = "false";
        annotations["/vendor/kolab/folder-test2"] = "true";
        annotationsAttr->setAnnotations(annotations);
        collection.addAttribute(annotationsAttr);

        parts << "NAME"
              << "AccessRights"
              << "imapacl"
              << "collectionannotations";

        caps << QStringLiteral("ACL") << QStringLiteral("ANNOTATEMORE");

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000003 OK acl changed"
                 << R"(C: A000004 SETANNOTATION "Foo" "/vendor/kolab/folder-test" ("value.shared" "false"))"
                 << "S: A000004 OK annotations changed"
                 << R"(C: A000005 SETANNOTATION "Foo" "/vendor/kolab/folder-test2" ("value.shared" "true"))"
                 << "S: A000005 OK annotations changed"
                 << R"(C: A000006 SETACL "Foo" "foo@kde.org" "lrswipcda")"
                 << "S: A000006 OK acl changed"
                 << R"(C: A000007 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000007 OK acl changed"
                 << R"(C: A000008 RENAME "Foo" "Bar")"
                 << "S: A000008 OK rename done"
                 << "C: A000009 SUBSCRIBE \"Bar\""
                 << "S: A000009 OK mailbox subscribed";

        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted");

        QTest::newRow("complete case") << collection << parts << scenario << callNames << collection.name() << caps;

        caps.clear();
        caps << QStringLiteral("ACL");
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000003 OK acl changed"
                 << R"(C: A000004 SETACL "Foo" "foo@kde.org" "lrswipcda")"
                 << "S: A000004 OK acl changed"
                 << R"(C: A000005 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000005 OK acl changed"
                 << R"(C: A000006 RENAME "Foo" "Bar")"
                 << "S: A000006 OK rename done"
                 << "C: A000007 SUBSCRIBE \"Bar\""
                 << "S: A000007 OK mailbox subscribed";
        QTest::newRow("no ANNOTATEMORE support") << collection << parts << scenario << callNames << collection.name() << caps;

        collection = createCollectionChain(QStringLiteral("/Foo"));
        collection.setName(QStringLiteral("Bar/Baz"));
        caps.clear();
        caps << QStringLiteral("ACL") << QStringLiteral("ANNOTATEMORE");
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 RENAME "Foo" "BarBaz")"
                 << "S: A000003 OK rename done"
                 << "C: A000004 SUBSCRIBE \"BarBaz\""
                 << "S: A000004 OK mailbox subscribed";
        parts.clear();
        parts << "NAME";
        callNames.clear();
        callNames << QStringLiteral("collectionChangeCommitted");
        QTest::newRow("rename with invalid separator") << collection << parts << scenario << callNames << "BarBaz" << caps;

        collection = createCollectionChain(QStringLiteral(".INBOX.Foo"));
        collection.setName(QStringLiteral("Bar"));
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 RENAME "INBOX.Foo" "INBOX.Bar")"
                 << "S: A000003 OK rename done"
                 << "C: A000004 SUBSCRIBE \"INBOX.Bar\""
                 << "S: A000004 OK mailbox subscribed";
        QTest::newRow("rename with non-standard separator") << collection << parts << scenario << callNames << "Bar" << caps;

        collection = createCollectionChain(QStringLiteral("/Foo"));
        collection.setName(QStringLiteral("Bar"));
        collection.setRights(Akonadi::Collection::AllRights);

        acls = new Akonadi::ImapAclAttribute;
        // Old rights
        rights["test@kdab.com"] = KIMAP::Acl::rightsFromString("lrswipckxtda");
        rights["foo@kde.org"] = KIMAP::Acl::rightsFromString("lrswipcda");
        acls->setRights(rights);

        // New rights
        rights["test@kdab.com"] = KIMAP::Acl::rightsFromString("lrswipckxtda");
        rights["foo@kde.org"] = KIMAP::Acl::rightsFromString("lrswipcda");
        acls->setRights(rights);
        collection.addAttribute(acls);

        annotationsAttr = new Akonadi::CollectionAnnotationsAttribute;
        annotations["/vendor/kolab/folder-test"] = "false";
        annotations["/vendor/kolab/folder-test2"] = "true";
        annotationsAttr->setAnnotations(annotations);
        collection.addAttribute(annotationsAttr);

        parts << "NAME"
              << "AccessRights"
              << "imapacl"
              << "collectionannotations";

        caps << QStringLiteral("ACL") << QStringLiteral("METADATA");

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000003 OK acl changed"
                 << R"(C: A000004 SETMETADATA "Foo" ("/shared/vendor/kolab/folder-test" "false"))"
                 << "S: A000004 OK SETMETADATA complete"
                 << R"(C: A000005 SETMETADATA "Foo" ("/shared/vendor/kolab/folder-test2" "true"))"
                 << "S: A000005 OK SETMETADATA complete"
                 << R"(C: A000006 SETACL "Foo" "foo@kde.org" "lrswipcda")"
                 << "S: A000006 OK acl changed"
                 << R"(C: A000007 SETACL "Foo" "test@kdab.com" "lrswipckxtda")"
                 << "S: A000007 OK acl changed"
                 << R"(C: A000008 RENAME "Foo" "Bar")"
                 << "S: A000008 OK rename done"
                 << "C: A000009 SUBSCRIBE \"Bar\""
                 << "S: A000009 OK mailbox subscribed";
        QTest::newRow("complete case METADATA") << collection << parts << scenario << callNames << collection.name() << caps;
    }

    void shouldUpdateMetadataAclAndName()
    {
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QSet<QByteArray>, parts);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(QString, collectionName);
        QFETCH(QStringList, caps);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setUserName(defaultUserName());
        state->setServerCapabilities(caps);
        state->setCollection(collection);
        state->setParts(parts);
        auto task = new ChangeCollectionTask(state);
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
            if (command == QLatin1String("collectionChangeCommitted")) {
                QCOMPARE(parameter.value<Akonadi::Collection>().name(), collectionName);
                QCOMPARE(parameter.value<Akonadi::Collection>().remoteId().right(collectionName.length()), collectionName);
            }
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_GUILESS_MAIN(TestChangeCollectionTask)

#include "testchangecollectiontask.moc"
