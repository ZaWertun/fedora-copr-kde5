/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "filestore/collectionfetchjob.h"

#include "libmaildir/maildir.h"

#include <KMime/Message>

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

class CollectionFetchTest : public QObject
{
    Q_OBJECT

public:
    CollectionFetchTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
        // for monitoring signals
        qRegisterMetaType<Akonadi::Collection::List>();
    }

    ~CollectionFetchTest() override
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
    void testEmptyDir();
    void testMixedTree();
};

void CollectionFetchTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void CollectionFetchTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void CollectionFetchTest::testEmptyDir()
{
    mStore->setPath(mDir->path());

    FileStore::CollectionFetchJob *job = nullptr;
    QSignalSpy *spy = nullptr;
    Collection::List collections;

    // test base fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::Base);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QCOMPARE(spy->count(), 1);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 1);
    QCOMPARE(collections.first(), mStore->topLevelCollection());
    QCOMPARE(job->collections(), collections);

    // test first level fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::FirstLevel);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QCOMPARE(spy->count(), 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 0);
    QCOMPARE(job->collections(), collections);

    // test recursive fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::Recursive);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QCOMPARE(spy->count(), 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 0);
    QCOMPARE(job->collections(), collections);

    // test fail of base fetching non existent collection
    Collection collection;
    collection.setName(QStringLiteral("collection"));
    collection.setRemoteId(QStringLiteral("collection"));
    collection.setParentCollection(mStore->topLevelCollection());

    job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::Base);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(spy->count(), 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 0);
    QCOMPARE(job->collections(), collections);

    // test fail of first level fetching non existent collection
    job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::FirstLevel);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(spy->count(), 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 0);
    QCOMPARE(job->collections(), collections);

    // test fail of recursive fetching non existent collection
    job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::FirstLevel);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QCOMPARE(spy->count(), 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 0);
    QCOMPARE(job->collections(), collections);
}

