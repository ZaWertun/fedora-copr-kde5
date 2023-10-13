/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/collectionmodifyjob.h"
#include "filestore/itemfetchjob.h"

#include "libmaildir/maildir.h"

#include <QTemporaryDir>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

class CollectionModifyTest : public QObject
{
    Q_OBJECT

public:
    CollectionModifyTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~CollectionModifyTest() override
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
    void testRename();
    void testIndexPreservation();
    void testIndexCacheUpdate();
};

void CollectionModifyTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void CollectionModifyTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void CollectionModifyTest::testRename()
{
    QDir topDir(mDir->path());
    QVERIFY(topDir.mkdir(QStringLiteral("topLevel")));
    QVERIFY(topDir.cd(QStringLiteral("topLevel")));

    KPIM::Maildir topLevelMd(topDir.path(), true);
    QVERIFY(topLevelMd.isValid(false));

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);
    KPIM::Maildir md1_2(md1.addSubFolder(QStringLiteral("collection1_2")), false);

    // simulate second level mbox in maildir parent
    QFileInfo fileInfo1_1(KPIM::Maildir::subDirPathForFolderPath(md1.path()), QStringLiteral("collection1_1"));
    QFile file1_1(fileInfo1_1.absoluteFilePath());
    file1_1.open(QIODevice::WriteOnly);
    file1_1.close();
    QVERIFY(fileInfo1_1.exists());

    KPIM::Maildir md2(topLevelMd.addSubFolder(QStringLiteral("collection2")), false);

    // simulate first level mbox
    QFileInfo fileInfo3(topDir.path(), QStringLiteral("collection3"));
    QFile file3(fileInfo3.absoluteFilePath());
    file3.open(QIODevice::WriteOnly);
    file3.close();
    QVERIFY(fileInfo3.exists());

    // simulate first level mbox with subtree
    QFileInfo fileInfo4(topDir.path(), QStringLiteral("collection4"));
    QFile file4(fileInfo4.absoluteFilePath());
    file4.open(QIODevice::WriteOnly);
    file4.close();
    QVERIFY(fileInfo4.exists());

    QFileInfo subDirInfo4(KPIM::Maildir::subDirPathForFolderPath(fileInfo4.absoluteFilePath()));
    QVERIFY(topDir.mkpath(subDirInfo4.absoluteFilePath()));

    KPIM::Maildir md4(subDirInfo4.absoluteFilePath(), true);
    KPIM::Maildir md4_1(md4.addSubFolder(QStringLiteral("collection4_1")), false);

    // simulate second level mbox in mbox parent
    QFileInfo fileInfo4_2(subDirInfo4.absoluteFilePath(), QStringLiteral("collection4_2"));
    QFile file4_2(fileInfo4_2.absoluteFilePath());
    file4_2.open(QIODevice::WriteOnly);
    file4_2.close();
    QVERIFY(fileInfo4_2.exists());

    mStore->setPath(topDir.path());

    FileStore::CollectionModifyJob *job = nullptr;
    Collection collection;

    // test renaming top level collection
    topDir.cdUp();
    QVERIFY(!topDir.exists(QStringLiteral("newTopLevel")));

    Collection topLevelCollection = mStore->topLevelCollection();
    topLevelCollection.setName(QStringLiteral("newTopLevel"));
    job = mStore->modifyCollection(topLevelCollection);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(topDir.exists(QStringLiteral("newTopLevel")));
    QVERIFY(!topDir.exists(QStringLiteral("topLevel")));
    QVERIFY(topDir.cd(QStringLiteral("newTopLevel")));
    QCOMPARE(mStore->path(), topDir.path());

    collection = job->collection();
    QCOMPARE(collection.remoteId(), mStore->path());
    QCOMPARE(collection, mStore->topLevelCollection());

    // test failure of renaming again
    job = mStore->modifyCollection(topLevelCollection);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(collection.remoteId(), mStore->path());
    QCOMPARE(collection, mStore->topLevelCollection());

    // adjust local handles
    topLevelMd = KPIM::Maildir(topDir.path(), true);
    QVERIFY(topLevelMd.isValid(false));

    md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QVERIFY(md1.isValid(false));
    md1_2 = md1.subFolder(QStringLiteral("collection1_2"));

    fileInfo1_1 = QFileInfo(KPIM::Maildir::subDirPathForFolderPath(md1.path()), QStringLiteral("collection1_1"));
    QVERIFY(fileInfo1_1.exists());

    md2 = topLevelMd.subFolder(QStringLiteral("collection2"));

    fileInfo3 = QFileInfo(topDir.path(), QStringLiteral("collection3"));
    QVERIFY(fileInfo3.exists());

    fileInfo4 = QFileInfo(topDir.path(), QStringLiteral("collection4"));
    QVERIFY(fileInfo4.exists());

    subDirInfo4 = QFileInfo(KPIM::Maildir::subDirPathForFolderPath(fileInfo4.absoluteFilePath()));
    QVERIFY(subDirInfo4.exists());

    md4 = KPIM::Maildir(subDirInfo4.absoluteFilePath(), true);
    QVERIFY(md4.isValid(false));
    md4_1 = md4.subFolder(QStringLiteral("collection4_1"));

    fileInfo4_2 = QFileInfo(subDirInfo4.absoluteFilePath(), QStringLiteral("collection4_2"));
    QVERIFY(fileInfo4_2.exists());

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));

    // test rename first level maildir leaf
    Collection collection2;
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());
    collection2.setName(QStringLiteral("collection2_renamed"));

    job = mStore->modifyCollection(collection2);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection2);
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2_renamed"));
    QVERIFY(!md2.isValid(false));
    md2 = topLevelMd.subFolder(collection.remoteId());
    QVERIFY(md2.isValid(false));

    // test failure of renaming again
    job = mStore->modifyCollection(collection2);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2_renamed"));
    QVERIFY(md2.isValid(false));

    // test renaming of first level mbox leaf
    Collection collection3;
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());
    collection3.setName(QStringLiteral("collection3_renamed"));

    job = mStore->modifyCollection(collection3);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection3);
    fileInfo3.refresh();
    QVERIFY(!fileInfo3.exists());
    fileInfo3 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo3.exists());

    // test failure of renaming again
    job = mStore->modifyCollection(collection3);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    fileInfo3.refresh();
    QVERIFY(fileInfo3.exists());

    // test renaming second level maildir in mbox parent
    Collection collection4;
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());
    collection4.setName(QStringLiteral("collection4"));

    Collection collection4_1;
    collection4_1.setRemoteId(QStringLiteral("collection4_1"));
    collection4_1.setParentCollection(collection4);
    collection4_1.setName(QStringLiteral("collection4_1_renamed"));

    job = mStore->modifyCollection(collection4_1);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection4_1);
    QCOMPARE(md4.subFolderList(), QStringList() << QStringLiteral("collection4_1_renamed"));
    QVERIFY(!md4_1.isValid(false));
    md4_1 = md4.subFolder(collection.remoteId());
    QVERIFY(md4_1.isValid(false));

    // test failure of renaming again
    job = mStore->modifyCollection(collection4_1);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(md4.subFolderList(), QStringList() << QStringLiteral("collection4_1_renamed"));
    QVERIFY(md4_1.isValid(false));

    // test renaming of second level mbox in mbox parent
    Collection collection4_2;
    collection4_2.setRemoteId(QStringLiteral("collection4_2"));
    collection4_2.setParentCollection(collection4);
    collection4_2.setName(QStringLiteral("collection4_2_renamed"));

    job = mStore->modifyCollection(collection4_2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection4_2);
    fileInfo4_2.refresh();
    QVERIFY(!fileInfo4_2.exists());
    fileInfo4_2 = QFileInfo(md4.path(), collection.remoteId());
    QVERIFY(fileInfo4_2.exists());

    // test failure of renaming again
    job = mStore->modifyCollection(collection4_2);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    fileInfo4_2.refresh();
    QVERIFY(fileInfo4_2.exists());

    // test renaming of maildir with subtree
    Collection collection1;
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());
    collection1.setName(QStringLiteral("collection1_renamed"));

    job = mStore->modifyCollection(collection1);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection1);
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1_renamed") << QStringLiteral("collection2_renamed"));
    QVERIFY(!md1.isValid(false));
    md1 = topLevelMd.subFolder(collection.remoteId());
    QVERIFY(md1.isValid(false));
    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());
    QVERIFY(!md1_2.isValid(false));
    fileInfo1_1 = QFileInfo(KPIM::Maildir::subDirPathForFolderPath(md1.path()), QStringLiteral("collection1_1"));
    QVERIFY(fileInfo1_1.exists());
    md1_2 = md1.subFolder(QStringLiteral("collection1_2"));
    QVERIFY(md1_2.isValid(false));

    // test failure of renaming again
    job = mStore->modifyCollection(collection1);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1_renamed") << QStringLiteral("collection2_renamed"));
    QVERIFY(md2.isValid(false));
    QVERIFY(fileInfo1_1.exists());
    QVERIFY(md1_2.isValid(false));

    // test renaming of mbox with subtree
    collection4.setName(QStringLiteral("collection4_renamed"));
    job = mStore->modifyCollection(collection4);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();
    QCOMPARE(collection.remoteId(), collection.name());
    QCOMPARE(collection, collection4);
    fileInfo4.refresh();
    QVERIFY(!fileInfo4.exists());
    fileInfo4 = QFileInfo(topDir.path(), collection.remoteId());
    QVERIFY(fileInfo4.exists());
    md4 = KPIM::Maildir(KPIM::Maildir::subDirPathForFolderPath(fileInfo4.absoluteFilePath()), true);
    QVERIFY(md4.isValid(false));

    QVERIFY(!md4_1.isValid(false));
    fileInfo4_2.refresh();
    QVERIFY(!fileInfo4_2.exists());
    md4_1 = md4.subFolder(QStringLiteral("collection4_1_renamed"));
    QVERIFY(md4_1.isValid(false));
    fileInfo4_2 = QFileInfo(md4.path(), QStringLiteral("collection4_2_renamed"));
    QVERIFY(fileInfo4_2.exists());

    // test failure of renaming again
    job = mStore->modifyCollection(collection4);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    fileInfo4.refresh();
    QVERIFY(fileInfo4.exists());
}

