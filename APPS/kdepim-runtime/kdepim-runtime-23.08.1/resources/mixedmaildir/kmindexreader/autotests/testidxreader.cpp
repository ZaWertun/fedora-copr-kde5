/*
 *   SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *   SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "testidxreader.h"

#include "../kmindexreader.h"

#include "TestIdxReader_data.h"

#include <Akonadi/MessageStatus>
using Akonadi::MessageStatus;

#include <QTemporaryFile>

#include <QDebug>
#include <QTest>

QTEST_MAIN(TestIdxReader)

TestIdxReader::TestIdxReader() = default;

void TestIdxReader::testError()
{
    KMIndexReader reader(QStringLiteral("IDoNotExist"));

    QVERIFY(reader.error() == true);
}

void TestIdxReader::testReadHeader()
{
    QTemporaryFile tmp;
    if (!tmp.open()) {
        qDebug() << "Could not open temp file.";
        return;
    }
    tmp.write(QByteArray::fromBase64(mailDirOneEmail));
    tmp.close();
    KMIndexReader reader(tmp.fileName());

    QVERIFY(reader.error() == false);

    int version = 0;
    bool success = reader.readHeader(&version);

    QVERIFY(success == true);
    QCOMPARE(version, 1506);

    QVERIFY(reader.error() == false);
}

void TestIdxReader::testRead()
{
    QTemporaryFile tmp;
    if (!tmp.open()) {
        qDebug() << "Could not open temp file.";
        return;
    }
    tmp.write(QByteArray::fromBase64(mailDirTwoEmailOneTagFlags));
    tmp.close();
    KMIndexReader reader(tmp.fileName());
    QVERIFY(reader.error() == false);
    bool success = reader.readIndex();
    QVERIFY(success == true);

    QVERIFY(reader.messages().size() == 2);

    KMIndexDataPtr msg = reader.messages().front();

    QString subject = msg->mCachedStringParts[KMIndexReader::MsgSubjectPart];
    MessageStatus status;
    status.fromQInt32(msg->mCachedLongParts[KMIndexReader::MsgStatusPart]);
    QCOMPARE(subject, QStringLiteral("hello from kmail"));
    QVERIFY(!status.isImportant());
    QVERIFY(!msg->status().isImportant());
    QVERIFY(msg->status().isRead());
    QVERIFY(msg->tagList().contains(QLatin1String("N5tUHPOZFf")));

    msg = reader.messages().back();
    status.fromQInt32(msg->mCachedLongParts[KMIndexReader::MsgStatusPart]);
    subject = msg->mCachedStringParts[KMIndexReader::MsgSubjectPart];
    QCOMPARE(subject, QStringLiteral("foo bar"));
    QVERIFY(status.isImportant());
    QVERIFY(msg->status().isImportant());
    QVERIFY(!msg->status().isRead());
    QVERIFY(msg->tagList().size() == 0);
}
