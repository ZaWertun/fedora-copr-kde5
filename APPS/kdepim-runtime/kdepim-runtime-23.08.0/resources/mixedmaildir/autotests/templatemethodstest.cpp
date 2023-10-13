/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

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

#include <Akonadi/MessageParts>

#include <Akonadi/CachePolicy>

#include <KMime/Message>

#include <KRandom>
#include <QRandomGenerator>
#include <QTemporaryDir>

#include <QTest>

using namespace Akonadi;

class TestStore : public MixedMaildirStore
{
    Q_OBJECT

public:
    TestStore()
        : mLastCheckedJob(nullptr)
        , mErrorCode(0)
    {
    }

public:
    Collection mTopLevelCollection;

    mutable FileStore::Job *mLastCheckedJob = nullptr;
    mutable int mErrorCode;
    mutable QString mErrorText;

protected:
    void setTopLevelCollection(const Collection &collection) override
    {
        MixedMaildirStore::setTopLevelCollection(collection);
    }

    void checkCollectionMove(FileStore::CollectionMoveJob *job, int &errorCode, QString &errorText) const override
    {
        MixedMaildirStore::checkCollectionMove(job, errorCode, errorText);

        mLastCheckedJob = job;
        mErrorCode = errorCode;
        mErrorText = errorText;
    }

    void checkItemCreate(FileStore::ItemCreateJob *job, int &errorCode, QString &errorText) const override
    {
        MixedMaildirStore::checkItemCreate(job, errorCode, errorText);

        mLastCheckedJob = job;
        mErrorCode = errorCode;
        mErrorText = errorText;
    }
};

class TemplateMethodsTest : public QObject
{
    Q_OBJECT

public:
    TemplateMethodsTest()
        : QObject()
        , mStore(nullptr)
    {
    }

    ~TemplateMethodsTest() override
    {
        delete mStore;
    }

private:
    QTemporaryDir mDir;
    TestStore *mStore = nullptr;

private Q_SLOTS:
    void init();
    void testSetTopLevelCollection();
    void testMoveCollection();
    void testCreateItem();
};

void TemplateMethodsTest::init()
{
    delete mStore;
    mStore = new TestStore;
    QVERIFY(mDir.isValid());
    QVERIFY(QDir(mDir.path()).exists());
}

void TemplateMethodsTest::testSetTopLevelCollection()
{
    const QString file = KRandom::randomString(10);
    const QString path = mDir.path() + file;

    mStore->setPath(path);

    // check the adjustments on the top level collection
    const Collection collection = mStore->topLevelCollection();
    QCOMPARE(collection.contentMimeTypes(), QStringList() << Collection::mimeType());
    QCOMPARE(collection.rights(), Collection::CanCreateCollection | Collection::CanChangeCollection | Collection::CanDeleteCollection);
    const CachePolicy cachePolicy = collection.cachePolicy();
    QVERIFY(!cachePolicy.inheritFromParent());
    QCOMPARE(cachePolicy.localParts(), QStringList() << QLatin1String(MessagePart::Envelope));
    QVERIFY(cachePolicy.syncOnDemand());
}

void TemplateMethodsTest::testMoveCollection()
{
    mStore->setPath(mDir.path());

    FileStore::CollectionMoveJob *job = nullptr;

    // test moving into itself
    Collection collection(QRandomGenerator::global()->generate());
    collection.setParentCollection(mStore->topLevelCollection());
    collection.setRemoteId(QStringLiteral("collection"));
    job = mStore->moveCollection(collection, collection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(mStore->mErrorCode, job->error());
    QCOMPARE(mStore->mErrorText, job->errorText());

    QVERIFY(!job->exec());

    // test moving into child
    Collection childCollection(collection.id() + 1);
    childCollection.setParentCollection(collection);
    childCollection.setRemoteId(QStringLiteral("child"));
    job = mStore->moveCollection(collection, childCollection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(mStore->mErrorCode, job->error());
    QCOMPARE(mStore->mErrorText, job->errorText());

    QVERIFY(!job->exec());

    // test moving into grand child child
    Collection grandChildCollection(collection.id() + 2);
    grandChildCollection.setParentCollection(childCollection);
    grandChildCollection.setRemoteId(QStringLiteral("grandchild"));
    job = mStore->moveCollection(collection, grandChildCollection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(mStore->mErrorCode, job->error());
    QCOMPARE(mStore->mErrorText, job->errorText());

    QVERIFY(!job->exec());

    // test moving into unrelated collection
    Collection otherCollection(collection.id() + QRandomGenerator::global()->generate());
    otherCollection.setParentCollection(mStore->topLevelCollection());
    otherCollection.setRemoteId(QStringLiteral("other"));
    job = mStore->moveCollection(collection, otherCollection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);

    // collections don't exist in the store, so processing still fails
    QVERIFY(!job->exec());
}

void TemplateMethodsTest::testCreateItem()
{
    mStore->setPath(mDir.path());

    Collection collection(QRandomGenerator::global()->generate());
    collection.setParentCollection(mStore->topLevelCollection());
    collection.setRemoteId(QStringLiteral("collection"));

    FileStore::ItemCreateJob *job = nullptr;

    // test item without payload
    Item item(KMime::Message::mimeType());
    job = mStore->createItem(item, collection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);
    QVERIFY(!job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);
    QCOMPARE(mStore->mErrorCode, job->error());
    QCOMPARE(mStore->mErrorText, job->errorText());

    QVERIFY(!job->exec());

    // test item with payload
    item.setPayloadFromData("Subject: dummy payload\n\nwith some content");
    job = mStore->createItem(item, collection);
    QVERIFY(job != nullptr);
    QCOMPARE(job->error(), 0);
    QVERIFY(job->errorText().isEmpty());
    QCOMPARE(mStore->mLastCheckedJob, job);

    // collections don't exist in the store, so processing still fails
    QVERIFY(!job->exec());
}

QTEST_MAIN(TemplateMethodsTest)

#include "templatemethodstest.moc"
