/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/collectionmovejob.h"
#include "filestore/itemfetchjob.h"

#include "libmaildir/maildir.h"

#include <QTemporaryDir>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

class CollectionMoveTest : public QObject
{
    Q_OBJECT

public:
    CollectionMoveTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~CollectionMoveTest() override
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
    void testMoveToTopLevel();
    void testMoveToMaildir();
    void testMoveToMBox();
};

void CollectionMoveTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void CollectionMoveTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void CollectionMoveTest::testMoveToTopLevel()
{
    QDir topDir(mDir->path());

    // top level dir
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    // QFileInfo fileInfo1(topDir, QStringLiteral("collection1"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection2")));
    // QFileInfo fileInfo2(topDir, QStringLiteral("collection2"));

    // first level maildir parent
    QDir subDir1 = topDir;
    QVERIFY(subDir1.mkdir(QStringLiteral(".collection1.directory")));
    QVERIFY(subDir1.cd(QStringLiteral(".collection1.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir1.path(), QStringLiteral("collection1_1")));
    QFileInfo fileInfo1_1(subDir1.path(), QStringLiteral("collection1_1"));
    QVERIFY(fileInfo1_1.exists());
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir1.path(), QStringLiteral("collection1_2")));
    QFileInfo fileInfo1_2(subDir1.path(), QStringLiteral("collection1_2"));
    QVERIFY(fileInfo1_2.exists());

    // first level mbox parent
    QDir subDir2 = topDir;
    QVERIFY(subDir2.mkdir(QStringLiteral(".collection2.directory")));
    QVERIFY(subDir2.cd(QStringLiteral(".collection2.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir2.path(), QStringLiteral("collection2_1")));
    QFileInfo fileInfo2_1(subDir2.path(), QStringLiteral("collection2_1"));
    QVERIFY(fileInfo2_1.exists());
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir2.path(), QStringLiteral("collection2_2")));
    QFileInfo fileInfo2_2(subDir2.path(), QStringLiteral("collection2_2"));
    QVERIFY(fileInfo2_2.exists());

    mStore->setPath(topDir.path());

    // common variables
    FileStore::CollectionMoveJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    Collection collection;
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    // test moving maildir from maildir parent
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);

    job = mStore->moveCollection(collection1_1, mStore->topLevelCollection());

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // test moving mbox from maildir parent
    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    collection1_2.setRemoteId(QStringLiteral("collection1_2"));
    collection1_2.setParentCollection(collection1);

    job = mStore->moveCollection(collection1_2, mStore->topLevelCollection());

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());

    fileInfo1_2.refresh();
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // test moving mbox from mbox parent
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Collection collection2_1;
    collection2_1.setName(QStringLiteral("collection2_1"));
    collection2_1.setRemoteId(QStringLiteral("collection2_1"));
    collection2_1.setParentCollection(collection2);

    job = mStore->moveCollection(collection2_1, mStore->topLevelCollection());

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection2_1.remoteId());
    QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());

    fileInfo2_1.refresh();
    QVERIFY(!fileInfo2_1.exists());
    fileInfo2_1 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo2_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // test moving maildir from mbox parent
    Collection collection2_2;
    collection2_2.setName(QStringLiteral("collection2_2"));
    collection2_2.setRemoteId(QStringLiteral("collection2_2"));
    collection2_2.setParentCollection(collection2);

    job = mStore->moveCollection(collection2_2, mStore->topLevelCollection());

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection2_2.remoteId());
    QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());

    fileInfo2_2.refresh();
    QVERIFY(!fileInfo2_2.exists());
    fileInfo2_2 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo2_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();
}

