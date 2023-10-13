/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/entitycompactchangeattribute.h"
#include "filestore/itemdeletejob.h"
#include "filestore/itemfetchjob.h"
#include "filestore/storecompactjob.h"

#include "libmaildir/maildir.h"

#include <KMbox/MBox>

#include <KMime/Message>

#include <QRandomGenerator>
#include <QTemporaryDir>

#include <QSignalSpy>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

static Collection::List collectionsFromSpy(QSignalSpy *spy)
{
    Collection::List collections;

    QListIterator<QList<QVariant>> it(*spy);
    while (it.hasNext()) {
        const QList<QVariant> invocation = it.next();
        Q_ASSERT(invocation.count() == 1);

        collections << invocation.first().value<Collection::List>();
    }

    return collections;
}

static Item::List itemsFromSpy(QSignalSpy *spy)
{
    Item::List items;

    QListIterator<QList<QVariant>> it(*spy);
    while (it.hasNext()) {
        const QList<QVariant> invocation = it.next();
        Q_ASSERT(invocation.count() == 1);

        items << invocation.first().value<Item::List>();
    }

    return items;
}

static bool fullEntryCompare(const KMBox::MBoxEntry &a, const KMBox::MBoxEntry &b)
{
    return a.messageOffset() == b.messageOffset() && a.separatorSize() == b.separatorSize() && a.messageSize() == b.messageSize();
}

class ItemDeleteTest : public QObject
{
    Q_OBJECT

public:
    ItemDeleteTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
        // for monitoring signals
        qRegisterMetaType<Akonadi::Collection::List>();
        qRegisterMetaType<Akonadi::Item::List>();
    }

    ~ItemDeleteTest() override
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
    void testMaildir();
    void testMBox();
    void testCachePreservation();
    void testExpectedFailure();
};

void ItemDeleteTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void ItemDeleteTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void ItemDeleteTest::testMaildir()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));

    KPIM::Maildir topLevelMd(topDir.path(), true);
    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QVERIFY(md1.isValid());

    QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemDeleteJob *job = nullptr;
    QSet<QString> entrySet;
    QSet<QString> delIdSet;
    QString delId;

    // test deleting one message
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setId(QRandomGenerator::global()->generate());
    item1.setRemoteId(*entrySet1.cbegin());
    item1.setParentCollection(collection1);

    job = mStore->deleteItem(item1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();
    QCOMPARE(item.id(), item1.id());

    md1EntryList = md1.entryList();
    entrySet = QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet.count(), 3);

    delIdSet = entrySet1.subtract(entrySet);
    QCOMPARE((int)delIdSet.count(), 1);

    delId = *delIdSet.cbegin();
    QCOMPARE(delId, *entrySet1.cbegin());
    QCOMPARE(delId, item.remoteId());

    // test failure of deleting again
    job = mStore->deleteItem(item1);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
}

