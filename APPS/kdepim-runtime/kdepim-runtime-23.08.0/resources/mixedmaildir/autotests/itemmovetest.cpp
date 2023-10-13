/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/entitycompactchangeattribute.h"
#include "filestore/itemfetchjob.h"
#include "filestore/itemmovejob.h"
#include "filestore/storecompactjob.h"

#include "libmaildir/maildir.h"

#include <KMbox/MBox>

#include <KRandom>

#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QTemporaryDir>
#include <QTest>
using namespace Akonadi;
using namespace KMBox;

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

class ItemMoveTest : public QObject
{
    Q_OBJECT

public:
    ItemMoveTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~ItemMoveTest() override
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
    void testMaildirItem();
    void testMBoxItem();
};

void ItemMoveTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void ItemMoveTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void ItemMoveTest::testExpectedFail()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    const MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    QSet<qint64> entrySet2;
    for (const MBoxEntry &entry : entryList2) {
        entrySet2 << entry.messageOffset();
    }

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemMoveJob *job = nullptr;

    // test failure of moving from a non-existent collection
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    Item item3;
    item3.setRemoteId(QStringLiteral("item3"));
    item3.setParentCollection(collection3);

    job = mStore->moveItem(item3, collection1);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // test failure of moving from maildir to non-existent collection
    Item item1;
    item1.setId(QRandomGenerator::global()->generate());
    item1.setRemoteId(*entrySet1.cbegin());
    item1.setParentCollection(collection1);

    job = mStore->moveItem(item1, collection3);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // test failure of moving from mbox to non-existent collection
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Item item2;
    item2.setId(QRandomGenerator::global()->generate());
    item2.setRemoteId(QStringLiteral("0"));
    item2.setParentCollection(collection2);

    job = mStore->moveItem(item1, collection3);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List tmpEntryList = mbox2.entries();
    QVERIFY(std::equal(tmpEntryList.begin(), tmpEntryList.end(), entryList2.begin(), fullEntryCompare));

    // test failure of moving from maildir to top level collection
    job = mStore->moveItem(item1, mStore->topLevelCollection());
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // test failure of moving from mbox to top level collection
    job = mStore->moveItem(item1, mStore->topLevelCollection());
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    tmpEntryList = mbox2.entries();
    QVERIFY(std::equal(tmpEntryList.begin(), tmpEntryList.end(), entryList2.begin(), fullEntryCompare));

    // test failure of moving a non-existent maildir entry
    QString remoteId1;
    do {
        remoteId1 = KRandom::randomString(20);
    } while (entrySet1.contains(remoteId1));

    item1.setRemoteId(remoteId1);

    job = mStore->moveItem(item1, collection2);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    tmpEntryList = mbox2.entries();
    QVERIFY(std::equal(tmpEntryList.begin(), tmpEntryList.end(), entryList2.begin(), fullEntryCompare));

    // test failure of moving a non-existent mbox entry
    quint64 remoteId2;
    do {
        remoteId2 = QRandomGenerator::global()->generate();
    } while (entrySet2.contains(remoteId2));

    item2.setRemoteId(QString::number(remoteId2));

    job = mStore->moveItem(item2, collection1);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    tmpEntryList = mbox2.entries();
    QVERIFY(std::equal(tmpEntryList.begin(), tmpEntryList.end(), entryList2.begin(), fullEntryCompare));
}

