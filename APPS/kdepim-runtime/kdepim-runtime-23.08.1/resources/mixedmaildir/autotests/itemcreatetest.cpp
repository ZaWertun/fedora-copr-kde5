/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/itemcreatejob.h"
#include "filestore/itemfetchjob.h"

#include "libmaildir/maildir.h"

#include <KMbox/MBox>

#include <KMime/Message>

#include <QRandomGenerator>
#include <QTemporaryDir>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

class ItemCreateTest : public QObject
{
    Q_OBJECT

public:
    ItemCreateTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~ItemCreateTest() override
    {
        delete mStore;
        delete mDir;
    }

private:
    MixedMaildirStore *mStore = nullptr;
    QTemporaryDir *mDir = nullptr;

private Q_SLOTS:
    void init();
    void cleanup();
    void testExpectedFail();
    void testMBox();
    void testMaildir();
};

void ItemCreateTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void ItemCreateTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void ItemCreateTest::testExpectedFail()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("data")));
    QDir dataDir = topDir;
    QVERIFY(dataDir.cd(QLatin1String("data")));
    KPIM::Maildir dataMd(dataDir.path(), false);
    QVERIFY(dataMd.isValid());

    const QStringList dataEntryList = dataMd.entryList();
    QCOMPARE(dataEntryList.count(), 4);
    KMime::Message::Ptr msgPtr(new KMime::Message);
    msgPtr->setContent(KMime::CRLFtoLF(dataMd.readEntry(dataEntryList.first())));

    QVERIFY(topDir.mkdir(QLatin1String("store")));
    QVERIFY(topDir.cd(QLatin1String("store")));
    mStore->setPath(topDir.path());

    FileStore::ItemCreateJob *job = nullptr;

    // test failure of adding item to top level collection
    Item item;
    item.setMimeType(KMime::Message::mimeType());
    item.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->createItem(item, mStore->topLevelCollection());

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    // test failure of adding item to non existent collection
    Collection collection;
    collection.setName(QStringLiteral("collection"));
    collection.setRemoteId(QStringLiteral("collection"));
    collection.setParentCollection(mStore->topLevelCollection());

    job = mStore->createItem(item, collection);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
}

void ItemCreateTest::testMBox()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("data")));
    QDir dataDir = topDir;
    QVERIFY(dataDir.cd(QLatin1String("data")));
    KPIM::Maildir dataMd(dataDir.path(), false);
    QVERIFY(dataMd.isValid());

    const QStringList dataEntryList = dataMd.entryList();
    QCOMPARE(dataEntryList.count(), 4);
    KMime::Message::Ptr msgPtr1(new KMime::Message);
    msgPtr1->setContent(KMime::CRLFtoLF(dataMd.readEntry(dataEntryList.first())));
    KMime::Message::Ptr msgPtr2(new KMime::Message);
    msgPtr2->setContent(KMime::CRLFtoLF(dataMd.readEntry(dataEntryList.last())));

    QVERIFY(topDir.mkdir(QLatin1String("store")));
    QVERIFY(topDir.cd(QLatin1String("store")));

    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection1")));

    QFileInfo fileInfo1(topDir.path(), QStringLiteral("collection1"));
    KMBox::MBox mbox1;
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    QCOMPARE((int)mbox1.entries().count(), 4);
    const int size1 = fileInfo1.size();

    // simulate empty mbox
    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    QFile file2(fileInfo2.absoluteFilePath());
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.close();
    QVERIFY(file2.exists());
    QCOMPARE((int)file2.size(), 0);

    mStore->setPath(topDir.path());

    // common variables
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    FileStore::ItemCreateJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;

    // test adding to empty mbox
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setId(QRandomGenerator::global()->generate());
    item1.setMimeType(KMime::Message::mimeType());
    item1.setPayload<KMime::Message::Ptr>(msgPtr1);

    job = mStore->createItem(item1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();
    QCOMPARE(item.id(), item1.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.remoteId(), QStringLiteral("0"));
    QCOMPARE(item.parentCollection(), collection2);

    fileInfo2.refresh();
    QVERIFY(fileInfo2.size() > 0);
    const int size2 = fileInfo2.size();

    KMBox::MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    QCOMPARE((int)mbox2.entries().count(), 1);

    Item item2;
    item2.setId(QRandomGenerator::global()->generate());
    item2.setMimeType(KMime::Message::mimeType());
    item2.setPayload<KMime::Message::Ptr>(msgPtr2);

    job = mStore->createItem(item2, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.remoteId(), QString::number(size2 + 1));
    QCOMPARE(item.parentCollection(), collection2);

    fileInfo2.refresh();
    QVERIFY(fileInfo2.size() > 0);

    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    QCOMPARE((int)mbox2.entries().count(), 2);

    // test adding to non-empty mbox
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    job = mStore->createItem(item1, collection1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item1.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.remoteId(), QString::number(size1 + 1));
    QCOMPARE(item.parentCollection(), collection1);

    fileInfo1.refresh();
    QVERIFY(fileInfo1.size() > size1);

    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    QCOMPARE((int)mbox1.entries().count(), 5);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 5);
    for (const Item &item : std::as_const(items)) {
        const auto flags{item.flags()};
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    job = mStore->createItem(item2, collection1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.remoteId(), QString::number(size1 + 1 + size2 + 1));
    QCOMPARE(item.parentCollection(), collection1);

    fileInfo1.refresh();
    QVERIFY(fileInfo1.size() > (size1 + size2));

    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    QCOMPARE((int)mbox1.entries().count(), 6);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 6);
    for (const Item &item : std::as_const(items)) {
        const auto flags{item.flags()};
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();
}

