/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-FileCopyrightText: 2011 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/itemfetchjob.h"
#include "filestore/itemmodifyjob.h"
#include "filestore/storecompactjob.h"

#include "libmaildir/maildir.h"

#include <Akonadi/MessageParts>

#include <Akonadi/ItemFetchScope>

#include <KMbox/MBox>

#include <KMime/Message>

#include <KRandom>

#include <QCryptographicHash>
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

class ItemModifyTest : public QObject
{
    Q_OBJECT

public:
    ItemModifyTest()
        : QObject()
        , mStore(nullptr)
        , mDir(nullptr)
    {
    }

    ~ItemModifyTest() override
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
    void testIgnorePayload();
    void testModifyPayload();
    void testModifyFlags();
    void testModifyFlagsAndPayload();
};

void ItemModifyTest::init()
{
    mStore = new MixedMaildirStore;

    mDir = new QTemporaryDir;
    QVERIFY(mDir->isValid());
    QVERIFY(QDir(mDir->path()).exists());
}

void ItemModifyTest::cleanup()
{
    delete mStore;
    mStore = nullptr;
    delete mDir;
    mDir = nullptr;
}

void ItemModifyTest::testExpectedFail()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));

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
    FileStore::ItemModifyJob *job = nullptr;

    // test failure of modifying a non-existent maildir entry
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    QString remoteId1;
    do {
        remoteId1 = KRandom::randomString(20);
    } while (entrySet1.contains(remoteId1));

    KMime::Message::Ptr msgPtr(new KMime::Message);

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(remoteId1);
    item1.setParentCollection(collection1);
    item1.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item1);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    md1EntryList = md1.entryList();
    QSet<QString> entrySet(md1EntryList.cbegin(), md1EntryList.cend());
    QCOMPARE(entrySet, entrySet1);

    // test failure of modifying a non-existent mbox entry
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    qint64 remoteId2;
    do {
        remoteId2 = QRandomGenerator::global()->generate();
    } while (entrySet2.contains(remoteId2));

    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setRemoteId(QString::number(remoteId2));
    item2.setParentCollection(collection2);
    item2.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item2);

    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)FileStore::Job::InvalidJobContext);

    QVERIFY(mbox2.load(mbox2.fileName()));
    MBoxEntry::List entryList = mbox2.entries();
    QVERIFY(std::equal(entryList.begin(), entryList.end(), entryList2.begin(), fullEntryCompare));
}

void ItemModifyTest::testIgnorePayload()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QStringList entryList1 = md1.entryList();
    QCOMPARE((int)entryList1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemModifyJob *job = nullptr;

    // test failure of modifying a non-existent maildir entry
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    const QByteArray data1 = md1.readEntry(entryList1.first());

    KMime::Message::Ptr msgPtr(new KMime::Message);
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList1.first());
    item1.setParentCollection(collection1);
    item1.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item1);
    job->setIgnorePayload(true);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QCOMPARE(md1.entryList(), entryList1);
    QCOMPARE(md1.readEntry(entryList1.first()), data1);

    // test failure of modifying a non-existent mbox entry
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    const QByteArray data2 = mbox2.readRawMessage(MBoxEntry(0));

    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setRemoteId(QStringLiteral("0"));
    item2.setParentCollection(collection2);
    item2.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item2);
    job->setIgnorePayload(true);

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QVERIFY(mbox2.load(mbox2.fileName()));
    QCOMPARE(mbox2.entries(), entryList2);
    QCOMPARE(mbox2.readRawMessage(MBoxEntry(0)), data2);
}

void ItemModifyTest::testModifyPayload()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    QStringList entryList1 = md1.entryList();
    QCOMPARE((int)entryList1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemModifyJob *job = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;

    // test modifying a maildir entry's header
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    const QByteArray data1 = md1.readEntry(entryList1.first());

    KMime::Message::Ptr msgPtr(new KMime::Message);
    msgPtr->setContent(KMime::CRLFtoLF(data1));
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList1.first());
    item1.setParentCollection(collection1);
    item1.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item1);
    // changing subject, so indicate a header change
    job->setParts(QSet<QByteArray>() << QByteArray("PLD:") + QByteArray(MessagePart::Header));

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    QCOMPARE(md1.entryList(), entryList1);

    QCOMPARE(md1.readEntry(entryList1.first()).size(), msgPtr->encodedContent().size());
    QCOMPARE(md1.readEntry(entryList1.first()), msgPtr->encodedContent());

    // check for index preservation
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // test modifying an mbox entry's header
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    const QByteArray data2 = mbox2.readRawMessage(MBoxEntry(0));

    msgPtr->setContent(KMime::CRLFtoLF(data2));
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setRemoteId(QStringLiteral("0"));
    item2.setParentCollection(collection2);
    item2.setPayload<KMime::Message::Ptr>(msgPtr);

    job = mStore->modifyItem(item2);
    // changing subject, so indicate a header change
    job->setParts(QSet<QByteArray>() << QByteArray("PLD:") + QByteArray(MessagePart::Header));

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();

    QVERIFY(mbox2.load(mbox2.fileName()));
    MBoxEntry::List entryList = mbox2.entries();
    QCOMPARE((int)entryList.count(), 5); // mbox file not purged yet
    QCOMPARE(entryList.last().messageOffset(), item.remoteId().toULongLong());

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
    QCOMPARE(collections.first(), collection2);

    FileStore::ItemFetchJob *itemFetch = mStore->fetchItem(item2);
    QVERIFY(!itemFetch->exec()); // item at old offset gone

    FileStore::StoreCompactJob *storeCompact = mStore->compactStore();
    QVERIFY(storeCompact->exec());

    QVERIFY(mbox2.load(mbox2.fileName()));
    entryList = mbox2.entries();
    QCOMPARE((int)entryList.count(), 4);

    QCOMPARE(mbox2.readRawMessage(entryList.last()), msgPtr->encodedContent());
}