void ItemMoveTest::testMaildirItem()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection2")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection5")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QStringList md1EntryList = md1.entryList();
    QSet<QString> entrySet1(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE((int)entrySet1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    KPIM::Maildir md3(topLevelMd.addSubFolder(QStringLiteral("collection3")), false);
    QVERIFY(md3.isValid());
    QStringList md3EntryList = md3.entryList();
    QSet<QString> entrySet3(md3EntryList.cbegin(), md3EntryList.cend());
    QCOMPARE((int)entrySet3.count(), 0);

    QFileInfo fileInfo4(topDir.path(), QStringLiteral("collection4"));
    QFile file4(fileInfo4.absoluteFilePath());
    QVERIFY(file4.open(QIODevice::WriteOnly));
    file4.close();
    fileInfo4.refresh();
    QVERIFY(fileInfo4.exists());
    MBox mbox4;
    QVERIFY(mbox4.load(fileInfo4.absoluteFilePath()));
    MBoxEntry::List entryList4 = mbox4.entries();
    QCOMPARE((int)entryList4.count(), 0);

    KPIM::Maildir md5 = topLevelMd.subFolder(QStringLiteral("collection5"));
    QStringList md5EntryList = md5.entryList();
    QSet<QString> entrySet5(md5EntryList.cbegin(), md5EntryList.cend());
    QCOMPARE((int)entrySet5.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemMoveJob *job = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item movedItem;
    MBoxEntry::List entryList;

    // test moving to an empty maildir
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setId(QRandomGenerator::global()->generate());
    item1.setRemoteId(*entrySet1.cbegin());
    item1.setParentCollection(collection1);

    job = mStore->moveItem(item1, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection3);

    entrySet3 << movedItem.remoteId();
    md3EntryList = md3.entryList();
    QCOMPARE(QSet<QString>(md3EntryList.cbegin(), md3EntryList.cend()), entrySet3);
    entrySet1.remove(item1.remoteId());
    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // test moving to a non empty maildir
    item1.setRemoteId(*entrySet1.cbegin());

    Collection collection5;
    collection5.setName(QStringLiteral("collection5"));
    collection5.setRemoteId(QStringLiteral("collection5"));
    collection5.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveItem(item1, collection5);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection5);

    entrySet5 << movedItem.remoteId();
    md5EntryList = md5.entryList();
    QCOMPARE(QSet<QString>(md5EntryList.cbegin(), md5EntryList.cend()), entrySet5);
    entrySet1.remove(item1.remoteId());
    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 2);
    QCOMPARE(collections, Collection::List() << collection1 << collection5);

    // test moving to an empty mbox
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    item1.setRemoteId(*entrySet1.cbegin());

    job = mStore->moveItem(item1, collection4);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection4);

    QVERIFY(mbox4.load(mbox4.fileName()));
    entryList = mbox4.entries();
    QCOMPARE((int)entryList.count(), 1);

    QCOMPARE(entryList.last().messageOffset(), movedItem.remoteId().toULongLong());
    entrySet1.remove(item1.remoteId());
    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // test moving to a non empty mbox
    item1.setRemoteId(*entrySet1.cbegin());

    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveItem(item1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection2);

    QVERIFY(mbox2.load(mbox2.fileName()));
    entryList = mbox2.entries();
    QCOMPARE((int)entryList.count(), 5);

    QCOMPARE(entryList.last().messageOffset(), movedItem.remoteId().toULongLong());
    entrySet1.remove(item1.remoteId());
    md1EntryList = md1.entryList();
    QCOMPARE(QSet<QString>(md1EntryList.cbegin(), md1EntryList.cend()), entrySet1);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 2);
    QCOMPARE(collections, Collection::List() << collection1 << collection2);
}

