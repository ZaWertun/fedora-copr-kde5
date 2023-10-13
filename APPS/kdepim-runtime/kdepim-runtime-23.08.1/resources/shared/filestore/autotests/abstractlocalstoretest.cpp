/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractlocalstore.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionmovejob.h"
#include "itemcreatejob.h"
#include "itemdeletejob.h"
#include "itemfetchjob.h"
#include "itemmodifyjob.h"
#include "itemmovejob.h"
#include "sessionimpls_p.h"
#include "storecompactjob.h"

#include <KRandom>
#include <QRandomGenerator>

#include <QTest>

using namespace Akonadi;
using namespace Akonadi::FileStore;

class TestStore : public AbstractLocalStore
{
    Q_OBJECT

public:
    TestStore()
        : mLastCheckedJob(nullptr)
        , mLastProcessedJob(nullptr)
        , mErrorCode(0)
    {
    }

public:
    mutable Akonadi::FileStore::Job *mLastCheckedJob = nullptr;
    Akonadi::FileStore::Job *mLastProcessedJob = nullptr;

    Collection mTopLevelCollection;

    int mErrorCode;
    QString mErrorText;

protected:
    void processJob(Akonadi::FileStore::Job *job) override;

protected:
    void setTopLevelCollection(const Collection &collection) override
    {
        mTopLevelCollection = collection;

        Collection modifiedCollection = collection;
        modifiedCollection.setContentMimeTypes(QStringList() << Collection::mimeType());

        AbstractLocalStore::setTopLevelCollection(modifiedCollection);
    }