void CollectionMoveTest::testMoveToMaildir()
{
    QDir topDir(mDir->path());

    // top level dir
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QFileInfo fileInfo1(topDir, QStringLiteral("collection1"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection2")));
    QFileInfo fileInfo2(topDir, QStringLiteral("collection2"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection3")));
    QFileInfo fileInfo3(topDir, QStringLiteral("collection3"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection4")));
    QFileInfo fileInfo4(topDir, QStringLiteral("collection4"));

    // first level maildir parent
    QDir subDir1 = topDir;
    QVERIFY(subDir1.mkdir(QStringLiteral(".collection1.directory")));
    QVERIFY(subDir1.cd(QStringLiteral(".collection1.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir1.path(), QStringLiteral("collection1_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir1.path(), QStringLiteral("collection1_2")));

    // first level mbox parent
    QDir subDir4 = topDir;
    QVERIFY(subDir4.mkdir(QStringLiteral(".collection4.directory")));
    QVERIFY(subDir4.cd(QStringLiteral(".collection4.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir4.path(), QStringLiteral("collection4_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir4.path(), QStringLiteral("collection4_2")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir4.path(), QStringLiteral("collection4_3")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir4.path(), QStringLiteral("collection4_4")));

    // target maildir
    KPIM::Maildir topLevelMd(topDir.path(), true);
    KPIM::Maildir targetMd(topLevelMd.addSubFolder(QStringLiteral("target")), false);
    QVERIFY(targetMd.isValid());
    QDir subDirTarget;

    mStore->setPath(topDir.path());

    // common variables
    FileStore::CollectionMoveJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    Collection collection;
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    Collection target;
    target.setName(QStringLiteral("target"));
    target.setRemoteId(QStringLiteral("target"));
    target.setParentCollection(mStore->topLevelCollection());

    // test move leaf maildir into sibling
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveCollection(collection2, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    subDirTarget = topDir;
    QVERIFY(subDirTarget.cd(QStringLiteral(".target.directory")));

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection2.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo2.refresh();
    QVERIFY(!fileInfo2.exists());
    fileInfo2 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move leaf mbox into sibling
    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveCollection(collection3, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection3.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo3.refresh();
    QVERIFY(!fileInfo3.exists());
    fileInfo3 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo3.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // test move maildir with subtree into sibling
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    // load sub collection index data to check for correct cache updates
    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);
    itemFetch = mStore->fetchItems(collection1_1);
    QVERIFY(itemFetch->exec());

    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    collection1_2.setRemoteId(QStringLiteral("collection1_2"));
    collection1_2.setParentCollection(collection1);
    itemFetch = mStore->fetchItems(collection1_2);
    QVERIFY(itemFetch->exec());

    job = mStore->moveCollection(collection1, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1.refresh();
    QVERIFY(!fileInfo1.exists());
    fileInfo1 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1.exists());
    QVERIFY(!subDir1.exists());
    subDir1 = subDirTarget;
    QVERIFY(subDir1.cd(QStringLiteral(".collection1.directory")));
    QCOMPARE(subDir1.entryList(QStringList() << QStringLiteral("collection*")),
             QStringList() << QStringLiteral("collection1_1") << QStringLiteral("collection1_2"));

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // check for children cache path updates
    collection1.setParentCollection(target);
    collection1_1.setParentCollection(collection1);
    collection1_2.setParentCollection(collection1);

    itemFetch = mStore->fetchItems(collection1_1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    itemFetch = mStore->fetchItems(collection1_2);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move mbox with subtree into sibling
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    // load sub collection index data to check for correct cache updates
    Collection collection4_1;
    collection4_1.setName(QStringLiteral("collection4_1"));
    collection4_1.setRemoteId(QStringLiteral("collection4_1"));
    collection4_1.setParentCollection(collection4);
    itemFetch = mStore->fetchItems(collection4_1);
    QVERIFY(itemFetch->exec());

    Collection collection4_2;
    collection4_2.setName(QStringLiteral("collection4_2"));
    collection4_2.setRemoteId(QStringLiteral("collection4_2"));
    collection4_2.setParentCollection(collection4);
    itemFetch = mStore->fetchItems(collection4_2);
    QVERIFY(itemFetch->exec());

    job = mStore->moveCollection(collection4, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo4.refresh();
    QVERIFY(!fileInfo4.exists());
    fileInfo4 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo4.exists());
    QVERIFY(!subDir4.exists());
    subDir4 = subDirTarget;
    QVERIFY(subDir4.cd(QStringLiteral(".collection4.directory")));
    QCOMPARE(subDir4.entryList(QStringList() << QStringLiteral("collection*")),
             QStringList() << QStringLiteral("collection4_1") << QStringLiteral("collection4_2") << QStringLiteral("collection4_3")
                           << QStringLiteral("collection4_4"));

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // check for children cache path updates
    collection4.setParentCollection(target);
    collection4_1.setParentCollection(collection4);
    collection4_2.setParentCollection(collection4);

    itemFetch = mStore->fetchItems(collection4_1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    itemFetch = mStore->fetchItems(collection4_2);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // move from parent maildir to parent's sibling
    collection2.setParentCollection(target);

    job = mStore->moveCollection(collection1_1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QDir subDir2 = subDirTarget;
    QVERIFY(subDir2.cd(QStringLiteral(".collection2.directory")));

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    QFileInfo fileInfo1_1(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // move from parent maildir to parent's sibling
    job = mStore->moveCollection(collection1_2, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    QFileInfo fileInfo1_2(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // move from parent mbox to parent's sibling
    job = mStore->moveCollection(collection4_1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    QFileInfo fileInfo4_1(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_1.exists());
    fileInfo4_1 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo4_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // move from parent mbox to parent's sibling
    job = mStore->moveCollection(collection4_2, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    QFileInfo fileInfo4_2(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_2.exists());
    fileInfo4_2 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo4_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
    flagCounts.clear();

    // move from parent maildir to grandparent
    collection1_1.setParentCollection(collection2);

    job = mStore->moveCollection(collection1_1, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent maildir to grandparent
    collection1_2.setParentCollection(collection2);
    job = mStore->moveCollection(collection1_2, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1_2.refresh();
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to grandparent
    Collection collection4_3;
    collection4_3.setName(QStringLiteral("collection4_3"));
    collection4_3.setRemoteId(QStringLiteral("collection4_3"));
    collection4_3.setParentCollection(collection4);

    job = mStore->moveCollection(collection4_3, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_3.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    QFileInfo fileInfo4_3(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_3.exists());
    fileInfo4_3 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo4_3.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to grandparent
    Collection collection4_4;
    collection4_4.setName(QStringLiteral("collection4_4"));
    collection4_4.setRemoteId(QStringLiteral("collection4_4"));
    collection4_4.setParentCollection(collection4);

    job = mStore->moveCollection(collection4_4, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_4.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    QFileInfo fileInfo4_4(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_4.exists());
    fileInfo4_4 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo4_4.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from maildir to grandchild
    collection1_1.setParentCollection(target);

    job = mStore->moveCollection(collection1_1, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from maildir to grandchild
    collection1_2.setParentCollection(target);

    job = mStore->moveCollection(collection1_2, collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection2);

    fileInfo1_2.refresh();
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDir2, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

void CollectionMoveTest::testMoveToMBox()
{
    QDir topDir(mDir->path());

    // top level dir
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection1")));
    QFileInfo fileInfo1(topDir, QStringLiteral("collection1"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), topDir.path(), QStringLiteral("collection2")));
    QFileInfo fileInfo2(topDir, QStringLiteral("collection2"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection3")));
    QFileInfo fileInfo3(topDir, QStringLiteral("collection3"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection4")));
    QFileInfo fileInfo4(topDir, QStringLiteral("collection4"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection5")));
    QFileInfo fileInfo5(topDir, QStringLiteral("collection5"));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), topDir.path(), QStringLiteral("collection6")));

    // first level maildir parent
    QDir subDir1 = topDir;
    QVERIFY(subDir1.mkdir(QStringLiteral(".collection1.directory")));
    QVERIFY(subDir1.cd(QStringLiteral(".collection1.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir1.path(), QStringLiteral("collection1_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir1.path(), QStringLiteral("collection1_2")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir1.path(), QStringLiteral("collection1_3")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir1.path(), QStringLiteral("collection1_4")));

    // first level mbox parent
    QDir subDir4 = topDir;
    QVERIFY(subDir4.mkdir(QStringLiteral(".collection4.directory")));
    QVERIFY(subDir4.cd(QStringLiteral(".collection4.directory")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), subDir4.path(), QStringLiteral("collection4_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), subDir4.path(), QStringLiteral("collection4_2")));

    // target mbox
    QFileInfo fileInfoTarget(topDir.path(), QStringLiteral("target"));
    QFile fileTarget(fileInfoTarget.absoluteFilePath());
    QVERIFY(fileTarget.open(QIODevice::WriteOnly));
    fileTarget.close();
    QVERIFY(fileInfoTarget.exists());

    QDir subDirTarget;

    mStore->setPath(topDir.path());

    // common variables
    FileStore::CollectionMoveJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    Collection collection;
    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    Collection target;
    target.setName(QStringLiteral("target"));
    target.setRemoteId(QStringLiteral("target"));
    target.setParentCollection(mStore->topLevelCollection());

    // test move leaf maildir into sibling
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveCollection(collection2, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    subDirTarget = topDir;
    QVERIFY(subDirTarget.cd(QStringLiteral(".target.directory")));

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection2.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo2.refresh();
    QVERIFY(!fileInfo2.exists());
    fileInfo2 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move leaf mbox into sibling
    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveCollection(collection3, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection3.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo3.refresh();
    QVERIFY(!fileInfo3.exists());
    fileInfo3 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo3.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move leaf mbox into sibling without subtree
    Collection collection5;
    collection5.setName(QStringLiteral("collection5"));
    collection5.setRemoteId(QStringLiteral("collection5"));
    collection5.setParentCollection(mStore->topLevelCollection());

    Collection collection6;
    collection6.setName(QStringLiteral("collection6"));
    collection6.setRemoteId(QStringLiteral("collection6"));
    collection6.setParentCollection(mStore->topLevelCollection());

    job = mStore->moveCollection(collection5, collection6);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection5.remoteId());
    QCOMPARE(collection.parentCollection(), collection6);

    fileInfo5.refresh();
    QVERIFY(!fileInfo5.exists());
    QDir subDir6 = topDir;
    QVERIFY(subDir6.cd(QStringLiteral(".collection6.directory")));
    fileInfo5 = QFileInfo(subDir6, collection.remoteId());
    QVERIFY(fileInfo5.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move maildir with subtree into sibling
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    // load sub collection index data to check for correct cache updates
    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);
    itemFetch = mStore->fetchItems(collection1_1);
    QVERIFY(itemFetch->exec());

    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    collection1_2.setRemoteId(QStringLiteral("collection1_2"));
    collection1_2.setParentCollection(collection1);
    itemFetch = mStore->fetchItems(collection1_2);
    QVERIFY(itemFetch->exec());

    job = mStore->moveCollection(collection1, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1.refresh();
    QVERIFY(!fileInfo1.exists());
    fileInfo1 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1.exists());
    QVERIFY(!subDir1.exists());
    subDir1 = subDirTarget;
    QVERIFY(subDir1.cd(QStringLiteral(".collection1.directory")));
    QCOMPARE(subDir1.entryList(QStringList() << QStringLiteral("collection*")),
             QStringList() << QStringLiteral("collection1_1") << QStringLiteral("collection1_2") << QStringLiteral("collection1_3")
                           << QStringLiteral("collection1_4"));

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // check for children cache path updates
    collection1.setParentCollection(target);
    collection1_1.setParentCollection(collection1);
    collection1_2.setParentCollection(collection1);

    itemFetch = mStore->fetchItems(collection1_1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    itemFetch = mStore->fetchItems(collection1_2);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // test move mbox with subtree into sibling
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    // load sub collection index data to check for correct cache updates
    Collection collection4_1;
    collection4_1.setName(QStringLiteral("collection4_1"));
    collection4_1.setRemoteId(QStringLiteral("collection4_1"));
    collection4_1.setParentCollection(collection4);
    itemFetch = mStore->fetchItems(collection4_1);
    QVERIFY(itemFetch->exec());

    Collection collection4_2;
    collection4_2.setName(QStringLiteral("collection4_2"));
    collection4_2.setRemoteId(QStringLiteral("collection4_2"));
    collection4_2.setParentCollection(collection4);
    itemFetch = mStore->fetchItems(collection4_2);
    QVERIFY(itemFetch->exec());

    job = mStore->moveCollection(collection4, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo4.refresh();
    QVERIFY(!fileInfo4.exists());
    fileInfo4 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo4.exists());
    QVERIFY(!subDir4.exists());
    subDir4 = subDirTarget;
    QVERIFY(subDir4.cd(QStringLiteral(".collection4.directory")));
    QCOMPARE(subDir4.entryList(QStringList() << QStringLiteral("collection*")),
             QStringList() << QStringLiteral("collection4_1") << QStringLiteral("collection4_2"));

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // check for children cache path updates
    collection4.setParentCollection(target);
    collection4_1.setParentCollection(collection4);
    collection4_2.setParentCollection(collection4);

    itemFetch = mStore->fetchItems(collection4_1);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    itemFetch = mStore->fetchItems(collection4_2);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent maildir to parent's sibling
    collection3.setParentCollection(target);

    job = mStore->moveCollection(collection1_1, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QDir subDir3 = subDirTarget;
    QVERIFY(subDir3.cd(QStringLiteral(".collection3.directory")));

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    QFileInfo fileInfo1_1(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent maildir to parent's sibling
    job = mStore->moveCollection(collection1_2, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    QFileInfo fileInfo1_2(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to parent's sibling
    job = mStore->moveCollection(collection4_1, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    QFileInfo fileInfo4_1(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_1.exists());
    fileInfo4_1 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo4_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to parent's sibling
    job = mStore->moveCollection(collection4_2, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection4_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    QFileInfo fileInfo4_2(subDir4, collection.remoteId());
    QVERIFY(!fileInfo4_2.exists());
    fileInfo4_2 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo4_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to grandparent
    collection1_1.setParentCollection(collection3);

    job = mStore->moveCollection(collection1_1, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent mbox to grandparent
    collection1_2.setParentCollection(collection3);
    job = mStore->moveCollection(collection1_2, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    fileInfo1_2.refresh();
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent maildir to grandparent
    Collection collection1_3;
    collection1_3.setName(QStringLiteral("collection1_3"));
    collection1_3.setRemoteId(QStringLiteral("collection1_3"));
    collection1_3.setParentCollection(collection1);

    job = mStore->moveCollection(collection1_3, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_3.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    QFileInfo fileInfo1_3(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_3.exists());
    fileInfo1_3 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_3.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from parent maildir to grandparent
    Collection collection1_4;
    collection1_4.setName(QStringLiteral("collection1_4"));
    collection1_4.setRemoteId(QStringLiteral("collection1_4"));
    collection1_4.setParentCollection(collection1);

    job = mStore->moveCollection(collection1_4, target);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_4.remoteId());
    QCOMPARE(collection.parentCollection(), target);

    QFileInfo fileInfo1_4(subDir1, collection.remoteId());
    QVERIFY(!fileInfo1_4.exists());
    fileInfo1_4 = QFileInfo(subDirTarget, collection.remoteId());
    QVERIFY(fileInfo1_4.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from mbox to grandchild
    collection1_1.setParentCollection(target);

    job = mStore->moveCollection(collection1_1, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_1.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    fileInfo1_1 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo1_1.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

    // move from maildir to grandchild
    collection1_2.setParentCollection(target);

    job = mStore->moveCollection(collection1_2, collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection1_2.remoteId());
    QCOMPARE(collection.parentCollection(), collection3);

    fileInfo1_2.refresh();
    QVERIFY(!fileInfo1_2.exists());
    fileInfo1_2 = QFileInfo(subDir3, collection.remoteId());
    QVERIFY(fileInfo1_2.exists());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection);

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collection);
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);
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

QTEST_MAIN(CollectionMoveTest)

#include "collectionmovetest.moc"
