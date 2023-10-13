/*

   SPDX-License-Identifier: GPL-2.0-or-later
*/

namespace Akonadi
{
class Tag;
}

unsigned int qHash(const Akonadi::Tag &tag);

#include "imaptestbase.h"

#include <Akonadi/Tag>

#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionQuotaAttribute>
#include <Akonadi/ServerManager>
#include <akonadi/qtest_akonadi.h>
//#include <AkonadiCore/virtualresource.h>
#include <Akonadi/TagCreateJob>
#include <kolabobject.h>

#include "kolabchangeitemstagstask.h"
#include "kolabhelpers.h"

using namespace Akonadi;

typedef QHash<QString, Akonadi::Item::List> Members;

Q_DECLARE_METATYPE(TagListAndMembers);
Q_DECLARE_METATYPE(Members);

struct TestTagConverter : public TagConverter {
    virtual KMime::Message::Ptr createMessage(const Akonadi::Tag &tag, const Akonadi::Item::List &items)
    {
        return KMime::Message::Ptr(new KMime::Message());
    }
};

class TestChangeItemsTagsTask : public ImapTestBase
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

        Akonadi::Tag tag("tagname");
        {
            Akonadi::TagCreateJob *createJob = new Akonadi::TagCreateJob(tag);
            AKVERIFYEXEC(createJob);
            tag = createJob->tag();
        }

        Akonadi::Item item(KMime::Message::mimeType());
        item.setRemoteId("20");
        item.setTag(tag);
        item = resource->createItem(item, mailcol);

        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");
        QTest::addColumn<Akonadi::Tag::List>("expectedTags");
        QTest::addColumn<Members>("expectedMembers");
        QTest::addColumn<DummyResourceState::Ptr>("resourceState");

        {
            QList<QByteArray> scenario;
            scenario << defaultPoolConnectionScenario();

            QStringList callNames;
            callNames << "changeProcessed";

            QHash<QString, Akonadi::Item::List> expectedMembers;

            DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
            state->setServerCapabilities(QStringList() << "METADATA"
                                                       << "ACL");
            state->setUserName("Hans");

            QTest::newRow("nothing changed") << scenario << callNames << Akonadi::Tag::List() << expectedMembers << state;
        }
        {
            KMime::Message::Ptr msg(new KMime::Message());

            const QByteArray &content = msg->encodedContent(true);
            QList<QByteArray> scenario;
            scenario << defaultPoolConnectionScenario() << "C: A000003 APPEND \"configuration\"  {" + QByteArray::number(content.size()) + "}"
                     << "S: A000003 OK append done [ APPENDUID 1239890035 65 ]";

            QStringList callNames;
            callNames << "changeProcessed";

            Akonadi::Tag expectedTag = tag;
            expectedTag.setRemoteId("7");

            QHash<QString, Akonadi::Item::List> expectedMembers;
            Akonadi::Item member;
            member.setRemoteId("20");
            member.setParentCollection(createCollectionChain("/INBOX"));
            expectedMembers.insert(expectedTag.remoteId(), (Akonadi::Item::List() << member));

            DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
            state->setServerCapabilities(QStringList() << "METADATA"
                                                       << "ACL");
            state->setUserName("Hans");
            state->setAddedTags(QSet<Akonadi::Tag>() << tag);

            QTest::newRow("list single tag") << scenario << callNames << (Akonadi::Tag::List() << expectedTag) << expectedMembers << state;
        }
    }

    void testRetrieveTags()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);
        QFETCH(Akonadi::Tag::List, expectedTags);
        QFETCH(Members, expectedMembers);
        QFETCH(DummyResourceState::Ptr, resourceState);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        KolabChangeItemsTagsTask *task = new KolabChangeItemsTagsTask(resourceState, QSharedPointer<TestTagConverter>(new TestTagConverter));

        task->start(&pool);

        QTRY_COMPARE(resourceState->calls().count(), callNames.size());
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(resourceState->calls().at(i).first);
            QVariant parameter = resourceState->calls().at(i).second;

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

    void testTagConverter()
    {
        TagConverter converter;
        Akonadi::Item item(KMime::Message::mimeType());
        KMime::Message::Ptr msg = KMime::Message::Ptr(new KMime::Message());
        msg->subject(true)->from7BitString("subject");

        msg->messageID(true)->from7BitString("<messageid@example.com>");
        msg->date(true)->setDateTime(QDateTime(QDate(2014, 12, 10), QTime(9, 8, 7)));
        item.setPayload<KMime::Message::Ptr>(msg);
        item.setRemoteId(QLatin1String("20"));
        item.setParentCollection(createCollectionChain("/INBOX"));
        const QString member = KolabHelpers::createMemberUrl(item, QLatin1String("localuser@localhost"));
        const QString expected = QLatin1String(
            "imap:///user/localuser%40localhost/INBOX/"
            "20?message-id=%3Cmessageid%40example.com%3E&subject=subject&date=Wed%2C%2010%20Dec%202014%2009%3A08%3A07%20%2B0000");
        QCOMPARE(member, expected);
    }
};

QTEST_AKONADIMAIN(TestChangeItemsTagsTask)

#include "testchangeitemstagstask.moc"