void ItemDeleteTest::testMBox()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection1")));

    QFileInfo fileInfo1(topDir.path(), QStringLiteral("collection1"));
    KMBox::MBox mbox1;
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    KMBox::MBoxEntry::List entryList1 = mbox1.entries();
    QCOMPARE((int)entryList1.count(), 4);
    int size1 = fileInfo1.size();

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemDeleteJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    FileStore::StoreCompactJob *storeCompact = nullptr;

    Item::List items;
    Collection::List collections;
    KMBox::MBoxEntry::List entryList;

    QSignalSpy *collectionsSpy = nullptr;
    QSignalSpy *itemsSpy = nullptr;

    QVariant var;

    // test deleting last item in mbox
    // file stays untouched, message still accessible through MBox, but item gone
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Item item4;
    item4.setMimeType(KMime::Message::mimeType());
    item4.setId(QRandomGenerator::global()->generate());
    item4.setRemoteId(QString::number(entryList1.value(3).messageOffset()));
    item4.setParentCollection(collection1);

    job = mStore->deleteItem(item4);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();
    QCOMPARE(item.id(), item4.id());

    fileInfo1.refresh();
    QCOMPARE((int)fileInfo1.size(), size1);
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    entryList = mbox1.entries();
    QCOMPARE(entryList.count(), entryList1.count());
    QCOMPARE(entryList.value(3).messageOffset(), entryList1.value(3).messageOffset());

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    itemFetch = mStore->fetchItems(collection1);

    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 3);
    QCOMPARE(items.value(0).remoteId(), QString::number(entryList1.value(0).messageOffset()));
    QCOMPARE(items.value(1).remoteId(), QString::number(entryList1.value(1).messageOffset()));
    QCOMPARE(items.value(2).remoteId(), QString::number(entryList1.value(2).messageOffset()));

    // test that the item is purged from the file on store compaction
    // last item purging does not change any others
    storeCompact = mStore->compactStore();

    collectionsSpy = new QSignalSpy(storeCompact, &FileStore::StoreCompactJob::collectionsChanged);
    itemsSpy = new QSignalSpy(storeCompact, &FileStore::StoreCompactJob::itemsChanged);

    QVERIFY(storeCompact->exec());
    QCOMPARE(storeCompact->error(), 0);

    collections = storeCompact->changedCollections();
    QCOMPARE(collections.count(), 0);
    items = storeCompact->changedItems();
    QCOMPARE(items.count(), 0);

    QCOMPARE(collectionsFromSpy(collectionsSpy), collections);
    QCOMPARE(itemsFromSpy(itemsSpy), items);

    fileInfo1.refresh();
    QVERIFY(fileInfo1.size() < size1);
    size1 = fileInfo1.size();
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    entryList = mbox1.entries();
    entryList1.pop_back();
    QVERIFY(std::equal(entryList1.begin(), entryList1.end(), entryList.begin(), fullEntryCompare));

    // test deleting item somewhere between first and last
    // again, file stays untouched, message still accessible through MBox, but item gone
    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setId(QRandomGenerator::global()->generate());
    item2.setRemoteId(QString::number(entryList1.value(1).messageOffset()));
    item2.setParentCollection(collection1);

    job = mStore->deleteItem(item2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());

    fileInfo1.refresh();
    QCOMPARE((int)fileInfo1.size(), size1);
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    entryList = mbox1.entries();
    QCOMPARE(entryList.count(), entryList1.count());
    QCOMPARE(entryList.value(1).messageOffset(), entryList1.value(1).messageOffset());

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    itemFetch = mStore->fetchItems(collection1);

    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 2);
    QCOMPARE(items.value(0).remoteId(), QString::number(entryList1.value(0).messageOffset()));
    QCOMPARE(items.value(1).remoteId(), QString::number(entryList1.value(2).messageOffset()));

    // test that the item is purged from the file on store compaction
    // non-last item purging changes all items after it
    storeCompact = mStore->compactStore();

    collectionsSpy = new QSignalSpy(storeCompact, &FileStore::StoreCompactJob::collectionsChanged);
    itemsSpy = new QSignalSpy(storeCompact, &FileStore::StoreCompactJob::itemsChanged);

    QVERIFY(storeCompact->exec());
    QCOMPARE(storeCompact->error(), 0);

    collections = storeCompact->changedCollections();
    QCOMPARE(collections.count(), 1);
    items = storeCompact->changedItems();
    QCOMPARE(items.count(), 1);

    QCOMPARE(collectionsFromSpy(collectionsSpy), collections);
    QCOMPARE(itemsFromSpy(itemsSpy), items);

    Item item3;
    item3.setRemoteId(QString::number(entryList1.value(2).messageOffset()));

    item = items.first();
    QCOMPARE(item3.remoteId(), item.remoteId());

    QVERIFY(item.hasAttribute<FileStore::EntityCompactChangeAttribute>());
    auto attribute = item.attribute<FileStore::EntityCompactChangeAttribute>();

    QString newRemoteId = attribute->remoteId();
    QVERIFY(!newRemoteId.isEmpty());

    fileInfo1.refresh();
    QVERIFY(fileInfo1.size() < size1);
    size1 = fileInfo1.size();
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    entryList = mbox1.entries();
    QCOMPARE(QString::number(entryList.value(1).messageOffset()), newRemoteId);

    entryList1.removeAt(1);
    QCOMPARE(entryList1.count(), entryList.count());
    QCOMPARE(QString::number(entryList1.value(1).messageOffset()), item3.remoteId());
}

