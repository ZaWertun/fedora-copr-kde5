/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "noselectattribute.h"
#include "retrievecollectionstask.h"
#include <noinferiorsattribute.h>

#include <Akonadi/MessageParts>

#include <Akonadi/CachePolicy>
#include <Akonadi/EntityDisplayAttribute>

#include <QTest>

class TestRetrieveCollectionsTask : public ImapTestBase
{
    Q_OBJECT
public:
    TestRetrieveCollectionsTask(QObject *parent = nullptr)
        : ImapTestBase(parent)
        , m_nextCollectionId(1)
    {
    }

private Q_SLOTS:
    void shouldListCollections_data()
    {
        QTest::addColumn<Akonadi::Collection::List>("expectedCollections");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<bool>("isSubscriptionEnabled");
        QTest::addColumn<bool>("isDisconnectedModeEnabled");
        QTest::addColumn<int>("intervalCheckTime");
        QTest::addColumn<char>("separator");

        Akonadi::Collection collection;

        Akonadi::Collection::List expectedCollections;
        QList<QByteArray> scenario;
        QStringList callNames;
        bool isSubscriptionEnabled;
        bool isDisconnectedModeEnabled;
        int intervalCheckTime;

        expectedCollections.clear();
        expectedCollections << createRootCollection() << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar/Private"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Archives"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: * LIST ( \\HasChildren ) / INBOX/Calendar"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( ) / INBOX/Archives"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("first listing, connected IMAP")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        expectedCollections.clear();
        expectedCollections << createRootCollection(true, 5) << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar/Private"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Archives"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: * LIST ( \\HasChildren ) / INBOX/Calendar"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( ) / INBOX/Archives"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = true;
        intervalCheckTime = 5;

        QTest::newRow("first listing, disconnected IMAP")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        expectedCollections.clear();
        expectedCollections << createRootCollection(true, 5) << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Archives"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: * LIST ( \\HasChildren ) / INBOX/"
                 << "S: * LIST ( \\HasChildren ) / INBOX/Archives"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = true;
        intervalCheckTime = 5;

        QTest::newRow("first listing, spurious INBOX/ (BR: 25342)")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        expectedCollections.clear();
        expectedCollections << createRootCollection() << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar"), true)
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar/Private"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Archives"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( ) / INBOX/Archives"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("auto-insert missing nodes in the tree")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( ) / INBOX/Archives"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("auto-insert missing nodes in the tree (reverse order)")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        expectedCollections.clear();
        expectedCollections << createRootCollection() << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar"))
                            << createCollection(QStringLiteral("/"), QStringLiteral("INBOX/Calendar/Private"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( ) / INBOX/Unsubscribed"
                 << "S: * LIST ( ) / INBOX/Calendar"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: A000003 OK list done"
                 << "C: A000004 LSUB \"\" *"
                 << "S: * LSUB ( \\HasChildren ) / INBOX"
                 << "S: * LSUB ( ) / INBOX/SubscribedButNotExisting"
                 << "S: * LSUB ( ) / INBOX/Calendar"
                 << "S: * LSUB ( ) / INBOX/Calendar/Private"
                 << "S: A000004 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = true;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("subscription enabled") << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled
                                              << intervalCheckTime << '/';
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( ) / INBOX/Unsubscribed"
                 << "S: * LIST ( ) / INBOX/Calendar"
                 << "S: * LIST ( ) / INBOX/Calendar/Private"
                 << "S: * LIST ( \\HasChildren ) / INBOX"
                 << "S: A000003 OK list done"
                 << "C: A000004 LSUB \"\" *"
                 << "S: * LSUB ( \\HasChildren ) / Inbox"
                 << "S: * LSUB ( ) / Inbox/SubscribedButNotExisting"
                 << "S: * LSUB ( ) / Inbox/Calendar"
                 << "S: * LSUB ( ) / Inbox/Calendar/Private"
                 << "S: A000004 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = true;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("subscription enabled, case insensitive inbox")
            << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime << '/';

        expectedCollections.clear();
        expectedCollections << createRootCollection() << createCollection(QStringLiteral("/"), QStringLiteral("INBOX"), false, true)
                            << createCollection(QStringLiteral("/"), QStringLiteral("Archive"));

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( \\Noinferiors ) / INBOX"
                 << "S: * LIST ( ) / Archive"
                 << "S: A000003 OK list done";

        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("Noinferiors") << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled << intervalCheckTime
                                     << '/';

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 LIST \"\" *"
                 << "S: * LIST ( ) . INBOX"
                 << "S: * LIST ( ) . INBOX.Foo"
                 << "S: * LIST ( ) . INBOX.Bar"
                 << "S: A000003 OK list done";
        callNames.clear();
        callNames << QStringLiteral("setIdleCollection") << QStringLiteral("collectionsRetrieved");

        expectedCollections.clear();
        expectedCollections << createRootCollection() << createCollection(QStringLiteral("."), QStringLiteral("INBOX"))
                            << createCollection(QStringLiteral("."), QStringLiteral("INBOX.Foo"))
                            << createCollection(QStringLiteral("."), QStringLiteral("INBOX.Bar"));
        isSubscriptionEnabled = false;
        isDisconnectedModeEnabled = false;
        intervalCheckTime = -1;

        QTest::newRow("non-standard separators") << expectedCollections << scenario << callNames << isSubscriptionEnabled << isDisconnectedModeEnabled
                                                 << intervalCheckTime << '.';
    }

    void shouldListCollections()
    {
        QFETCH(Akonadi::Collection::List, expectedCollections);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(bool, isSubscriptionEnabled);
        QFETCH(bool, isDisconnectedModeEnabled);
        QFETCH(int, intervalCheckTime);
        QFETCH(char, separator);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setResourceName(QStringLiteral("resource"));
        state->setSubscriptionEnabled(isSubscriptionEnabled);
        state->setDisconnectedModeEnabled(isDisconnectedModeEnabled);
        state->setIntervalCheckTime(intervalCheckTime);

        auto task = new RetrieveCollectionsTask(state);
        task->start(&pool);

        Akonadi::Collection::List collections;

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
            } else if (command == QLatin1String("collectionsRetrieved")) {
                collections += parameter.value<Akonadi::Collection::List>();
            }
        }

        QCOMPARE(state->separatorCharacter(), QChar::fromLatin1(separator));

        QVERIFY(server.isAllScenarioDone());
        compareCollectionLists(collections, expectedCollections);

        server.quit();
    }

private:
    qint64 m_nextCollectionId;