void CollectionFetchTest::testMixedTree()
{
    QDir topDir(mDir->path());

    KPIM::Maildir topLevelMd(mDir->path(), true);
    QVERIFY(topLevelMd.isValid());

    KPIM::Maildir md1(topLevelMd.addSubFolder(QStringLiteral("collection1")), false);
    KPIM::Maildir md1_2(md1.addSubFolder(QStringLiteral("collection1_2")), false);
    KPIM::Maildir md1_2_1(md1_2.addSubFolder(QStringLiteral("collection1_2_1")), false);

    // simulate second level mbox in maildir parent
    QFileInfo fileInfo1_1(KPIM::Maildir::subDirPathForFolderPath(md1.path()), QStringLiteral("collection1_1"));
    QFile file1_1(fileInfo1_1.absoluteFilePath());
    file1_1.open(QIODevice::WriteOnly);
    file1_1.close();
    QVERIFY(fileInfo1_1.exists());

    QFileInfo subDirInfo1_1(KPIM::Maildir::subDirPathForFolderPath(fileInfo1_1.absoluteFilePath()));
    QVERIFY(topDir.mkpath(subDirInfo1_1.absoluteFilePath()));
    KPIM::Maildir md1_1(subDirInfo1_1.absoluteFilePath(), true);
    KPIM::Maildir md1_1_1(md1_1.addSubFolder(QStringLiteral("collection1_1_1")), false);

    // simulate third level mbox in mbox parent
    QFileInfo fileInfo1_1_2(md1_1.path(), QStringLiteral("collection1_1_2"));
    QFile file1_1_2(fileInfo1_1_2.absoluteFilePath());
    file1_1_2.open(QIODevice::WriteOnly);
    file1_1_2.close();
    QVERIFY(fileInfo1_1_2.exists());

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

    QSet<QString> firstLevelNames;
    firstLevelNames << md1.name() << md2.name() << fileInfo3.fileName() << fileInfo4.fileName();

    QSet<QString> secondLevelNames;
    secondLevelNames << md1_2.name() << md4_1.name() << fileInfo1_1.fileName() << fileInfo4_2.fileName();

    QSet<QString> thirdLevelNames;
    thirdLevelNames << md1_1_1.name() << fileInfo1_1_2.fileName() << md1_2_1.name();

    mStore->setPath(mDir->path());
    // mDir = 0;

    FileStore::CollectionFetchJob *job = nullptr;
    QSignalSpy *spy = nullptr;
    Collection::List collections;

    // test base fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::Base);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QCOMPARE(spy->count(), 1);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), 1);
    QCOMPARE(collections.first(), mStore->topLevelCollection());
    QCOMPARE(job->collections(), collections);

    // test first level fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::FirstLevel);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QVERIFY(spy->count() > 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), firstLevelNames.count());
    QCOMPARE(job->collections(), collections);

    for (const Collection &collection : std::as_const(collections)) {
        QVERIFY(!collection.remoteId().isEmpty());
        QCOMPARE(collection.remoteId(), collection.name());
        QCOMPARE(collection.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

        QCOMPARE(collection.rights(),
                 Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                     | Collection::CanChangeCollection | Collection::CanDeleteCollection);

        QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());
        QVERIFY(firstLevelNames.contains(collection.name()));
    }

    // test recursive fetch of top level collection
    job = mStore->fetchCollections(mStore->topLevelCollection(), FileStore::CollectionFetchJob::Recursive);

    spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);
    QVERIFY(spy->count() > 0);

    collections = collectionsFromSpy(spy);
    QCOMPARE(collections.count(), firstLevelNames.count() + secondLevelNames.count() + thirdLevelNames.count());
    QCOMPARE(job->collections(), collections);

    for (const Collection &collection : std::as_const(collections)) {
        QVERIFY(!collection.remoteId().isEmpty());
        QCOMPARE(collection.remoteId(), collection.name());
        QCOMPARE(collection.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

        QCOMPARE(collection.rights(),
                 Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                     | Collection::CanChangeCollection | Collection::CanDeleteCollection);

        if (firstLevelNames.contains(collection.name())) {
            QCOMPARE(collection.parentCollection(), mStore->topLevelCollection());
        } else if (secondLevelNames.contains(collection.name())) {
            QVERIFY(firstLevelNames.contains(collection.parentCollection().name()));
            QCOMPARE(collection.parentCollection().parentCollection(), mStore->topLevelCollection());
        } else if (thirdLevelNames.contains(collection.name())) {
            QVERIFY(secondLevelNames.contains(collection.parentCollection().name()));
            QCOMPARE(collection.parentCollection().parentCollection().parentCollection(), mStore->topLevelCollection());
        }
    }

    // test base fetching all collections
    for (const Collection &collection : std::as_const(collections)) {
        job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::Base);

        spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

        QVERIFY(job->exec());
        QCOMPARE(job->error(), 0);
        QCOMPARE(spy->count(), 1);

        const Collection::List list = collectionsFromSpy(spy);
        QCOMPARE(list.count(), 1);
        QCOMPARE(list.first(), collection);
        QCOMPARE(job->collections(), list);

        const Collection col = list.first();
        QVERIFY(!col.remoteId().isEmpty());
        QCOMPARE(col.remoteId(), col.name());
        QCOMPARE(col.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

        QCOMPARE(col.rights(),
                 Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                     | Collection::CanChangeCollection | Collection::CanDeleteCollection);
    }

    // test first level fetching all collections
    for (const Collection &collection : std::as_const(collections)) {
        job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::FirstLevel);

        spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

        QVERIFY(job->exec());
        QCOMPARE(job->error(), 0);

        const Collection::List list = collectionsFromSpy(spy);
        QCOMPARE(job->collections(), list);

        for (const Collection &childCollection : list) {
            QCOMPARE(childCollection.parentCollection(), collection);

            QVERIFY(!childCollection.remoteId().isEmpty());
            QCOMPARE(childCollection.remoteId(), childCollection.name());
            QCOMPARE(childCollection.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

            QCOMPARE(childCollection.rights(),
                     Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                         | Collection::CanChangeCollection | Collection::CanDeleteCollection);
        }

        if (firstLevelNames.contains(collection.name())) {
            for (const Collection &childCollection : list) {
                QVERIFY(secondLevelNames.contains(childCollection.name()));
            }
        } else if (secondLevelNames.contains(collection.name())) {
            for (const Collection &childCollection : list) {
                QVERIFY(thirdLevelNames.contains(childCollection.name()));
            }
            if (collection.name() == md1_2.name()) {
                QCOMPARE(list.count(), 1);
                QCOMPARE(list.first().name(), md1_2_1.name());
            } else if (collection.name() == fileInfo1_1.fileName()) {
                QCOMPARE(list.count(), 2);
            }
        } else {
            QCOMPARE(list.count(), 0);
        }
    }

    // test recursive fetching all collections
    for (const Collection &collection : std::as_const(collections)) {
        job = mStore->fetchCollections(collection, FileStore::CollectionFetchJob::Recursive);

        spy = new QSignalSpy(job, &FileStore::CollectionFetchJob::collectionsReceived);

        QVERIFY(job->exec());
        QCOMPARE(job->error(), 0);

        const Collection::List list = collectionsFromSpy(spy);
        QCOMPARE(job->collections(), list);

        for (const Collection &childCollection : list) {
            QVERIFY(childCollection.parentCollection() == collection || childCollection.parentCollection().parentCollection() == collection);
            QVERIFY(!childCollection.remoteId().isEmpty());
            QCOMPARE(childCollection.remoteId(), childCollection.name());
            QCOMPARE(childCollection.contentMimeTypes(), QStringList() << Collection::mimeType() << KMime::Message::mimeType());

            QCOMPARE(childCollection.rights(),
                     Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem | Collection::CanCreateCollection
                         | Collection::CanChangeCollection | Collection::CanDeleteCollection);
        }

        if (firstLevelNames.contains(collection.name())) {
            for (const Collection &childCollection : list) {
                QVERIFY(secondLevelNames.contains(childCollection.name()) || thirdLevelNames.contains(childCollection.name()));
            }
        } else if (secondLevelNames.contains(collection.name())) {
            for (const Collection &childCollection : list) {
                QVERIFY(thirdLevelNames.contains(childCollection.name()));
            }
            if (collection.name() == md1_2.name()) {
                QCOMPARE(list.count(), 1);
                QCOMPARE(list.first().name(), md1_2_1.name());
            } else if (collection.name() == fileInfo1_1.fileName()) {
                QCOMPARE(list.count(), 2);
            }
        } else {
            QCOMPARE(list.count(), 0);
        }
    }
}

QTEST_MAIN(CollectionFetchTest)

#include "collectionfetchtest.moc"