void ItemModifyTest::testModifyFlags()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    const QStringList entryList1 = md1.entryList();
    QCOMPARE((int)entryList1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    QCryptographicHash cryptoHash(QCryptographicHash::Sha1);

    QFile file2(fileInfo2.absoluteFilePath());
    QVERIFY(file2.open(QIODevice::ReadOnly));
    cryptoHash.addData(file2.readAll());
    const QByteArray mbox2Sha1 = cryptoHash.result();

    file2.close();
    cryptoHash.reset();

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemModifyJob *job = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    KMime::Message::Ptr msgPtr(new KMime::Message);

    // test modifying a flag of a maildir items changes the entry name but not the
    // message contents
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    // check that the \SEEN flag is not set yet
    QVERIFY(!md1.readEntryFlags(entryList1.first()).contains("\\SEEN"));

    const QByteArray data1 = md1.readEntry(entryList1.first());

    msgPtr->setContent(KMime::CRLFtoLF(data1));
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList1.first());
    item1.setParentCollection(collection1);
    item1.setPayload<KMime::Message::Ptr>(msgPtr);
    item1.setFlag("\\SEEN");

    job = mStore->modifyItem(item1);
    // setting \SEEN, so indicate a flags change
    job->setParts(QSet<QByteArray>() << "FLAGS");

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();

    // returned item should contain the change
    QVERIFY(item.flags().contains("\\SEEN"));

    // remote ID has changed
    QVERIFY(item.remoteId() != entryList1.first());
    QVERIFY(!md1.entryList().contains(entryList1.first()));

    // no change in number of entries, one difference
    QStringList entryList3 = md1.entryList();
    QCOMPARE(entryList3.count(), entryList1.count());
    for (const QString &oldEntry : entryList1) {
        entryList3.removeAll(oldEntry);
    }
    QCOMPARE(entryList3.count(), 1);

    // no change to data
    QCOMPARE(md1.readEntry(entryList3.first()), data1);

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // fetch new item, check flag
    item1 = Item();
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList3.first());
    item1.setParentCollection(collection1);

    FileStore::ItemFetchJob *itemFetch = mStore->fetchItem(item1);

    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    QCOMPARE(itemFetch->items().count(), 1);
    QEXPECT_FAIL("", "ItemFetch handling needs to be fixed to also fetch flags", Continue);
    QVERIFY(itemFetch->items()[0].flags().contains("\\SEEN"));

    // test modifying flags of an mbox item "succeeds" (no error) but does not change
    // anything in store or on disk
    Collection collection2;
    collection2.setName(QStringLiteral("collection2"));
    collection2.setRemoteId(QStringLiteral("collection2"));
    collection2.setParentCollection(mStore->topLevelCollection());

    const QByteArray data2 = mbox2.readRawMessage(MBoxEntry(0));

    msgPtr->setContent(KMime::CRLFtoLF(data2));
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item2;
    item2.setMimeType(KMime::Message::mimeType());
    item2.setRemoteId(QStringLiteral("0"));
    item2.setParentCollection(collection2);
    item2.setPayload<KMime::Message::Ptr>(msgPtr);
    item2.setFlag("\\SEEN");

    job = mStore->modifyItem(item2);
    // setting \SEEN, so indicate a flags change
    job->setParts(QSet<QByteArray>() << "FLAGS");

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    item = job->item();

    // returned item should contain the change
    QVERIFY(item.flags().contains("\\SEEN"));

    // mbox not changed
    QVERIFY(mbox2.load(mbox2.fileName()));
    MBoxEntry::List entryList = mbox2.entries();
    QCOMPARE((int)entryList.count(), 4);

    var = job->property("compactStore");
    QVERIFY(!var.isValid());

    // file not modified
    QVERIFY(file2.open(QIODevice::ReadOnly));
    cryptoHash.addData(file2.readAll());
    QCOMPARE(cryptoHash.result(), mbox2Sha1);

    file2.close();
    cryptoHash.reset();

    // check index preservation is not triggered
    var = job->property("onDiskIndexInvalidated");
    QVERIFY(!var.isValid());
}