    void checkCollectionCreate(Akonadi::FileStore::CollectionCreateJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkCollectionDelete(Akonadi::FileStore::CollectionDeleteJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkCollectionFetch(Akonadi::FileStore::CollectionFetchJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkCollectionModify(Akonadi::FileStore::CollectionModifyJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkCollectionMove(Akonadi::FileStore::CollectionMoveJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkItemCreate(Akonadi::FileStore::ItemCreateJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkItemDelete(Akonadi::FileStore::ItemDeleteJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkItemFetch(Akonadi::FileStore::ItemFetchJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkItemModify(Akonadi::FileStore::ItemModifyJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkItemMove(Akonadi::FileStore::ItemMoveJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }

    void checkStoreCompact(Akonadi::FileStore::StoreCompactJob *job, int &errorCode, QString &errorText) const override
    {
        mLastCheckedJob = job;
        errorCode = mErrorCode;
        errorText = mErrorText;
    }
};

void TestStore::processJob(Akonadi::FileStore::Job *job)
{
    mLastProcessedJob = job;

    QCOMPARE(currentJob(), job);
    QVERIFY(job->error() == 0);

    if (mErrorCode != 0) {
        notifyError(mErrorCode, mErrorText);
    }
}

class AbstractLocalStoreTest : public QObject
{
    Q_OBJECT

public:
    AbstractLocalStoreTest()
        : QObject()
        , mStore(nullptr)
    {
    }

    ~AbstractLocalStoreTest() override
    {
        delete mStore;
    }

private:
    TestStore *mStore = nullptr;

private Q_SLOTS:
    void init();
    void testSetPath();
    void testCreateCollection();
    void testDeleteCollection();
    void testFetchCollection();
    void testModifyCollection();
    void testMoveCollection();
    void testFetchItems();
    void testFetchItem();
    void testCreateItem();
    void testDeleteItem();
    void testModifyItem();
    void testMoveItem();
    void testCompactStore();
};

void AbstractLocalStoreTest::init()
{
    delete mStore;
    mStore = new TestStore;
}

void AbstractLocalStoreTest::testSetPath()
{
    const QString file = KRandom::randomString(10);
    const QString path = QLatin1String("/tmp/test/") + file;

    // check that setTopLevelCollection() has been called
    mStore->setPath(path);
    QCOMPARE(mStore->mTopLevelCollection.remoteId(), path);

    // check that the modified collection is the one returned by topLevelCollection()
    QVERIFY(mStore->mTopLevelCollection.contentMimeTypes().isEmpty());
    QCOMPARE(mStore->topLevelCollection().remoteId(), path);
    QCOMPARE(mStore->topLevelCollection().contentMimeTypes(), QStringList() << Collection::mimeType());
    QCOMPARE(mStore->topLevelCollection().name(), file);

    // check that calling with the same path again, does not call the template method
    mStore->mTopLevelCollection = Collection();
    mStore->setPath(path);
    QVERIFY(mStore->mTopLevelCollection.remoteId().isEmpty());
    QCOMPARE(mStore->topLevelCollection().remoteId(), path);
    QCOMPARE(mStore->topLevelCollection().contentMimeTypes(), QStringList() << Collection::mimeType());
    QCOMPARE(mStore->topLevelCollection().name(), file);

    // check that calling with a different path works like the first call
    const QString file2 = KRandom::randomString(10);
    const QString path2 = QLatin1String("/tmp/test2/") + file2;

    mStore->setPath(path2);
    QCOMPARE(mStore->mTopLevelCollection.remoteId(), path2);
    QCOMPARE(mStore->topLevelCollection().remoteId(), path2);
    QCOMPARE(mStore->topLevelCollection().contentMimeTypes(), QStringList() << Collection::mimeType());
    QCOMPARE(mStore->topLevelCollection().name(), file2);
}

void AbstractLocalStoreTest::testCreateCollection()
{
    Akonadi::FileStore::CollectionCreateJob *job = nullptr;

    // test without setPath()
    job = mStore->createCollection(Collection(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collections
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->createCollection(Collection(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId), but invalid target parent
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->createCollection(collection, Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collections
    Collection targetParent;
    targetParent.setRemoteId(QStringLiteral("/tmp/test2"));
    job = mStore->createCollection(collection, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->createCollection(collection, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testDeleteCollection()
{
    Akonadi::FileStore::CollectionDeleteJob *job = nullptr;

    // test without setPath()
    job = mStore->deleteCollection(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collection
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->deleteCollection(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with ivalid collection (has remoteId, but no parent collection remoteId)
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->deleteCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId and parent collection remoteId)
    Collection parentCollection;
    parentCollection.setRemoteId(QStringLiteral("/tmp/test"));
    collection.setParentCollection(parentCollection);
    job = mStore->deleteCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->deleteCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testFetchCollection()
{
    Akonadi::FileStore::CollectionFetchJob *job = nullptr;

    // test without setPath()
    job = mStore->fetchCollections(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collection
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->fetchCollections(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId)
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->fetchCollections(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->fetchCollections(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();

    // test fetch of top level collection only
    collection.setRemoteId(mStore->topLevelCollection().remoteId());
    job = mStore->fetchCollections(collection, Akonadi::FileStore::CollectionFetchJob::Base);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr); // job not handed to subclass because it is full processed in base class
    QCOMPARE(job->collections().count(), 1);
    QCOMPARE(job->collections()[0], mStore->topLevelCollection());
}

void AbstractLocalStoreTest::testModifyCollection()
{
    Akonadi::FileStore::CollectionModifyJob *job = nullptr;

    // test without setPath()
    job = mStore->modifyCollection(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid item
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->modifyCollection(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId, but no parent remoteId)
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->modifyCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test with potentially valid collection (has remoteId and parent remoteId)
    Collection parentCollection;
    parentCollection.setRemoteId(QStringLiteral("/tmp/test"));
    collection.setParentCollection(parentCollection);
    job = mStore->modifyCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->modifyCollection(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
}

void AbstractLocalStoreTest::testMoveCollection()
{
    Akonadi::FileStore::CollectionMoveJob *job = nullptr;

    // test without setPath()
    job = mStore->moveCollection(Collection(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collections
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->moveCollection(Collection(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId and parent remoteId), but invalid target parent
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    Collection parentCollection;
    parentCollection.setRemoteId(QStringLiteral("/tmp/test"));
    collection.setParentCollection(parentCollection);
    job = mStore->moveCollection(collection, Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with invalid parent collection, but with potentially valid collection and target parent
    collection.setParentCollection(Collection());
    job = mStore->moveCollection(collection, parentCollection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collections
    Collection targetParent;
    targetParent.setRemoteId(QStringLiteral("/tmp/test2"));
    collection.setParentCollection(parentCollection);
    job = mStore->moveCollection(collection, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->moveCollection(collection, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
}

void AbstractLocalStoreTest::testFetchItems()
{
    Akonadi::FileStore::ItemFetchJob *job = nullptr;

    // test without setPath()
    job = mStore->fetchItems(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collection
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->fetchItems(Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId)
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->fetchItems(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->fetchItems(collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testFetchItem()
{
    Akonadi::FileStore::ItemFetchJob *job = nullptr;

    // test without setPath()
    job = mStore->fetchItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid item
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->fetchItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid item (has remoteId)
    Item item;
    item.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->fetchItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->fetchItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testCreateItem()
{
    Akonadi::FileStore::ItemCreateJob *job = nullptr;

    // test without setPath()
    job = mStore->createItem(Item(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid collection
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->createItem(Item(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid collection (has remoteId)
    Collection collection;
    collection.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->createItem(Item(), collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->createItem(Item(), collection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testDeleteItem()
{
    Akonadi::FileStore::ItemDeleteJob *job = nullptr;

    // test without setPath()
    job = mStore->deleteItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid item
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->deleteItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid item (has remoteId)
    Item item;
    item.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->deleteItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->deleteItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

void AbstractLocalStoreTest::testModifyItem()
{
    Akonadi::FileStore::ItemModifyJob *job = nullptr;

    // test without setPath()
    job = mStore->modifyItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid item
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->modifyItem(Item());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid item (has remoteId)
    Item item;
    item.setRemoteId(QStringLiteral("/tmp/test/foo"));
    job = mStore->modifyItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->modifyItem(item);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
}

void AbstractLocalStoreTest::testMoveItem()
{
    Akonadi::FileStore::ItemMoveJob *job = nullptr;

    // test without setPath()
    job = mStore->moveItem(Item(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path but with invalid item and collection
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->moveItem(Item(), Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid item (has remoteId and parent remoteId), but invalid target parent
    Item item;
    item.setRemoteId(QStringLiteral("/tmp/test/foo"));
    Collection parentCollection;
    parentCollection.setRemoteId(QStringLiteral("/tmp/test"));
    item.setParentCollection(parentCollection);
    job = mStore->moveItem(item, Collection());
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with invalid parent collection, but with potentially valid item and target parent
    item.setParentCollection(Collection());
    job = mStore->moveItem(item, parentCollection);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with potentially valid item and collections
    Collection targetParent;
    targetParent.setRemoteId(QStringLiteral("/tmp/test2"));
    item.setParentCollection(parentCollection);
    job = mStore->moveItem(item, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->moveItem(item, targetParent);
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
}

void AbstractLocalStoreTest::testCompactStore()
{
    Akonadi::FileStore::StoreCompactJob *job = nullptr;

    // test without setPath()
    job = mStore->compactStore();
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), (int)Akonadi::FileStore::Job::InvalidStoreState);
    QVERIFY(!job->errorText().isEmpty());

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);

    // test with path
    mStore->setPath(QStringLiteral("/tmp/test"));
    job = mStore->compactStore();
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());

    QVERIFY(job->exec());
    QCOMPARE(mStore->mLastProcessedJob, job);
    mStore->mLastProcessedJob = nullptr;

    // test template check method
    mStore->mErrorCode = QRandomGenerator::global()->generate() + 1;
    mStore->mErrorText = KRandom::randomString(10);

    job = mStore->compactStore();
    QVERIFY(job != nullptr);
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(job->error(), mStore->mErrorCode);
    QCOMPARE(job->errorText(), mStore->mErrorText);

    QVERIFY(!job->exec());
    QVERIFY(mStore->mLastProcessedJob == nullptr);
    mStore->mErrorCode = 0;
    mStore->mErrorText = QString();
}

QTEST_MAIN(AbstractLocalStoreTest)

#include "abstractlocalstoretest.moc"
