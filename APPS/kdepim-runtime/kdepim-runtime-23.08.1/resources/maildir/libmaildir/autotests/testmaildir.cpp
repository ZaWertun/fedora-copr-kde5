/*
  This file is part of the kpimutils library.

  SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testmaildir.h"

#include <memory>

#include <Akonadi/MessageFlags>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

QTEST_MAIN(MaildirTest)

#include "../maildir.h"
using namespace KPIM;

static const char *testDir = "libmaildir-unit-test";
static const char *testString = "From: theDukeOfMonmouth@uk.gov\n \nTo: theDukeOfBuccleuch@uk.gov\n\ntest\n";
static const char *testStringHeaders = "From: theDukeOfMonmouth@uk.gov\n \nTo: theDukeOfBuccleuch@uk.gov\n";

void MaildirTest::init()
{
    QString tmpPath(QDir::tempPath() + QLatin1Char('/') + QLatin1String(testDir));
    QDir().mkpath(tmpPath);
    m_temp = new QTemporaryDir(tmpPath);

    QDir temp(m_temp->path());
    QVERIFY(temp.exists());

    temp.mkdir(QStringLiteral("new"));
    QVERIFY(temp.exists(QLatin1String("new")));
    temp.mkdir(QStringLiteral("cur"));
    QVERIFY(temp.exists(QLatin1String("cur")));
    temp.mkdir(QStringLiteral("tmp"));
    QVERIFY(temp.exists(QLatin1String("tmp")));
}

void MaildirTest::cleanup()
{
    m_temp->remove();
    QDir d(m_temp->path());
    const QString subFolderPath(QStringLiteral(".%1.directory").arg(d.dirName()));
    QDir((subFolderPath)).removeRecursively();

    delete m_temp;
    m_temp = nullptr;
}

void MaildirTest::fillDirectory(const QString &name, int limit)
{
    QFile file;
    QDir::setCurrent(m_temp->path() + QLatin1Char('/') + name);
    for (int i = 0; i < limit; i++) {
        file.setFileName(QLatin1String("testmail-") + QString::number(i));
        file.open(QIODevice::WriteOnly);
        file.write(testString);
        file.flush();
        file.close();
    }
}

void MaildirTest::createSubFolders()
{
    QDir d(m_temp->path());
    const QString subFolderPath(QStringLiteral(".%1.directory").arg(d.dirName()));
    d.cdUp();
    d.mkdir(subFolderPath);
    d.cd(subFolderPath);
    d.mkdir(QStringLiteral("foo"));
    d.mkdir(QStringLiteral("barbar"));
    d.mkdir(QStringLiteral("bazbaz"));
}

void MaildirTest::fillNewDirectory()
{
    fillDirectory(QStringLiteral("new"), 140);
}

void MaildirTest::fillCurrentDirectory()
{
    fillDirectory(QStringLiteral("cur"), 20);
}

void MaildirTest::testMaildirInstantiation()
{
    Maildir d(QStringLiteral("/foo/bar/Mail"));
    Maildir d2(d);
    Maildir d3;
    d3 = d;
    QVERIFY(d == d2);
    QVERIFY(d3 == d2);
    QVERIFY(d == d3);
    QCOMPARE(d.path(), QString(QLatin1String("/foo/bar/Mail")));
    QCOMPARE(d.name(), QString(QLatin1String("Mail")));

    QVERIFY(!d.isValid(false));

    Maildir good(m_temp->path());
    QVERIFY(good.isValid(false));

    QDir temp(m_temp->path());
    temp.rmdir(QStringLiteral("new"));
    QVERIFY(!good.isValid(false));
    QVERIFY(!good.lastError().isEmpty());

    Maildir root1(QStringLiteral("/foo/bar/Mail"), true);
    QVERIFY(root1.isRoot());

    Maildir root1Copy = root1;
    QCOMPARE(root1Copy.path(), root1.path());
    QCOMPARE(root1Copy.isRoot(), root1.isRoot());

    // FIXME test insufficient permissions?
}

void MaildirTest::testMaildirListing()
{
    fillNewDirectory();

    Maildir d(m_temp->path());
    QStringList entries = d.entryList();

    QCOMPARE(entries.count(), 140);

    fillCurrentDirectory();
    entries = d.entryList();
    QCOMPARE(entries.count(), 160);
}

void MaildirTest::testMaildirAccess()
{
    fillCurrentDirectory();
    Maildir d(m_temp->path());
    QStringList entries = d.entryList();
    QCOMPARE(entries.count(), 20);

    QByteArray data = d.readEntry(entries[0]);
    QVERIFY(!data.isEmpty());
    QCOMPARE(data, QByteArray(testString));
}

void MaildirTest::testMaildirReadHeaders()
{
    fillCurrentDirectory();
    Maildir d(m_temp->path());
    QStringList entries = d.entryList();
    QCOMPARE(entries.count(), 20);

    QByteArray data = d.readEntryHeaders(entries[0]);
    QVERIFY(!data.isEmpty());
    QCOMPARE(data, QByteArray(testStringHeaders));
}

void MaildirTest::testMaildirWrite()
{
    fillCurrentDirectory();
    Maildir d(m_temp->path());
    QStringList entries = d.entryList();
    QCOMPARE(entries.count(), 20);

    QByteArray data = d.readEntry(entries[0]);
    QByteArray data2 = "changed\n";
    QVERIFY(d.writeEntry(entries[0], data2));
    QCOMPARE(data2, d.readEntry(entries[0]));
}

void MaildirTest::testMaildirAppend()
{
    Maildir d(m_temp->path());
    QByteArray data = "newentry\n";
    QString key = d.addEntry(data);
    QVERIFY(!key.isEmpty());
    QCOMPARE(data, d.readEntry(key));
}

void MaildirTest::testMaildirCreation()
{
    QString p(QStringLiteral("CREATETEST"));
    QString tmpPath(QDir::tempPath() + QLatin1Char('/') + p);
    QDir().mkpath(tmpPath);
    std::unique_ptr<QTemporaryDir> temp(new QTemporaryDir(tmpPath));
    Maildir d(temp->path() + p);
    QVERIFY(!d.isValid(false));
    d.create();
    QVERIFY(d.isValid(false));
}

void MaildirTest::testMaildirRemoveEntry()
{
    Maildir d(m_temp->path());
    QByteArray data = "newentry\n";
    QString key = d.addEntry(data);
    QVERIFY(!key.isEmpty());
    QCOMPARE(data, d.readEntry(key));
    QVERIFY(d.removeEntry(key));
    QVERIFY(d.readEntry(key).isEmpty());
}

void MaildirTest::testMaildirListSubfolders()
{
    fillNewDirectory();

    Maildir d(m_temp->path());
    QStringList entries = d.subFolderList();

    QVERIFY(entries.isEmpty());

    createSubFolders();

    entries = d.subFolderList();
    QVERIFY(!entries.isEmpty());
    QCOMPARE(entries.count(), 3);
}

void MaildirTest::testMaildirCreateSubfolder()
{
    Maildir d(m_temp->path());
    QStringList entries = d.subFolderList();
    QVERIFY(entries.isEmpty());

    d.addSubFolder(QStringLiteral("subFolderTest"));
    entries = d.subFolderList();
    QVERIFY(!entries.isEmpty());
    QCOMPARE(entries.count(), 1);
    Maildir child = d.subFolder(entries.first());
    QVERIFY(child.isValid(false));
}

void MaildirTest::testMaildirRemoveSubfolder()
{
    Maildir d(m_temp->path());
    QVERIFY(d.isValid(false));

    QString folderPath = d.addSubFolder(QStringLiteral("subFolderTest"));
    QVERIFY(!folderPath.isEmpty());
    QVERIFY(folderPath.endsWith(QLatin1String(".directory/subFolderTest")));
    bool removingWorked = d.removeSubFolder(QStringLiteral("subFolderTest"));
    QVERIFY(removingWorked);
}

void MaildirTest::testMaildirRename()
{
    Maildir d(m_temp->path());
    QVERIFY(d.isValid(false));

    QString folderPath = d.addSubFolder(QStringLiteral("rename me!"));
    QVERIFY(!folderPath.isEmpty());

    Maildir d2(folderPath);
    QVERIFY(d2.isValid(false));
    QVERIFY(d2.rename(QLatin1String("renamed")));
    QCOMPARE(d2.name(), QString(QLatin1String("renamed")));

    // same again, should not fail
    QVERIFY(d2.rename(QLatin1String("renamed")));
    QCOMPARE(d2.name(), QString(QLatin1String("renamed")));

    // already existing name
    QVERIFY(!d.addSubFolder(QLatin1String("this name is already taken")).isEmpty());
    QVERIFY(!d2.rename(QLatin1String("this name is already taken")));
}

void MaildirTest::testMaildirMoveTo()
{
    Maildir d(m_temp->path());
    QVERIFY(d.isValid(false));

    QString folderPath1 = d.addSubFolder(QStringLiteral("child1"));
    QVERIFY(!folderPath1.isEmpty());

    Maildir d2(folderPath1);
    QVERIFY(d2.isValid(false));

    QDir d2Dir(d2.path());
    QVERIFY(d2Dir.exists());

    QString folderPath11 = d2.addSubFolder(QStringLiteral("grandchild1"));

    Maildir d21(folderPath11);
    QVERIFY(d21.isValid(false));

    QDir d2SubDir(Maildir::subDirPathForFolderPath(d2.path()));
    QVERIFY(d2SubDir.exists());

    QString folderPath2 = d.addSubFolder(QStringLiteral("child2"));
    QVERIFY(!folderPath2.isEmpty());

    Maildir d3(folderPath2);
    QVERIFY(d3.isValid(false));

    // move child1 to child2
    QVERIFY(d2.moveTo(d3));

    Maildir d31 = d3.subFolder(QStringLiteral("child1"));
    QVERIFY(d31.isValid(false));

    QVERIFY(!d2Dir.exists());
    QVERIFY(!d2SubDir.exists());

    QDir d31Dir(d31.path());
    QVERIFY(d31Dir.exists());

    QDir d31SubDir(Maildir::subDirPathForFolderPath(d31.path()));
    QVERIFY(d31SubDir.exists());

    Maildir d311 = d31.subFolder(QStringLiteral("grandchild1"));
    QVERIFY(d311.isValid(false));

    // try moving again
    d2 = Maildir(folderPath1);
    QVERIFY(!d2.isValid(false));
    QVERIFY(!d2.moveTo(d3));
}

void MaildirTest::testMaildirFlagsReading()
{
    QFile file;
    const QStringList markers = QStringList() << QStringLiteral("P") << QStringLiteral("R") << QStringLiteral("S") << QStringLiteral("F")
                                              << QStringLiteral("FPRS");
    QDir::setCurrent(m_temp->path() + QStringLiteral("/cur"));
    for (int i = 0; i < 6; i++) {
        QString fileName = QLatin1String("testmail-") + QString::number(i);
        if (i < 5) {
            fileName +=
#ifdef Q_OS_WIN
                QLatin1String("!2,")
#else
                QLatin1String(":2,")
#endif
                + markers[i];
        }
        file.setFileName(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(testString);
        file.flush();
        file.close();
    }

    Maildir d(m_temp->path());
    QStringList entries = d.entryList();
    // Maildir::entryList() doesn't sort for performance reasons,
    // do it here to make test sequence reliable.
    entries.sort();

    QCOMPARE(entries.count(), 6);

    Akonadi::Item::Flags flags = d.readEntryFlags(entries[0]);
    QCOMPARE(flags.count(), 1);
    QVERIFY(flags.contains(Akonadi::MessageFlags::Forwarded));

    flags = d.readEntryFlags(entries[1]);
    QCOMPARE(flags.count(), 1);
    QVERIFY(flags.contains(Akonadi::MessageFlags::Replied));

    flags = d.readEntryFlags(entries[2]);
    QCOMPARE(flags.count(), 1);
    QVERIFY(flags.contains(Akonadi::MessageFlags::Seen));

    flags = d.readEntryFlags(entries[3]);
    QCOMPARE(flags.count(), 1);
    QVERIFY(flags.contains(Akonadi::MessageFlags::Flagged));

    flags = d.readEntryFlags(entries[4]);
    QCOMPARE(flags.count(), 4);
    QVERIFY(flags.contains(Akonadi::MessageFlags::Forwarded));
    QVERIFY(flags.contains(Akonadi::MessageFlags::Replied));
    QVERIFY(flags.contains(Akonadi::MessageFlags::Seen));
    QVERIFY(flags.contains(Akonadi::MessageFlags::Flagged));

    flags = d.readEntryFlags(entries[5]);
    QVERIFY(flags.isEmpty());
}

void MaildirTest::testMaildirFlagsWriting_data()
{
    QTest::addColumn<QString>("origDir");
    QTest::addColumn<QString>("origFileName");
    QTest::newRow("cur/") << "cur"
                          << "testmail";
    QTest::newRow("cur/S") << "cur"
                           << "testmail:2,S"; // wrongly marked as "seen" on disk (#289428)
    QTest::newRow("new/") << "new"
                          << "testmail";
    QTest::newRow("new/S") << "new"
                           << "testmail:2,S";
}

void MaildirTest::testMaildirFlagsWriting()
{
    QFETCH(QString, origDir);
    QFETCH(QString, origFileName);

    // create an initially new mail
    QFile file;
    QDir::setCurrent(m_temp->path());
    file.setFileName(origDir + QLatin1Char('/') + origFileName);
    file.open(QIODevice::WriteOnly);
    file.write(testString);
    file.flush();
    file.close();

    // add a single flag
    Maildir d(m_temp->path());
    const QStringList entries = d.entryList();
    QCOMPARE(entries.size(), 1);
    QVERIFY(QFile::exists(origDir + QLatin1Char('/') + entries[0]));
    const QString newKey = d.changeEntryFlags(entries[0], Akonadi::Item::Flags() << Akonadi::MessageFlags::Seen);
    // make sure the new key exists
    QCOMPARE(newKey, d.entryList()[0]);
    QVERIFY(QFile::exists(QStringLiteral("cur/") + newKey));
    // and it's the right file
    QCOMPARE(d.readEntry(newKey), QByteArray(testString));
    // now check the file name
    QVERIFY(newKey.endsWith(QLatin1String("2,S")));
    // and more flags
    const QString newKey2 = d.changeEntryFlags(newKey, Akonadi::Item::Flags() << Akonadi::MessageFlags::Seen << Akonadi::MessageFlags::Replied);
    // check the file name, and the sorting of markers
    QVERIFY(newKey2.endsWith(QLatin1String("2,RS")));
    QVERIFY(QFile::exists(QStringLiteral("cur/") + newKey2));
}
