/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "filestore/collectioncreatejob.h"

#include "libmaildir/maildir.h"

#include <KMime/Message>

#include <QTemporaryDir>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

class CollectionCreateTest : public QObject
{
    Q_OBJECT

public:
    CollectionCreateTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~CollectionCreateTest() override
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
    void testCollectionProperties();
    void testEmptyDir();
    void testMaildirTree();
    void testMixedTree();
};

void CollectionCreateTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void CollectionCreateTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void CollectionCreateTest::testCollectionProperties()
{
    mStore->setPath(mDir->path());

    FileStore::CollectionCreateJob *job = nullptr;

    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    job = mStore->createCollection(collection1, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1 = job->collection();
    QCOMPARE(collection1.remoteId(), collection1.name());

    QCOMPARE(collection1.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

    QCOMPARE(collection1.rights(),
             Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                 | Collection::CanChangeCollection | Collection::CanDeleteCollection);
}

void CollectionCreateTest::testEmptyDir()
{
    mStore->setPath(mDir->path());

    KPIM::Maildir topLevelMd(mStore->path(), true);

    FileStore::CollectionCreateJob *job = nullptr;

    // test creating first level collections
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    job = mStore->createCollection(collection1, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1 = job->collection();
    QVERIFY(!collection1.remoteId().isEmpty());
    QVERIFY(collection1.parentCollection() == mStore->topLevelCollection());

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1"));
    KPIM::Maildir md1 = topLevelMd.subFolder(collection1.remoteId());
    QVERIFY(md1.isValid());

    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    job = mStore->createCollection(collection2, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection2 = job->collection();
    QVERIFY(!collection2.remoteId().isEmpty());
    QVERIFY(collection2.parentCollection() == mStore->topLevelCollection());

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));
    KPIM::Maildir md2 = topLevelMd.subFolder(collection2.remoteId());
    QVERIFY(md2.isValid());

    // test creating second level collections
    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    job = mStore->createCollection(collection1_1, collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1_1 = job->collection();
    QVERIFY(!collection1_1.remoteId().isEmpty());
    QVERIFY(collection1_1.parentCollection() == collection1);

    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_1"));
    KPIM::Maildir md1_1 = md1.subFolder(collection1_1.remoteId());
    QVERIFY(md1_1.isValid());

    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    job = mStore->createCollection(collection1_2, collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1_2 = job->collection();
    QVERIFY(!collection1_2.remoteId().isEmpty());
    QVERIFY(collection1_2.parentCollection() == collection1);

    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_1") << QStringLiteral("collection1_2"));
    KPIM::Maildir md1_2 = md1.subFolder(collection1_2.remoteId());
    QVERIFY(md1_2.isValid());

    QCOMPARE(md2.subFolderList(), QStringList());
}

void CollectionCreateTest::testMaildirTree()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid());

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);

    KPIM::Maildir md1_2(md1.addSubFolder(QStringLiteral("collection1_2")), false);

    mStore->setPath(mDir->path());

    FileStore::CollectionCreateJob *job = nullptr;

    // test creating first level collections
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    job = mStore->createCollection(collection1, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec()); // works because it already exists
    QCOMPARE(job->error(), 0);

    collection1 = job->collection();
    QVERIFY(!collection1.remoteId().isEmpty());
    QVERIFY(collection1.parentCollection() == mStore->topLevelCollection());

    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    job = mStore->createCollection(collection2, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection2 = job->collection();
    QVERIFY(!collection2.remoteId().isEmpty());
    QVERIFY(collection2.parentCollection() == mStore->topLevelCollection());

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));
    KPIM::Maildir md2 = topLevelMd.subFolder(collection2.remoteId());
    QVERIFY(md2.isValid());

    // test creating second level collections
    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    job = mStore->createCollection(collection1_1, collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1_1 = job->collection();
    QVERIFY(!collection1_1.remoteId().isEmpty());
    QCOMPARE(collection1_1.parentCollection().remoteId(), QStringLiteral("collection1"));

    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_1") << QStringLiteral("collection1_2"));
    KPIM::Maildir md1_1 = md1.subFolder(collection1_1.remoteId());
    QVERIFY(md1_1.isValid());

    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    job = mStore->createCollection(collection1_2, collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec()); // works because it already exists
    QCOMPARE(job->error(), 0);

    collection1_2 = job->collection();
    QVERIFY(!collection1_2.remoteId().isEmpty());
    QCOMPARE(collection1_2.parentCollection().remoteId(), QStringLiteral("collection1"));

    QCOMPARE(md2.subFolderList(), QStringList());
}

void CollectionCreateTest::testMixedTree()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid());

    // simulate a first level MBox
    QFileInfo fileInfo1(mDir->path(), QStringLiteral("collection1"));
    QFile file1(fileInfo1.absoluteFilePath());
    file1.open(QIODevice::WriteOnly);
    file1.close();
    QVERIFY(fileInfo1.exists());

    mStore->setPath(mDir->path());

    FileStore::CollectionCreateJob *job = nullptr;

    // test creating first level collections
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    job = mStore->createCollection(collection1, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec()); // fails, there is an MBox with that name
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    collection1 = job->collection();
    QVERIFY(collection1.remoteId().isEmpty());

    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    job = mStore->createCollection(collection2, mStore->topLevelCollection());
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection2 = job->collection();
    QVERIFY(!collection2.remoteId().isEmpty());
    QVERIFY(collection2.parentCollection() == mStore->topLevelCollection());

    // mbox does not show up as a maildir subfolder
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection2"));
    KPIM::Maildir md2 = topLevelMd.subFolder(collection2.remoteId());
    QVERIFY(md2.isValid());

    // test creating second level collections inside mbox
    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    job = mStore->createCollection(collection1_1, collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    collection1_1 = job->collection();
    QVERIFY(!collection1_1.remoteId().isEmpty());
    QCOMPARE(collection1_1.parentCollection().remoteId(), QStringLiteral("collection1"));

    // treat the MBox subdir path like a top level maildir
    KPIM::Maildir md1(KPIM::Maildir::subDirPathForFolderPath(fileInfo1.absoluteFilePath()), true);
    KPIM::Maildir md1_1 = md1.subFolder(collection1_1.remoteId());
    QVERIFY(md1_1.isValid());

    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_1"));
}

QTEST_MAIN(CollectionCreateTest)

#include "collectioncreatetest.moc"