    Akonadi::Collection createRootCollection(bool isDisconnectedImap = false, int intervalCheck = -1)
    {
        // Root
        Akonadi::Collection collection = Akonadi::Collection(m_nextCollectionId++);
        collection.setName(QStringLiteral("resource"));
        collection.setRemoteId(QStringLiteral("root-id"));
        collection.setContentMimeTypes(QStringList(Akonadi::Collection::mimeType()));
        collection.setParentCollection(Akonadi::Collection::root());
        collection.addAttribute(new NoSelectAttribute(true));
        collection.setRights(Akonadi::Collection::CanCreateCollection);

        Akonadi::CachePolicy policy;
        policy.setInheritFromParent(false);
        policy.setSyncOnDemand(true);

        if (isDisconnectedImap) {
            policy.setLocalParts(QStringList() << QLatin1String(Akonadi::MessagePart::Envelope) << QLatin1String(Akonadi::MessagePart::Header)
                                               << QLatin1String(Akonadi::MessagePart::Body));
            policy.setCacheTimeout(-1);
        } else {
            policy.setLocalParts(QStringList() << QLatin1String(Akonadi::MessagePart::Envelope) << QLatin1String(Akonadi::MessagePart::Header));
            policy.setCacheTimeout(60);
        }

        policy.setIntervalCheckTime(intervalCheck);

        collection.setCachePolicy(policy);

        return collection;
    }

