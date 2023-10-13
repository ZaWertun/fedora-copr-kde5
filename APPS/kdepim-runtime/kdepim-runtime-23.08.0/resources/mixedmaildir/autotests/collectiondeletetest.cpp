/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "filestore/collectiondeletejob.h"

#include "libmaildir/maildir.h"

#include <QTemporaryDir>

#include <QDir>
#include <QFileInfo>
#include <QTest>

using namespace Akonadi;

class CollectionDeleteTest : public QObject
{
    Q_OBJECT

public:
    CollectionDeleteTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~CollectionDeleteTest() override
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
    void testNonExisting();
    void testLeaves();
    void testSubTrees();
};

void CollectionDeleteTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void CollectionDeleteTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void CollectionDeleteTest::testNonExisting()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid(false));

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);
    KPIM::Maildir md1_2(md1.addSubFolder(QStringLiteral("collection1_2")), false);

    KPIM::Maildir md2(topLevelMd.addSubFolder(QStringLiteral("collection2")), false);

    // simulate mbox
    QFileInfo fileInfo1(mDir->path(), QStringLiteral("collection3"));
    QFile file1(fileInfo1.absoluteFilePath());
    file1.open(QIODevice::WriteOnly);
    file1.close();
    QVERIFY(fileInfo1.exists());

    // simulate mbox with empty subtree
    QFileInfo fileInfo2(mDir->path(), QStringLiteral("collection4"));
    QFile file2(fileInfo2.absoluteFilePath());
    file2.open(QIODevice::WriteOnly);
    file2.close();
    QVERIFY(fileInfo2.exists());

    QFileInfo subDirInfo2(KPIM::Maildir::subDirPathForFolderPath(fileInfo2.absoluteFilePath()));
    QDir topDir(mDir->path());
    QVERIFY(topDir.mkpath(subDirInfo2.absoluteFilePath()));

    mStore->setPath(mDir->path());

    FileStore::CollectionDeleteJob *job = nullptr;

    // test fail of deleting first level collection
    Collection collection5;
    collection5.setName(QStringLiteral("collection5"));
    collection5.setRemoteId(QStringLiteral("collection5"));
    collection5.setParentCollection(mStore->topLevelCollection());
    job = mStore->deleteCollection(collection5);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));
    QVERIFY(fileInfo1.exists());

    // test fail of deleting second level collection in maildir leaf parent
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    Collection collection2_1;
    collection2_1.setName(QStringLiteral("collection2_1"));
    collection2_1.setRemoteId(QStringLiteral("collection2_1"));
    collection2_1.setParentCollection(collection2);
    job = mStore->deleteCollection(collection2_1);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));

    // test fail of deleting second level collection in maildir parent with subtree
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);
    job = mStore->deleteCollection(collection1_1);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));
    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_2"));

    // test fail of deleting second level collection in mbox leaf parent
    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    Collection collection3_1;
    collection3_1.setName(QStringLiteral("collection3_1"));
    collection3_1.setRemoteId(QStringLiteral("collection3_1"));
    collection3_1.setParentCollection(collection3);
    job = mStore->deleteCollection(collection3_1);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QVERIFY(fileInfo1.exists());

    // test fail of deleting second level collection in mbox parent with subtree
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    Collection collection4_1;
    collection4_1.setName(QStringLiteral("collection4_1"));
    collection4_1.setRemoteId(QStringLiteral("collection4_1"));
    collection4_1.setParentCollection(collection4);
    job = mStore->deleteCollection(collection4_1);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QVERIFY(fileInfo2.exists());
    QVERIFY(subDirInfo2.exists());

    // test fail of deleting second level collection with non existent parent
    Collection collection5_1;
    collection5_1.setName(QStringLiteral("collection5_1"));
    collection5_1.setRemoteId(QStringLiteral("collection5_1"));
    collection5_1.setParentCollection(collection5);
    job = mStore->deleteCollection(collection5_1);
    QVERIFY(job != nullptr);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1") << QStringLiteral("collection2"));
    QVERIFY(fileInfo1.exists());
    QCOMPARE(md1.subFolderList(), QStringList() << QStringLiteral("collection1_2"));
}