void ItemDeleteTest::testCachePreservation()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);
    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QVERIFY(md1.isValid());

    const QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    KMBox::MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    KMBox::MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    FileStore::ItemDeleteJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;

    // test deleting from maildir
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setId(QRandomGenerator::global()->generate());
    item1.setRemoteId(*entrySet1.cbegin());
    item1.setParentCollection(collection1);

    job = mStore->deleteItem(item1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();
    QCOMPARE(item.id(), item1.id());

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
    QCOMPARE((int)items.count(), 3);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    // TODO since we don't know which message we've deleted, we can only check if some flags are present
    int flagCountTotal = 0;
    for (int count : std::as_const(flagCounts)) {
        flagCountTotal += count;
    }
    QVERIFY(flagCountTotal > 0);
    flagCounts.clear();

    // test deleting from mbox
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setId(QRandomGenerator::global()->generate());
    item2.setRemoteId(QString::number(entryList2.value(1).messageOffset()));
    item2.setParentCollection(collection2);

    job = mStore->deleteItem(item2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();
    QCOMPARE(item.id(), item2.id());

    // at this point no change has been written to disk yet, so index and mbox file are
    // still in sync
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(!var.isValid());

    FileStore::StoreCompactJob *storeCompact = mStore->compactStore();

    QVERIFY(storeCompact->exec());
    QCOMPARE(storeCompact->error(), 0);

    // check for index preservation
    var = storeCompact->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection2);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection2);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 3);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    // we've deleted message 2, it flagged TODO and seen
    QCOMPARE(flagCounts["\\SEEN"], 1);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    flagCounts.clear();
}

void ItemDeleteTest::testExpectedFailure()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);
    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QVERIFY(md1.isValid());

    const QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    KMBox::MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    KMBox::MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemDeleteJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    FileStore::StoreCompactJob *storeCompact = nullptr;

    // test failure of fetching an item previously deleted from maildir
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Item item1_1;
    item1_1.setRemoteId(*entrySet1.cbegin());
    item1_1.setParentCollection(collection1);

    job = mStore->deleteItem(item1_1);

    QVERIFY(job->exec());

    itemFetch = mStore->fetchItem(item1_1);

    QVERIFY(!itemFetch->exec());
    QCOMPARE(itemFetch->error(), (int)FileStore::Job::InvalidJobContext);

    // test failure of deleting an item from maildir again
    job = mStore->deleteItem(item1_1);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    // test failure of fetching an item previously deleted from mbox
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item2_1;
    item2_1.setRemoteId(QString::number(entryList2.value(0).messageOffset()));
    item2_1.setParentCollection(collection2);

    job = mStore->deleteItem(item2_1);

    QVERIFY(job->exec());

    itemFetch = mStore->fetchItem(item2_1);

    QVERIFY(!itemFetch->exec());
    QCOMPARE(itemFetch->error(), (int)FileStore::Job::InvalidJobContext);

    // test failure of deleting an item from mbox again
    job = mStore->deleteItem(item2_1);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    // compact store and check that offset 0 is a valid remoteId again, but
    // offset of other items (e.f. item 4) are no longer valid (moved to the front of the file)
    storeCompact = mStore->compactStore();

    QVERIFY(storeCompact->exec());

    itemFetch = mStore->fetchItem(item2_1);

    QVERIFY(itemFetch->exec());

    Item item4_1;
    item4_1.setRemoteId(QString::number(entryList2.value(3).messageOffset()));
    item4_1.setParentCollection(collection2);

    itemFetch = mStore->fetchItem(item4_1);

    QVERIFY(!itemFetch->exec());
    QCOMPARE(itemFetch->error(), (int)FileStore::Job::InvalidJobContext);
}

QTEST_MAIN(ItemDeleteTest)

#include "itemdeletetest.moc"
