/*

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "kolabretrievetagstask.h"

#include "kolabhelpers.h"
#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionQuotaAttribute>
#include <Akonadi/ServerManager>
#include <akonadi/qtest_akonadi.h>
#include <akonadi/virtualresource.h>
#include <kolab/kolabobject.h>

typedef QHash<QString, Akonadi::Item::List> Members;

Q_DECLARE_METATYPE(TagListAndMembers);
Q_DECLARE_METATYPE(Members);

class TestRetrieveTagsTask : public ImapTestBase
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();
    }

    void testRetrieveTags_data()
    {
        Akonadi::VirtualResource *resource = new Akonadi::VirtualResource(QLatin1String("akonadi_knut_resource_0"), this);

        Akonadi::Collection root;
        root.setName(QLatin1String("akonadi_knut_resource_0"));
        root.setContentMimeTypes(QStringList() << Akonadi::Collection::mimeType());
        root.setParentCollection(Akonadi::Collection::root());
        root.setRemoteId("root-id");
        root = resource->createRootCollection(root);

        Akonadi::Collection col;
        col.setName("Configuration");
        col.setContentMimeTypes(QStringList() << KolabHelpers::getMimeType(Kolab::ConfigurationType));
        col.setRemoteId("/configuration");
        col = resource->createCollection(col);

        Akonadi::Collection mailcol;
        mailcol.setName("INBOX");
        mailcol.setContentMimeTypes(QStringList() << KMime::Message::mimeType());
        mailcol.setRemoteId("/INBOX");
        mailcol = resource->createCollection(mailcol);

        Akonadi::Item item(KMime::Message::mimeType());
        item.setRemoteId("20");
        item = resource->createItem(item, mailcol);

        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<Akonadi::Tag::List>("expectedTags");
        QTest::addColumn<Members>("expectedMembers");

        QList<QByteArray> scenario;
        QStringList callNames;

        Akonadi::Tag tag;
        tag.setName("tagname");
        Kolab::KolabObjectWriter writer;
        QStringList members;
        members << QLatin1String(
            "imap:///user/john.doe%40example.org/INBOX/"
            "20?message-id=%3Cf06aa3345a25005380b47547ad161d36%40lhm.klab.cc%3E&date=Tue%2C+12+Aug+2014+20%3A42%3A59+%2B0200&subject=Re%3A+test");
        KMime::Message::Ptr msg = writer.writeTag(tag, members);
        // qCDebug(KOLABRESOURCE_LOG) << msg->encodedContent();

        const QByteArray &content = msg->encodedContent(true);
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"configuration\""
                 << "S: A000003 OK select done"
                 << "C: A000004 FETCH 1:* (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 7 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {"
                + QByteArray::number(content.size()) + "}\r\n" + content + " )"
                 << "S: A000004 OK fetch done";

        callNames.clear();
        callNames << "tagsRetrieved";

        Akonadi::Tag expectedTag = tag;
        expectedTag.setRemoteId("7");

        QHash<QString, Akonadi::Item::List> expectedMembers;
        Akonadi::Item member;
        member.setRemoteId("20");
        member.setParentCollection(createCollectionChain("/INBOX"));
        expectedMembers.insert(expectedTag.remoteId(), (Akonadi::Item::List() << member));

        QTest::newRow("list single tag") << scenario << callNames << (Akonadi::Tag::List() << expectedTag) << expectedMembers;
    }

    void testRetrieveTags()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(Akonadi::Tag::List, expectedTags);
        QFETCH(Members, expectedMembers);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setServerCapabilities(QStringList() << "METADATA"
                                                   << "ACL");
        state->setUserName("Hans");
        KolabRetrieveTagTask *task = new KolabRetrieveTagTask(state, KolabRetrieveTagTask::RetrieveTags);

        task->start(&pool);

        QTRY_COMPARE(state->calls().count(), callNames.size());
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(state->calls().at(i).first);
            QVariant parameter = state->calls().at(i).second;

            if (command == "cancelTask" && callNames[i] != "cancelTask") {
                qCDebug(KOLABRESOURCE_LOG) << "Got a cancel:" << parameter.toString();
            }

            QCOMPARE(command, callNames[i]);

            if (command == "tagsRetrieved") {
                QPair<Akonadi::Tag::List, QHash<QString, Akonadi::Item::List>> pair = parameter.value<TagListAndMembers>();
                Akonadi::Tag::List tags = pair.first;
                QHash<QString, Akonadi::Item::List> members = pair.second;
                QCOMPARE(tags.size(), expectedTags.size());
                for (int i = 0; i < tags.size(); i++) {
                    QCOMPARE(tags[i].name(), expectedTags[i].name());
                    QCOMPARE(tags[i].remoteId(), expectedTags[i].remoteId());
                    const Akonadi::Item::List memberlist = members.value(tags[i].remoteId());
                    const Akonadi::Item::List expectedMemberlist = expectedMembers.value(tags[i].remoteId());
                    QCOMPARE(memberlist.size(), expectedMemberlist.size());
                    for (int i = 0; i < expectedMemberlist.size(); i++) {
                        QCOMPARE(memberlist[i].remoteId(), expectedMemberlist[i].remoteId());
                        Akonadi::Collection parent = memberlist[i].parentCollection();
                        Akonadi::Collection expectedParent = expectedMemberlist[i].parentCollection();
                        while (expectedParent.isValid()) {
                            QCOMPARE(parent.remoteId(), expectedParent.remoteId());
                            expectedParent = expectedParent.parentCollection();
                            parent = parent.parentCollection();
                        }
                    }
                }
            }
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_AKONADIMAIN(TestRetrieveTagsTask, NoGUI)

#include "testretrievetagstask.moc"