void ItemModifyTest::testModifyFlagsAndPayload()
{
    QDir topDir(mDir->path());

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), topDir.path(), QStringLiteral("collection1")));
    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), topDir.path(), QStringLiteral("collection2")));

    KPIM::Maildir topLevelMd(topDir.path(), true);

    KPIM::Maildir md1 = topLevelMd.subFolder(QStringLiteral("collection1"));
    const QStringList entryList1 = md1.entryList();
    QCOMPARE((int)entryList1.count(), 4);

    QFileInfo fileInfo2(topDir.path(), QStringLiteral("collection2"));
    MBox mbox2;
    QVERIFY(mbox2.load(fileInfo2.absoluteFilePath()));
    MBoxEntry::List entryList2 = mbox2.entries();
    QCOMPARE((int)entryList2.count(), 4);

    mStore->setPath(topDir.path());

    // common variables
    FileStore::ItemModifyJob *job = nullptr;

    const QVariant colListVar = QVariant::fromValue<Collection::List>(Collection::List());
    QVariant var;
    Collection::List collections;
    KMime::Message::Ptr msgPtr(new KMime::Message);

    // test modifying a flag of a maildir items changes the entry name but not the
    // message contents
    Collection collection1;
    collection1.setName(QStringLiteral("collection1"));
    collection1.setRemoteId(QStringLiteral("collection1"));
    collection1.setParentCollection(mStore->topLevelCollection());

    // check that the \SEEN flag is not set yet
    QVERIFY(!md1.readEntryFlags(entryList1.first()).contains("\\SEEN"));

    const QByteArray data1 = md1.readEntry(entryList1.first());

    msgPtr->setContent(KMime::CRLFtoLF(data1));
    msgPtr->subject()->from7BitString("Modify Test");
    msgPtr->assemble();

    Item item1;
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList1.first());
    item1.setParentCollection(collection1);
    item1.setPayload<KMime::Message::Ptr>(msgPtr);
    item1.setFlag("\\SEEN");

    job = mStore->modifyItem(item1);
    // setting \SEEN so indicate a flags change and
    // setting new subject so indicate a payload change
    job->setParts(QSet<QByteArray>() << "FLAGS" << QByteArray("PLD:") + QByteArray(MessagePart::Header));

    QVERIFY(job->exec());
    QCOMPARE(job->error(), 0);

    Item item = job->item();

    // returned item should contain the change
    QVERIFY(item.flags().contains("\\SEEN"));

    // remote ID has changed
    QVERIFY(item.remoteId() != entryList1.first());
    QVERIFY(!md1.entryList().contains(entryList1.first()));

    // no change in number of entries, one difference
    QStringList entryList3 = md1.entryList();
    QCOMPARE(entryList3.count(), entryList1.count());
    for (const QString &oldEntry : entryList1) {
        entryList3.removeAll(oldEntry);
    }
    QCOMPARE(entryList3.count(), 1);

    // data changed
    QCOMPARE(md1.readEntry(entryList3.first()), msgPtr->encodedContent());

    var = job->property("onDiskIndexInvalidated");
    QVERIFY(var.isValid());
    QCOMPARE(var.userType(), colListVar.userType());

    collections = var.value<Collection::List>();
    QCOMPARE((int)collections.count(), 1);
    QCOMPARE(collections.first(), collection1);

    // fetch new item, check flag
    item1 = Item();
    item1.setMimeType(KMime::Message::mimeType());
    item1.setRemoteId(entryList3.first());
    item1.setParentCollection(collection1);

    FileStore::ItemFetchJob *itemFetch = mStore->fetchItem(item1);
    itemFetch->fetchScope().fetchFullPayload();

    QVERIFY(itemFetch->exec());
    QCOMPARE(itemFetch->error(), 0);

    QCOMPARE(itemFetch->items().count(), 1);
    Item fetchedItem = itemFetch->items().first();
    QEXPECT_FAIL("", "ItemFetch handling needs to be fixed to also fetch flags", Continue);
    QVERIFY(fetchedItem.flags().contains("\\SEEN"));

    QVERIFY(fetchedItem.hasPayload<KMime::Message::Ptr>());
    auto fetchedMsgPtr = fetchedItem.payload<KMime::Message::Ptr>();
    QCOMPARE(msgPtr->encodedContent(), fetchedMsgPtr->encodedContent());

    // TODO test for mbox.
}

QTEST_MAIN(ItemModifyTest)

#include "itemmodifytest.moc"
