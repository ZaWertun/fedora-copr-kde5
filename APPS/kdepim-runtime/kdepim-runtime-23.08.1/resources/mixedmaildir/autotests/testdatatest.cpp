/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testdatautil.h"

#include <QTemporaryDir>

#include <QDir>
#include <QTest>

class TestDataTest : public QObject
{
    Q_OBJECT
public:
    TestDataTest() = default;

private Q_SLOTS:
    void testResources();
    void testInstall();
};

void TestDataTest::testResources()
{
    const QStringList testDataNames = TestDataUtil::testDataNames();
    QCOMPARE(testDataNames,
             QStringList() << QStringLiteral("dimap") << QStringLiteral("maildir") << QStringLiteral("maildir-tagged") << QStringLiteral("mbox")
                           << QStringLiteral("mbox-tagged") << QStringLiteral("mbox-unpurged"));

    for (const QString &testDataName : testDataNames) {
        if (testDataName.startsWith(QLatin1String("mbox"))) {
            QVERIFY(TestDataUtil::folderType(testDataName) == TestDataUtil::MBoxFolder);
        } else {
            QVERIFY(TestDataUtil::folderType(testDataName) == TestDataUtil::MaildirFolder);
        }
    }

    // TODO check contents?
}

void TestDataTest::testInstall()
{
    QTemporaryDir dir;
    QDir installDir(dir.path());
    QDir curDir;

    const QString indexFilePattern = QStringLiteral(".%1.index");

    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox"), dir.path(), QStringLiteral("mbox1")));
    QVERIFY(installDir.exists(QLatin1String("mbox1")));
    QVERIFY(installDir.exists(indexFilePattern.arg(QLatin1String("mbox1"))));

    QVERIFY(TestDataUtil::installFolder(QLatin1String("mbox-tagged"), dir.path(), QStringLiteral("mbox2")));
    QVERIFY(installDir.exists(QLatin1String("mbox2")));
    QVERIFY(installDir.exists(indexFilePattern.arg(QLatin1String("mbox2"))));

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir"), dir.path(), QStringLiteral("md1")));
    QVERIFY(installDir.exists(QLatin1String("md1")));
    QVERIFY(installDir.exists(QLatin1String("md1/new")));
    QVERIFY(installDir.exists(QLatin1String("md1/cur")));
    QVERIFY(installDir.exists(QLatin1String("md1/tmp")));
    QVERIFY(installDir.exists(indexFilePattern.arg(QLatin1String("md1"))));

    curDir = installDir;
    curDir.cd(QStringLiteral("md1"));
    curDir.cd(QStringLiteral("cur"));
    curDir.setFilter(QDir::Files);
    QCOMPARE((int)curDir.count(), 4);

    QVERIFY(TestDataUtil::installFolder(QLatin1String("maildir-tagged"), dir.path(), QStringLiteral("md2")));
    QVERIFY(installDir.exists(QLatin1String("md2")));
    QVERIFY(installDir.exists(QLatin1String("md2/new")));
    QVERIFY(installDir.exists(QLatin1String("md2/cur")));
    QVERIFY(installDir.exists(QLatin1String("md2/tmp")));
    QVERIFY(installDir.exists(indexFilePattern.arg(QLatin1String("md2"))));

    curDir = installDir;
    curDir.cd(QStringLiteral("md2"));
    curDir.cd(QStringLiteral("cur"));
    curDir.setFilter(QDir::Files);
    QCOMPARE((int)curDir.count(), 4);
}

#include "testdatatest.moc"

QTEST_MAIN(TestDataTest)
