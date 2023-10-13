/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/entitycompactchangeattribute.h"
#include "filestore/itemdeletejob.h"
#include "filestore/storecompactjob.h"

#include <KMbox/MBox>

#include <QTemporaryDir>

#include <QSignalSpy>

#include <QDir>
#include <QFileInfo>
#include <QTest>
using namespace Akonadi;
using namespace KMBox;

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

static bool fullEntryCompare(const MBoxEntry &a, const MBoxEntry &b)
{
    return a.messageOffset() == b.messageOffset() && a.separatorSize() == b.separatorSize() && a.messageSize() == b.messageSize();
}

static quint64 changedOffset(const Item &item)
{
    Q_ASSERT(item.hasAttribute<FileStore::EntityCompactChangeAttribute>());

    const QString remoteId = item.attribute<FileStore::EntityCompactChangeAttribute>()->remoteId();
    Q_ASSERT(!remoteId.isEmpty());

    bool ok = false;
    const quint64 result = remoteId.toULongLong(&ok);
    Q_ASSERT(ok);

    return result;
}

class StoreCompactTest : public QObject
{
    Q_OBJECT

public:
    StoreCompactTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
        // for monitoring signals
        qRegisterMetaType<Akonadi::Collection::List>();
        qRegisterMetaType<Akonadi::Item::List>();
    }

    ~StoreCompactTest() override
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
    void testCompact();
};

void StoreCompactTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void StoreCompactTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void StoreCompactTest::testCompact()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection3")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection4")));

    QFileInfo fileInfo1(topDir.path(), QStringLiteral("collection1"));
    MBox mbox1;
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    MBoxEntry::List entryList1 = mbox1.entries();
    QCOMPARE((int)entryList1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    QFileInfo fileInfo3(topDir.path(), QStringLiteral("collection3"));
    MBox mbox3;
    QVERIFY(mbox3.load(fileInfo3.absoluteFilePath()));
    MBoxEntry::List entryList3 = mbox3.entries();
    QCOMPARE((int)entryList3.count(), 4);

    QFileInfo fileInfo4(topDir.path(), QStringLiteral("collection4"));
    MBox mbox4;
    QVERIFY(mbox4.load(fileInfo4.absoluteFilePath()));
    MBoxEntry::List entryList4 = mbox4.entries();
    QCOMPARE((int)entryList4.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::CollectionFetchJob *collectionFetch = nullptr;
    FileStore::ItemDeleteJob *itemDelete = nullptr;
    FileStore::StoreCompactJob *job = nullptr;

    Collection::List collections;
    Item::List items;

    QSignalSpy *collectionSpy = nullptr;
    QSignalSpy *itemSpy = nullptr;

    MBoxEntry::List entryList;
    Collection collection;
    FileStore::EntityCompactChangeAttribute *attribute = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;

    // test compact after delete from the end of an mbox
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setRemoteId(QString::number(entryList1.last().messageOffset()));
    item1.setParentCollection(collection1);

    itemDelete = mStore->deleteItem(item1);

    QVERIFY(itemDelete->exec());

    job = mStore->compactStore();

    collectionSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::collectionsChanged);
    itemSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::itemsChanged);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collections = job->changedCollections();
    items = job->changedItems();

    QCOMPARE(collections.count(), 0);
    QCOMPARE(items.count(), 0);

    QCOMPARE(collectionSpy->count(), 0);
    QCOMPARE(itemSpy->count(), 0);

    QVERIFY(mbox1.load(mbox1.fileName()));
    entryList = mbox1.entries();
    entryList1.pop_back();
    QVERIFY(std::equal(entryList.begin(), entryList.end(), entryList1.begin(), fullEntryCompare));

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections, Collection::List() << collection1);

    // test compact after delete from before the end of an mbox
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item2;
    item2.setRemoteId(QString::number(entryList2.first().messageOffset()));
    item2.setParentCollection(collection2);

    itemDelete = mStore->deleteItem(item2);

    QVERIFY(itemDelete->exec());

    job = mStore->compactStore();

    collectionSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::collectionsChanged);
    itemSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::itemsChanged);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collections = job->changedCollections();
    items = job->changedItems();

    QCOMPARE(collections.count(), 1);
    QCOMPARE(items.count(), 3);

    QCOMPARE(collectionSpy->count(), 1);
    QCOMPARE(itemSpy->count(), 1);

    QCOMPARE(collectionsFromSpy(collectionSpy), collections);
    QCOMPARE(itemsFromSpy(itemSpy), items);

    collection = collections.first();
    QCOMPARE(collection, collection2);
    QVERIFY(collection.hasAttribute<FileStore::EntityCompactChangeAttribute>());
    attribute = collection.attribute<FileStore::EntityCompactChangeAttribute>();
    const QString remoteRevision = attribute->remoteRevision();
    QCOMPARE(remoteRevision.toInt(), collection2.remoteRevision().toInt() + 1);

    QVERIFY(mbox2.load(mbox2.fileName()));
    entryList = mbox2.entries();

    entryList2.pop_front();
    for (int i = 0; i < items.count(); ++i) {
        entryList2[i] = MBoxEntry(changedOffset(items[i]));
    }
    QCOMPARE(entryList, entryList2);

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections, Collection::List() << collection2);

    collectionFetch = mStore->fetchCollections(collection2, FileStore::CollectionFetchJob::Base);

    QVERIFY(collectionFetch->exec());

    collections = collectionFetch->collections();
    QCOMPARE((int)collections.count(), 1);

    collection = collections.first();
    QCOMPARE(collection, collection2);
    QCOMPARE(collection.remoteRevision(), remoteRevision);

    // test compact after delete from before the end of more than one mbox
    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    Item item3;
    item3.setRemoteId(QString::number(entryList3.first().messageOffset()));
    item3.setParentCollection(collection3);

    itemDelete = mStore->deleteItem(item3);

    QVERIFY(itemDelete->exec());

    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    Item item4;
    item4.setRemoteId(QString::number(entryList3.value(1).messageOffset()));
    item4.setParentCollection(collection4);

    itemDelete = mStore->deleteItem(item4);

    QVERIFY(itemDelete->exec());

    job = mStore->compactStore();

    collectionSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::collectionsChanged);
    itemSpy = new QSignalSpy(job, &FileStore::StoreCompactJob::itemsChanged);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collections = job->changedCollections();
    items = job->changedItems();

    QCOMPARE(collections.count(), 2);
    QCOMPARE(items.count(), 5);

    QCOMPARE(collectionSpy->count(), 2);
    QCOMPARE(itemSpy->count(), 2);

    QCOMPARE(collectionsFromSpy(collectionSpy), collections);
    QCOMPARE(itemsFromSpy(itemSpy), items);

    QHash<QString, Collection> compactedCollections;
    for (const Collection &col : std::as_const(collections)) {
        compactedCollections.insert(col.remoteId(), col);
    }
    QCOMPARE(compactedCollections.count(), 2);

    QVERIFY(compactedCollections.contains(collection3.remoteId()));
    collection = compactedCollections[collection3.remoteId()];
    QCOMPARE(collection, collection3);
    QVERIFY(collection.hasAttribute<FileStore::EntityCompactChangeAttribute>());
    attribute = collection.attribute<FileStore::EntityCompactChangeAttribute>();
    QCOMPARE(attribute->remoteRevision().toInt(), collection3.remoteRevision().toInt() + 1);

    QVERIFY(mbox3.load(mbox3.fileName()));
    entryList = mbox3.entries();

    // The order of items depends on the order of iteration of a QHash in MixedMaildirStore.
    // This makes sure that the items are always sorted by collection and offset
    std::sort(items.begin(), items.end(), [](const Akonadi::Item &left, const Akonadi::Item &right) {
        return left.parentCollection().remoteId().compare(right.parentCollection().remoteId()) < 0
            || (left.parentCollection().remoteId() == right.parentCollection().remoteId() && changedOffset(left) < changedOffset(right));
    });

    entryList3.pop_front();
    for (int i = 0; i < entryList3.count(); ++i) {
        entryList3[i] = MBoxEntry(changedOffset(items.first()));
        items.pop_front();
    }
    QCOMPARE(entryList, entryList3);

    QVERIFY(compactedCollections.contains(collection4.remoteId()));
    collection = compactedCollections[collection4.remoteId()];
    QCOMPARE(collection, collection4);
    QVERIFY(collection.hasAttribute<FileStore::EntityCompactChangeAttribute>());
    attribute = collection.attribute<FileStore::EntityCompactChangeAttribute>();
    QCOMPARE(attribute->remoteRevision().toInt(), collection4.remoteRevision().toInt() + 1);

    QVERIFY(mbox4.load(mbox4.fileName()));
    entryList = mbox4.entries();

    entryList4.removeAt(1);
    for (int i = 0; i < items.count(); ++i) {
        entryList4[i + 1] = MBoxEntry(changedOffset(items[i]));
    }
    QCOMPARE(entryList, entryList4);

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 2);
    QCOMPARE(collections, Collection::List() << collection3 << collection4);
}

QTEST_MAIN(StoreCompactTest)

#include "storecompacttest.moc"