void CollectionModifyTest::testIndexPreservation()
{
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), mDir->path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), mDir->path(), QStringLiteral("collection2")));

    mStore->setPath(mDir->path());

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    FileStore::CollectionModifyJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    QVariant var;
    Collection::List collections;
    Item::List items;

    QMap<QByteArray, int> flagCounts;

    // test renaming mbox
    Collection collection1;
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());
    collection1.setName(QStringLiteral("collection1_renamed"));

    job = mStore->modifyCollection(collection1);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    const QFileInfo indexFileInfo1(mDir->path(), QStringLiteral(".collection1_renamed.index"));
    QVERIFY(!indexFileInfo1.exists());

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collections.first());
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

    // test renaming maildir
    Collection collection2;
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());
    collection2.setName(QStringLiteral("collection2_renamed"));

    job = mStore->modifyCollection(collection2);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection2);

    const QFileInfo indexFileInfo2(mDir->path(), QStringLiteral(".collection2_renamed.index"));
    QVERIFY(!indexFileInfo2.exists());

    // get the items and check the flags (see data/README)
    itemFetch = mStore->fetchItems(collections.first());
    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    items = itemFetch->items();
    QCOMPARE((int)items.count(), 4);

    flagCounts.clear();
    for (const Item &item : std::as_const(items)) {
        const auto flags = item.flags();
        for (const QByteArray &flag : flags) {
            ++flagCounts[flag];
        }
    }

    QCOMPARE(flagCounts["\\SEEN"], 2);
    QCOMPARE(flagCounts["\\FLAGGED"], 1);
    QCOMPARE(flagCounts["$TODO"], 1);
}