void ItemMoveTest::testMBoxItem()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection2")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection5")));

    QFileInfo fileInfo1(topDir.path(), QStringLiteral("collection1"));
    MBox mbox1;
    QVERIFY(mbox1.load(fileInfo1.absoluteFilePath()));
    MBoxEntry::List entryList1 = mbox1.entries();
    QCOMPARE((int)entryList1.count(), 4);

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md2 = topLevelMd.subFolder(QStringLiteral("collection2"));
    QStringList md2EntryList = md2.entryList();
    QSet<QString> entrySet2(md2EntryList.cbegin(), md2EntryList.cend());
    QCOMPARE((int)entrySet2.count(), 4);

    KPIM::Maildir md3(topLevelMd.addSubFolder(QStringLiteral("collection3")), false);
    QVERIFY(md3.isValid());
    QStringList md3EntryList = md3.entryList();
    QSet<QString> entrySet3(md3EntryList.cbegin(), md3EntryList.cend());
    QCOMPARE((int)entrySet3.count(), 0);

    QFileInfo fileInfo4(topDir.path(), QStringLiteral("collection4"));
    QFile file4(fileInfo4.absoluteFilePath());
    QVERIFY(file4.open(QIODevice::WriteOnly));
    file4.close();
    fileInfo4.refresh();
    QVERIFY(fileInfo4.exists());
    MBox mbox4;
    QVERIFY(mbox4.load(fileInfo4.absoluteFilePath()));
    MBoxEntry::List entryList4 = mbox4.entries();
    QCOMPARE((int)entryList4.count(), 0);

    QFileInfo fileInfo5(topDir.path(), QStringLiteral("collection5"));
    MBox mbox5;
    QVERIFY(mbox5.load(fileInfo5.absoluteFilePath()));
    MBoxEntry::List entryList5 = mbox5.entries();
    QCOMPARE((int)entryList5.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemMoveJob *job = nullptr;
    FileStore::StoreCompactJob *compactStore = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item movedItem;
    MBoxEntry::List entryList;
    Item::List items;

    // test moving to an empty maildir
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    Item item1;
    item1.setId(QRandomGenerator::global()->generate());
    item1.setRemoteId(QString::number(entryList1.first().messageOffset()));
    item1.setParentCollection(collection1);

    job = mStore->moveItem(item1, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection3);

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    compactStore = mStore->compactStore();
    QVERIFY(compactStore->exec());

    items = compactStore->changedItems();
    QCOMPARE((int)items.count(), 3);

    entrySet3 << movedItem.remoteId();
    md3EntryList = md3.entryList();
    QCOMPARE(QSet<QString>(md3EntryList.cbegin(), md3EntryList.cend()), entrySet3);

    entryList1.removeAt(0);
    entryList1[0] = MBoxEntry(changedOffset(items[0]));
    entryList1[1] = MBoxEntry(changedOffset(items[1]));
    entryList1[2] = MBoxEntry(changedOffset(items[2]));
    QVERIFY(mbox1.load(mbox1.fileName()));
    QCOMPARE(mbox1.entries(), entryList1);

    // test moving to a non empty mbox
    Collection collection5;
    collection5.setName(QStringLiteral("collection5"));
    collection5.setRemoteId(QStringLiteral("collection5"));
    collection5.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveItem(item1, collection5);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection5);

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 2);
    QCOMPARE(collections, Collection::List() << collection1 << collection5);

    compactStore = mStore->compactStore();
    QVERIFY(compactStore->exec());

    items = compactStore->changedItems();
    QCOMPARE((int)items.count(), 2);

    QVERIFY(mbox5.load(mbox5.fileName()));
    QCOMPARE(mbox5.entries().count(), entryList5.count() + 1);
    QCOMPARE(mbox5.entries().last().messageOffset(), movedItem.remoteId().toULongLong());

    entryList1.removeAt(0);
    entryList1[0] = MBoxEntry(changedOffset(items[0]));
    entryList1[1] = MBoxEntry(changedOffset(items[1]));
    QVERIFY(mbox1.load(mbox1.fileName()));
    QCOMPARE(mbox1.entries(), entryList1);

    // test moving to an empty mbox
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveItem(item1, collection4);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection4);

    QVERIFY(mbox4.load(mbox4.fileName()));
    entryList = mbox4.entries();
    QCOMPARE((int)entryList.count(), 1);

    QCOMPARE(entryList.last().messageOffset(), movedItem.remoteId().toULongLong());

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    compactStore = mStore->compactStore();
    QVERIFY(compactStore->exec());

    items = compactStore->changedItems();
    QCOMPARE((int)items.count(), 1);

    QCOMPARE(mbox4.entries().count(), entryList4.count() + 1);
    QCOMPARE(mbox4.entries().last().messageOffset(), movedItem.remoteId().toULongLong());

    entryList1.removeAt(0);
    entryList1[0] = MBoxEntry(changedOffset(items[0]));
    QVERIFY(mbox1.load(mbox1.fileName()));
    QCOMPARE(mbox1.entries(), entryList1);

    // test moving to a non empty maildir
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveItem(item1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    movedItem = job->item();
    QCOMPARE(movedItem.id(), item1.id());
    QCOMPARE(movedItem.parentCollection(), collection2);

    md2EntryList = md2.entryList();
    QSet<QString> entrySet(md2EntryList.cbegin(), md2EntryList.cend());
    QCOMPARE((int)entrySet.count(), 5);

    QVERIFY(entrySet.contains(movedItem.remoteId()));

    var = job->property("compactStore");
    QVERIFY(var.isValid());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCOMPARE(var.type(), QVariant::Bool);
#else
    QCOMPARE(var.userType(), QMetaType::Bool);
#endif
    QCOMPARE(var.toBool(), true);

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 2);
    QCOMPARE(collections.first(), collection1);

    compactStore = mStore->compactStore();
    QVERIFY(compactStore->exec());

    items = compactStore->changedItems();
    QCOMPARE((int)items.count(), 0);

    entryList1.removeAt(0);
    QVERIFY(mbox1.load(mbox1.fileName()));
    const MBoxEntry::List newEntryList = mbox1.entries();
    QVERIFY(std::equal(newEntryList.begin(), newEntryList.end(), entryList1.begin(), fullEntryCompare));
}

QTEST_MAIN(ItemMoveTest)

#include "itemmovetest.moc"