void ItemCreateTest::testMaildir()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("data")));
    QDir dataDir = topDir;
    QVERIFY(dataDir.cd(QLatin1String("data")));
    KPIM::Maildir dataMd(dataDir.path(), false);
    QVERIFY(dataMd.isValid());

    const QStringList dataEntryList = dataMd.entryList();
    QCOMPARE(dataEntryList.count(), 4);
    KMime::Message::Ptr msgPtr1(new KMime::Message);
    msgPtr1->setContent(KMime::CRLFtoLF(dataMd.readEntry(dataEntryList.first())));
    KMime::Message::Ptr msgPtr2(new KMime::Message);
    msgPtr2->setContent(KMime::CRLFtoLF(dataMd.readEntry(dataEntryList.last())));

    QVERIFY(topDir.mkdir(QLatin1String("store")));
    QVERIFY(topDir.cd(QLatin1String("store")));

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));

    KPIM::Maildir topLevelMd(topDir.path(), true);
    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QVERIFY(md1.isValid());

    const QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    // simulate empty maildir
    KPIM::Maildir md2(topLevelMd.addSubFolder(QStringLiteral("collection2")), false);
    QVERIFY(md2.isValid());

    const QStringList md2EntryList = md2.entryList();
    QSet<QString> entrySet2(md2EntryList.cbegin(), md2EntryList.cend());
    QCOMPARE((int)entrySet2.count(), 0);

    mStore->setPath(topDir.path());

    // common variables
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    QStringList entryList;
    QSet<QString> entrySet;
    QSet<QString> newIdSet;
    QString newId;

    FileStore::ItemCreateJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;

    // test adding to empty maildir
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setId(QRandomGenerator::global()->generate());
    item1.setMimeType(KMime::Message::mimeType());
    item1.setPayload<KMime::Message::Ptr>(msgPtr1);

    job = mStore->createItem(item1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();
    QCOMPARE(item.id(), item1.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.parentCollection(), collection2);

    entryList = md2.entryList();
    entrySet = QSet<QString>(entryList.cbegin(), entryList.cend());
    QCOMPARE((int)entrySet.count(), 1);

    newIdSet = entrySet.subtract(entrySet2);
    QCOMPARE((int)newIdSet.count(), 1);

    newId = *newIdSet.cbegin();
    QCOMPARE(item.remoteId(), newId);
    entrySet2 << newId;
    QCOMPARE((int)entrySet2.count(), 1);

    Item item2;
    item2.setId(QRandomGenerator::global()->generate());
    item2.setMimeType(KMime::Message::mimeType());
    item2.setPayload<KMime::Message::Ptr>(msgPtr2);

    job = mStore->createItem(item2, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.parentCollection(), collection2);

    entryList = md2.entryList();
    entrySet = QSet<QString>(entryList.cbegin(), entryList.cend());
    QCOMPARE((int)entrySet.count(), 2);

    newIdSet = entrySet.subtract(entrySet2);
    QCOMPARE((int)newIdSet.count(), 1);

    newId = *newIdSet.cbegin();
    QCOMPARE(item.remoteId(), newId);
    entrySet2 << newId;
    QCOMPARE((int)entrySet2.count(), 2);

    // test adding to non-empty maildir
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    job = mStore->createItem(item1, collection1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item1.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.parentCollection(), collection1);

    entryList = md1.entryList();
    entrySet = QSet<QString>(entryList.cbegin(), entryList.cend());
    QCOMPARE((int)entrySet.count(), 5);

    newIdSet = entrySet.subtract(entrySet1);
    QCOMPARE((int)newIdSet.count(), 1);

    newId = *newIdSet.cbegin();
    QCOMPARE(item.remoteId(), newId);
    entrySet1 << newId;
    QCOMPARE((int)entrySet1.count(), 5);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 5);
    for (const Item &item : std::as_const(items)) {
        const auto flags{item.flags()};
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    job = mStore->createItem(item2, collection1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());
    QVERIFY(!item.remoteId().isEmpty());
    QCOMPARE(item.parentCollection(), collection1);

    entryList = md1.entryList();
    entrySet = QSet<QString>(entryList.cbegin(), entryList.cend());
    QCOMPARE((int)entrySet.count(), 6);

    newIdSet = entrySet.subtract(entrySet1);
    QCOMPARE((int)newIdSet.count(), 1);

    newId = *newIdSet.cbegin();
    QCOMPARE(item.remoteId(), newId);
    entrySet1 << newId;
    QCOMPARE((int)entrySet1.count(), 6);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 6);
    for (const Item &item : std::as_const(items)) {
        const auto flags{item.flags()};
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();
}

QTEST_MAIN(ItemCreateTest)

#include "itemcreatetest.moc"