void CollectionDeleteTest::testLeaves()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid(false));

    QDir topDir(mDir->path());

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
    QFileInfo fileInfo3(mDir->path(), QStringLiteral("collection3"));
    QFile file3(fileInfo3.absoluteFilePath());
    file3.open(QIODevice::WriteOnly);
    file3.close();
    QVERIFY(fileInfo3.exists());

    // simulate first level mbox with subtree
    QFileInfo fileInfo4(mDir->path(), QStringLiteral("collection4"));
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

    mStore->setPath(mDir->path());

    FileStore::CollectionDeleteJob *job = nullptr;

    // test second level leaves in maildir parent
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    Collection collection1_1;
    collection1_1.setName(QStringLiteral("collection1_1"));
    collection1_1.setRemoteId(QStringLiteral("collection1_1"));
    collection1_1.setParentCollection(collection1);
    job = mStore->deleteCollection(collection1_1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());

    Collection collection1_2;
    collection1_2.setName(QStringLiteral("collection1_2"));
    collection1_2.setRemoteId(QStringLiteral("collection1_2"));
    collection1_2.setParentCollection(collection1);
    job = mStore->deleteCollection(collection1_2);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(!md1_2.isValid(false));
    QCOMPARE(md1.subFolderList(), QStringList());

    // test second level leaves in mbox parent
    Collection collection4;
    collection4.setName(QStringLiteral("collection4"));
    collection4.setRemoteId(QStringLiteral("collection4"));
    collection4.setParentCollection(mStore->topLevelCollection());

    Collection collection4_1;
    collection4_1.setName(QStringLiteral("collection4_1"));
    collection4_1.setRemoteId(QStringLiteral("collection4_1"));
    collection4_1.setParentCollection(collection4);
    job = mStore->deleteCollection(collection4_1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(!md4_1.isValid(false));
    QCOMPARE(md4.subFolderList(), QStringList());

    Collection collection4_2;
    collection4_2.setName(QStringLiteral("collection4_2"));
    collection4_2.setRemoteId(QStringLiteral("collection4_2"));
    collection4_2.setParentCollection(collection4);
    job = mStore->deleteCollection(collection4_2);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    fileInfo4_2.refresh();
    QVERIFY(!fileInfo4_2.exists());

    // test deleting of first level leaves
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->deleteCollection(collection2);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(!md2.isValid(false));
    QCOMPARE(topLevelMd.subFolderList(), QStringList() << QStringLiteral("collection1"));

    Collection collection3;
    collection3.setName(QStringLiteral("collection3"));
    collection3.setRemoteId(QStringLiteral("collection3"));
    collection3.setParentCollection(mStore->topLevelCollection());

    job = mStore->deleteCollection(collection3);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    fileInfo3.refresh();
    QVERIFY(!fileInfo3.exists());

    // test deleting of first level leaves with empty subtrees
    QFileInfo subDirInfo1(KPIM::Maildir::subDirPathForFolderPath(md1.path()));
    QVERIFY(subDirInfo1.exists());

    job = mStore->deleteCollection(collection1);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(!md1.isValid(false));
    subDirInfo1.refresh();
    QVERIFY(!subDirInfo1.exists());
    QCOMPARE(topLevelMd.subFolderList(), QStringList());

    job = mStore->deleteCollection(collection4);
    QVERIFY(job != nullptr);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    fileInfo4.refresh();
    QVERIFY(!fileInfo4.exists());
    subDirInfo4.refresh();
    QVERIFY(!subDirInfo4.exists());
}

void CollectionDeleteTest::testSubTrees()
{
    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid(false));

    QDir topDir(mDir->path());

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);
    KPIM::Maildir md1_2(md1.addSubFolder(QStringLiteral("collection1_2")), false);

    // simulate second level mbox in maildir parent
    QFileInfo fileInfo1_1(KPIM::Maildir::subDirPathForFolderPath(md1.path()), QStringLiteral("collection1_1"));
    QFile file1_1(fileInfo1_1.absoluteFilePath());
    file1_1.open(QIODevice::WriteOnly);
    file1_1.close();
    QVERIFY(fileInfo1_1.exists());

    // simulate first level mbox with subtree
    QFileInfo fileInfo2(mDir->path(), QStringLiteral("collection2"));
    QFile file2(fileInfo2.absoluteFilePath());
    file2.open(QIODevice::WriteOnly);
    file2.close();
    QVERIFY(fileInfo2.exists());

    QFileInfo subDirInfo2(KPIM::Maildir::subDirPathForFolderPath(fileInfo2.absoluteFilePath()));
    QVERIFY(topDir.mkpath(subDirInfo2.absoluteFilePath()));

    KPIM::Maildir md2(subDirInfo2.absoluteFilePath(), true);
    KPIM::Maildir md2_1(md2.addSubFolder(QStringLiteral("collection2_1")), false);

    // simulate second level mbox in mbox parent
    QFileInfo fileInfo2_2(subDirInfo2.absoluteFilePath(), QStringLiteral("collection2_2"));
    QFile file2_2(fileInfo2_2.absoluteFilePath());
    file2_2.open(QIODevice::WriteOnly);
    file2_2.close();
    QVERIFY(fileInfo2_2.exists());

    mStore->setPath(mDir->path());

    FileStore::CollectionDeleteJob *job = nullptr;

    // test deleting maildir subtree
    QFileInfo subDirInfo1(KPIM::Maildir::subDirPathForFolderPath(md1.path()));
    QVERIFY(subDirInfo1.exists());

    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    job = mStore->deleteCollection(collection1);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(!md1.isValid(false));
    QVERIFY(!md1_2.isValid(false));
    fileInfo1_1.refresh();
    QVERIFY(!fileInfo1_1.exists());

    // test deleting mbox subtree
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    job = mStore->deleteCollection(collection2);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    fileInfo2.refresh();
    QVERIFY(!fileInfo2.exists());
    QVERIFY(!md2_1.isValid(false));
    fileInfo2_2.refresh();
    QVERIFY(!fileInfo2_2.exists());
    QVERIFY(!subDirInfo2.exists());
}

QTEST_MAIN(CollectionDeleteTest)

#include "collectiondeletetest.moc"
