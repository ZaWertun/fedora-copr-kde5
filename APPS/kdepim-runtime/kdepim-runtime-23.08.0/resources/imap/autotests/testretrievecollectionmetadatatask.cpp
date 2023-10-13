/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "retrievecollectionmetadatatask.h"

#include "imapaclattribute.h"
#include "imapquotaattribute.h"
#include "noselectattribute.h"
#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionAnnotationsAttribute>
#include <Akonadi/CollectionQuotaAttribute>
#include <QTest>
#include <noinferiorsattribute.h>
using QBYTEARRAYMAP = QMap<QByteArray, QByteArray>;
using QBYTEARRAYINT64MAP = QMap<QByteArray, qint64>;

Q_DECLARE_METATYPE(Akonadi::Collection::Rights)
Q_DECLARE_METATYPE(QBYTEARRAYMAP)

class TestRetrieveCollectionMetadataTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        Akonadi::AttributeFactory::registerAttribute<Akonadi::ImapAclAttribute>();
        Akonadi::AttributeFactory::registerAttribute<NoSelectAttribute>();
    }

    void shouldCollectionRetrieveMetadata_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QStringList>("capabilities");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<Akonadi::Collection::Rights>("expectedRights");
        QTest::addColumn<QBYTEARRAYMAP>("expectedAnnotations");
        QTest::addColumn<QList<QByteArray>>("expectedRoots");
        QTest::addColumn<QList<QBYTEARRAYINT64MAP>>("expectedLimits");
        QTest::addColumn<QList<QBYTEARRAYINT64MAP>>("expectedUsages");

        Akonadi::Collection collection;
        QStringList capabilities;
        QList<QByteArray> scenario;
        QStringList callNames;
        QMap<QByteArray, QByteArray> expectedAnnotations;
        QList<QByteArray> expectedRoots;
        QList<QMap<QByteArray, qint64>> expectedLimits;
        QList<QMap<QByteArray, qint64>> expectedUsages;

        expectedRoots.clear();
        expectedLimits.clear();
        expectedUsages.clear();

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setRights(Akonadi::Collection::ReadOnly);

        capabilities.clear();
        capabilities << QStringLiteral("ANNOTATEMORE") << QStringLiteral("ACL") << QStringLiteral("QUOTA");

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 GETANNOTATION "INBOX/Foo" "*" "value.shared")"
                 << "S: * ANNOTATION INBOX/Foo /vendor/kolab/folder-test ( value.shared true )"
                 << "S: A000003 OK annotations retrieved"
                 << "C: A000004 MYRIGHTS \"INBOX/Foo\""
                 << "S: * MYRIGHTS \"INBOX/Foo\" lrswipkxtecda"
                 << "S: A000004 OK rights retrieved"
                 << "C: A000005 GETQUOTAROOT \"INBOX/Foo\""
                 << "S: * QUOTAROOT INBOX/Foo user/foo"
                 << "S: * QUOTA user/foo ( )"
                 << "S: A000005 OK quota retrieved"
                 << "C: A000006 GETACL \"INBOX/Foo\""
                 << "S: * ACL INBOX/Foo foo@kde.org lrswipcda"
                 << "S: A000006 OK acl retrieved";

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        expectedAnnotations.clear();
        expectedAnnotations.insert("/shared/vendor/kolab/folder-test", "true");

        Akonadi::Collection::Rights rights = Akonadi::Collection::AllRights;
        QTest::newRow("first listing, connected IMAP")
            << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots << expectedLimits << expectedUsages;

        //
        // Test that if the parent collection doesn't allow renaming in its ACL, the child mailbox
        // can't be renamed, i.e. doesn't have the CanChangeCollection flag.
        //
        Akonadi::Collection parentCollection = createCollectionChain(QStringLiteral("/INBOX"));
        QMap<QByteArray, KIMAP::Acl::Rights> rightsMap;
        rightsMap.insert("Hans",
                         KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen | KIMAP::Acl::Write | KIMAP::Acl::Insert | KIMAP::Acl::Post
                             | KIMAP::Acl::Delete);
        auto aclAttribute = new Akonadi::ImapAclAttribute();
        aclAttribute->setRights(rightsMap);
        parentCollection.addAttribute(aclAttribute);
        collection.setParentCollection(parentCollection);
        rights = Akonadi::Collection::AllRights;
        rights &= ~Akonadi::Collection::CanChangeCollection;
        QTest::newRow("parent without create rights") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots
                                                      << expectedLimits << expectedUsages;

        //
        // Test that if the parent collection is a noselect folder, the child mailbox will not have
        // rename (CanChangeCollection) permission.
        //
        parentCollection = createCollectionChain(QStringLiteral("/INBOX"));
        auto noSelectAttribute = new NoSelectAttribute();
        parentCollection.addAttribute(noSelectAttribute);
        collection.setParentCollection(parentCollection);
        QTest::newRow("parent with noselect") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots
                                              << expectedLimits << expectedUsages;
        parentCollection.removeAttribute<NoSelectAttribute>();

        //
        // Test that the rights are properly set on the resulting collection if the mailbox doesn't
        // have full rights.
        //
        collection.setParentCollection(createCollectionChain(QStringLiteral("/INBOX")));
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 GETANNOTATION "INBOX/Foo" "*" "value.shared")"
                 << "S: * ANNOTATION INBOX/Foo /vendor/kolab/folder-test ( value.shared true )"
                 << "S: A000003 OK annotations retrieved"
                 << "C: A000004 MYRIGHTS \"INBOX/Foo\""
                 << "S: * MYRIGHTS \"INBOX/Foo\" wi"
                 << "S: A000004 OK rights retrieved"
                 << "C: A000005 GETQUOTAROOT \"INBOX/Foo\""
                 << "S: * QUOTAROOT INBOX/Foo user/foo"
                 << "S: * QUOTA user/foo ( )"
                 << "S: A000005 OK quota retrieved";
        rights = Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanChangeItem | Akonadi::Collection::CanChangeCollection;
        QTest::newRow("only some rights") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots
                                          << expectedLimits << expectedUsages;

        //
        // Test that a warning is issued if the insert rights of a folder have been revoked on the server.
        //
        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setParentCollection(createCollectionChain(QStringLiteral("/INBOX")));
        // We use the aclattribute to determine if a collection already has acl's or not
        collection.addAttribute(new Akonadi::ImapAclAttribute());
        collection.setRights(Akonadi::Collection::CanCreateItem);

        capabilities.clear();
        capabilities << QStringLiteral("ANNOTATEMORE") << QStringLiteral("ACL") << QStringLiteral("QUOTA");

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 GETANNOTATION "INBOX/Foo" "*" "value.shared")"
                 << "S: * ANNOTATION INBOX/Foo /vendor/kolab/folder-test ( value.shared true )"
                 << "S: A000003 OK annotations retrieved"
                 << "C: A000004 MYRIGHTS \"INBOX/Foo\""
                 << "S: * MYRIGHTS \"INBOX/Foo\" w"
                 << "S: A000004 OK rights retrieved"
                 << "C: A000005 GETQUOTAROOT \"INBOX/Foo\""
                 << "S: * QUOTAROOT INBOX/Foo user/foo"
                 << "S: * QUOTA user/foo ( )"
                 << "S: A000005 OK quota retrieved";

        callNames.clear();
        callNames << QStringLiteral("showInformationDialog");
        callNames << QStringLiteral("collectionAttributesRetrieved");

        rights = Akonadi::Collection::CanChangeItem | Akonadi::Collection::CanChangeCollection;
        QTest::newRow("revoked rights") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots
                                        << expectedLimits << expectedUsages;

        //
        // Test that NoInferiors overrides acl rights and disallows creating new mailboxes
        //
        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setParentCollection(createCollectionChain(QString()));
        collection.setRemoteId(QStringLiteral("/INBOX"));
        collection.setRights(Akonadi::Collection::AllRights);
        collection.addAttribute(new NoInferiorsAttribute(true));
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << R"(C: A000003 GETANNOTATION "INBOX" "*" "value.shared")"
                 << "S: * ANNOTATION INBOX /vendor/kolab/folder-test ( value.shared true )"
                 << "S: A000003 OK annotations retrieved"
                 << "C: A000004 MYRIGHTS \"INBOX\""
                 << "S: * MYRIGHTS \"INBOX\" wk"
                 << "S: A000004 OK rights retrieved"
                 << "C: A000005 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX user"
                 << "S: * QUOTA user ( )"
                 << "S: A000005 OK quota retrieved";

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        rights = Akonadi::Collection::CanChangeItem | Akonadi::Collection::CanChangeCollection;

        QTest::newRow("noinferiors") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots << expectedLimits
                                     << expectedUsages;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setRights(Akonadi::Collection::ReadOnly);

        capabilities.clear();
        capabilities << QStringLiteral("METADATA") << QStringLiteral("ACL") << QStringLiteral("QUOTA");

        expectedAnnotations.clear();
        expectedAnnotations.insert("/shared/vendor/kolab/folder-test", "true");
        expectedAnnotations.insert("/shared/vendor/kolab/folder-test2", "");

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 GETMETADATA (DEPTH infinity) \"INBOX/Foo\" (/shared)"
                 << R"(S: * METADATA "INBOX/Foo" (/shared/vendor/kolab/folder-test "true"))"
                 << R"(S: * METADATA "INBOX/Foo" (/shared/vendor/kolab/folder-test2 "NIL"))"
                 << R"(S: * METADATA "INBOX/Foo" (/shared/vendor/cmu/cyrus-imapd/lastupdate "true"))"
                 << "S: A000003 OK GETMETADATA complete"
                 << "C: A000004 MYRIGHTS \"INBOX/Foo\""
                 << "S: * MYRIGHTS \"INBOX/Foo\" lrswipkxtecda"
                 << "S: A000004 OK rights retrieved"
                 << "C: A000005 GETQUOTAROOT \"INBOX/Foo\""
                 << "S: * QUOTAROOT INBOX/Foo user/Foo"
                 << "S: * QUOTA user/Foo ( )"
                 << "S: A000005 OK quota retrieved"
                 << "C: A000006 GETACL \"INBOX/Foo\""
                 << "S: * ACL INBOX/Foo foo@kde.org lrswipcda"
                 << "S: A000006 OK acl retrieved";

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        rights = Akonadi::Collection::AllRights;
        QTest::newRow("METADATA") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots << expectedLimits
                                  << expectedUsages;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setRights(Akonadi::Collection::ReadOnly);

        capabilities.clear();
        expectedAnnotations.clear();

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        rights = Akonadi::Collection::ReadOnly;

        scenario.clear();
        scenario << defaultPoolConnectionScenario();

        QTest::newRow("no capabilities") << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots
                                         << expectedLimits << expectedUsages;

        //
        // Test for GETQUOTAROOT with multiple IMAP quota roots but only one QUOTA response.
        //
        collection = createCollectionChain(QStringLiteral("/INBOX"));
        collection.setRights(Akonadi::Collection::ReadOnly);

        capabilities.clear();
        capabilities << QStringLiteral("QUOTA");

        rights = Akonadi::Collection::ReadOnly;

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX mailbox1 mailbox2 mailbox3"
                 << "S: * QUOTA mailbox2 ( STORAGE 21 512 )"
                 << "S: A000003 OK quota retrieved";

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        expectedAnnotations.clear();

        expectedRoots = {"mailbox2"};
        expectedUsages = {{{"STORAGE", 21}}};
        expectedLimits = {{{"STORAGE", 512}}};

        QTest::newRow("multiple quota roots, one resource only")
            << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots << expectedLimits << expectedUsages;

        //
        // Test for GETQUOTAROOT with multiple IMAP quota roots, some with no resource,
        // but only (not the first on the list) matches the mailbox name.
        //
        collection = createCollectionChain(QStringLiteral("/INBOX"));
        collection.setRights(Akonadi::Collection::ReadOnly);

        capabilities.clear();
        capabilities << QStringLiteral("QUOTA");

        rights = Akonadi::Collection::ReadOnly;

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX mailbox1 INBOX mailbox2 mailbox3"
                 << "S: * QUOTA mailbox1 ( STORAGE 21 512 )"
                 << "S: * QUOTA INBOX ( STORAGE 11 250 )"
                 << "S: * QUOTA mailbox2 (  )"
                 << "S: * QUOTA mailbox3 ( STORAGE 31 500 )"
                 << "S: A000003 OK quota retrieved";

        callNames.clear();
        callNames << QStringLiteral("collectionAttributesRetrieved");

        expectedAnnotations.clear();

        expectedRoots = {"mailbox1", "INBOX", "mailbox3"};
        expectedUsages = {{{"STORAGE", 21}}, {{"STORAGE", 11}}, {{"STORAGE", 31}}};
        expectedLimits = {{{"STORAGE", 512}}, {{"STORAGE", 250}}, {{"STORAGE", 500}}};

        QTest::newRow("multiple quota roots, some with no resources, one matches the mailbox name")
            << collection << capabilities << scenario << callNames << rights << expectedAnnotations << expectedRoots << expectedLimits << expectedUsages;
    }

    void shouldCollectionRetrieveMetadata()
    {
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QStringList, capabilities);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(Akonadi::Collection::Rights, expectedRights);
        QFETCH(QBYTEARRAYMAP, expectedAnnotations);
        QFETCH(QList<QByteArray>, expectedRoots);
        QFETCH(QList<QBYTEARRAYINT64MAP>, expectedLimits);
        QFETCH(QList<QBYTEARRAYINT64MAP>, expectedUsages);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setCollection(collection);
        state->setServerCapabilities(capabilities);
        state->setUserName(QStringLiteral("Hans"));
        auto task = new RetrieveCollectionMetadataTask(state);

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

            if (command == QLatin1String("collectionAttributesRetrieved")) {
                auto collection = parameter.value<Akonadi::Collection>();
                QCOMPARE(collection.rights(), expectedRights);

                if (!expectedAnnotations.isEmpty()) {
                    QVERIFY(collection.hasAttribute<Akonadi::CollectionAnnotationsAttribute>());
                    QCOMPARE(collection.attribute<Akonadi::CollectionAnnotationsAttribute>()->annotations(), expectedAnnotations);
                }

                if (!expectedRoots.isEmpty()) {
                    int index = expectedRoots.indexOf("INBOX");

                    QVERIFY(collection.hasAttribute<Akonadi::ImapQuotaAttribute>());
                    QCOMPARE(collection.attribute<Akonadi::ImapQuotaAttribute>()->roots(), expectedRoots);
                    QCOMPARE(collection.attribute<Akonadi::ImapQuotaAttribute>()->limits(), expectedLimits);
                    QCOMPARE(collection.attribute<Akonadi::ImapQuotaAttribute>()->usages(), expectedUsages);

                    QVERIFY(collection.hasAttribute<Akonadi::CollectionQuotaAttribute>());
                    if (index != -1) {
                        QCOMPARE(collection.attribute<Akonadi::CollectionQuotaAttribute>()->currentValue(), expectedUsages.at(index)["STORAGE"] * 1024);
                        QCOMPARE(collection.attribute<Akonadi::CollectionQuotaAttribute>()->maximumValue(), expectedLimits.at(index)["STORAGE"] * 1024);
                    } else {
                        QCOMPARE(collection.attribute<Akonadi::CollectionQuotaAttribute>()->currentValue(), expectedUsages.first()["STORAGE"] * 1024);
                        QCOMPARE(collection.attribute<Akonadi::CollectionQuotaAttribute>()->maximumValue(), expectedLimits.first()["STORAGE"] * 1024);
                    }
                }
            }
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_GUILESS_MAIN(TestRetrieveCollectionMetadataTask)

#include "testretrievecollectionmetadatatask.moc"