void CollectionModifyTest::testIndexCacheUpdate()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid(false));

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);

    // simulate first level mbox
    QFileInfo fileInfo2(mDir->path(), QStringLiteral("collection2"));
    QFile file2(fileInfo2.absoluteFilePath());
    file2.open(QIODevice::WriteOnly);
    file2.close();
    QVERIFY(fileInfo2.exists());

    const QString colSubDir1 = KPIM::Maildir::subDirPathForFolderPath(md1.path());
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), colSubDir1, QStringLiteral("collection1_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), colSubDir1, QStringLiteral("collection1_2")));

    const QString colSubDir2 = KPIM::Maildir::subDirPathForFolderPath(fileInfo2.absoluteFilePath());
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("mbox"), colSubDir2, QStringLiteral("collection2_1")));
    QVERIFY(TestDataUtil::installFolder(QStringLiteral("maildir"), colSubDir2, QStringLiteral("collection2_2")));

    mStore->setPath(mDir->path());

    FileStore::CollectionModifyJob *job = nullptr;
    FileStore::ItemFetchJob *itemFetch = nullptr;
    Collection collection;
    Item::List items;
    QMap<QByteArray, int> flagCounts;

    // preparation: load all second level items to make sure respective index data is cached
    Collection collection1;
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());
    collection1.setName(QStringLiteral("collection1"));

    Collection collection1_1;
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);
    collection1_1.setName(QStringLiteral("collection1_1"));

    itemFetch = mStore->fetchItems(collection1_1);
    QVERIFY(itemFetch->exec());

    Collection collection1_2;
    collection1_2.setRemoteId(QStringLiteral("collection1_2"));
    collection1_2.setParentCollection(collection1);
    collection1_2.setName(QStringLiteral("collection1_2"));

    itemFetch = mStore->fetchItems(collection1_2);
    QVERIFY(itemFetch->exec());
    Collection collection2;
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());
    collection2.setName(QStringLiteral("collection2"));

    Collection collection2_1;
    collection2_1.setRemoteId(QStringLiteral("collection2_1"));
    collection2_1.setParentCollection(collection2);
    collection2_1.setName(QStringLiteral("collection2_1"));

    itemFetch = mStore->fetchItems(collection2_1);
    QVERIFY(itemFetch->exec());
    Collection collection2_2;
    collection2_2.setRemoteId(QStringLiteral("collection2_2"));
    collection2_2.setParentCollection(collection2);
    collection2_2.setName(QStringLiteral("collection2_2"));

    itemFetch = mStore->fetchItems(collection2_2);
    QVERIFY(itemFetch->exec());

    // test renaming the maildir parent
    collection1.setName(QStringLiteral("collection1_renamed"));

    job = mStore->modifyCollection(collection1);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();

    // get the items of the children and check the flags (see data/README)
    collection1_1.setParentCollection(collection);
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

    collection1_2.setParentCollection(collection);
    itemFetch = mStore->fetchItems(collection1_2);
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

    // test renaming the mbox parent
    collection2.setName(QStringLiteral("collection2_renamed"));

    job = mStore->modifyCollection(collection2);
    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection = job->collection();

    // get the items of the children and check the flags (see data/README)
    collection2_1.setParentCollection(collection);
    itemFetch = mStore->fetchItems(collection2_1);
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

    collection2_2.setParentCollection(collection);
    itemFetch = mStore->fetchItems(collection2_2);
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

QTEST_MAIN(CollectionModifyTest)

#include "collectionmodifytest.moc"