    Akonadi::Collection createCollection(const QString &separator, const QString &path, bool isNoSelect = false, bool isNoInferiors = false)
    {
        // No path? That's the root of this resource then
        if (path.isEmpty()) {
            return createRootCollection();
        }

        QStringList pathParts = path.split(separator);

        const QString pathPart = pathParts.takeLast();
        const QString parentPath = pathParts.join(separator);

        // Here we should likely reuse already produced collections if possible to be 100% accurate
        // but in the tests we check only a limited amount of properties (namely remote id and name).
        const Akonadi::Collection parentCollection = createCollection(separator, parentPath);

        Akonadi::Collection collection(m_nextCollectionId++);
        collection.setName(pathPart);
        collection.setRemoteId(separator + pathPart);

        collection.setParentCollection(parentCollection);
        collection.setContentMimeTypes(QStringList() << QStringLiteral("message/rfc822") << Akonadi::Collection::mimeType());

        // If the folder is the Inbox, make some special settings.
        if (pathPart.compare(QLatin1String("INBOX"), Qt::CaseInsensitive) == 0) {
            auto attr = new Akonadi::EntityDisplayAttribute;
            attr->setDisplayName(QStringLiteral("Inbox"));
            attr->setIconName(QStringLiteral("mail-folder-inbox"));
            collection.addAttribute(attr);
        }

        // If the folder is the user top-level folder, mark it as well, even although it is not officially noted in the RFC
        if ((pathPart.compare(QLatin1String("user"), Qt::CaseInsensitive) == 0) && isNoSelect) {
            auto attr = new Akonadi::EntityDisplayAttribute;
            attr->setDisplayName(QStringLiteral("Shared Folders"));
            attr->setIconName(QStringLiteral("x-mail-distribution-list"));
            collection.addAttribute(attr);
        }

        // If this folder is a noselect folder, make some special settings.
        if (isNoSelect) {
            collection.addAttribute(new NoSelectAttribute(true));
            collection.setContentMimeTypes(QStringList() << Akonadi::Collection::mimeType());
            collection.setRights(Akonadi::Collection::ReadOnly);
        }

        if (isNoInferiors) {
            collection.addAttribute(new NoInferiorsAttribute(true));
            collection.setRights(collection.rights() & ~Akonadi::Collection::CanCreateCollection);
        }

        return collection;
    }

    void compareCollectionLists(const Akonadi::Collection::List &resultList, const Akonadi::Collection::List &expectedList)
    {
        for (int i = 0; i < expectedList.size(); i++) {
            Akonadi::Collection expected = expectedList[i];
            bool found = false;

            for (int j = 0; j < resultList.size(); j++) {
                Akonadi::Collection result = resultList[j];

                if (result.remoteId() == expected.remoteId()) {
                    found = true;

                    QVERIFY(!result.name().isEmpty());

                    QCOMPARE(result.name(), expected.name());
                    QCOMPARE(result.contentMimeTypes(), expected.contentMimeTypes());
                    QCOMPARE(result.rights(), expected.rights());
                    if (expected.parentCollection() == Akonadi::Collection::root()) {
                        QCOMPARE(result.parentCollection(), expected.parentCollection());
                    } else {
                        QCOMPARE(result.parentCollection().remoteId(), expected.parentCollection().remoteId());
                    }

                    QCOMPARE(result.cachePolicy().inheritFromParent(), expected.cachePolicy().inheritFromParent());
                    QCOMPARE(result.cachePolicy().syncOnDemand(), expected.cachePolicy().syncOnDemand());
                    QCOMPARE(result.cachePolicy().localParts(), expected.cachePolicy().localParts());
                    QCOMPARE(result.cachePolicy().cacheTimeout(), expected.cachePolicy().cacheTimeout());
                    QCOMPARE(result.cachePolicy().intervalCheckTime(), expected.cachePolicy().intervalCheckTime());

                    QCOMPARE(result.hasAttribute<NoSelectAttribute>(), expected.hasAttribute<NoSelectAttribute>());
                    QCOMPARE(result.hasAttribute<Akonadi::EntityDisplayAttribute>(), expected.hasAttribute<Akonadi::EntityDisplayAttribute>());

                    break;
                }
            }

            QVERIFY2(found, QString::fromLatin1("%1 not found!").arg(expected.remoteId()).toUtf8().constData());
        }

        QCOMPARE(resultList.size(), expectedList.size());
    }
};

QTEST_GUILESS_MAIN(TestRetrieveCollectionsTask)

#include "testretrievecollectionstask.moc"
