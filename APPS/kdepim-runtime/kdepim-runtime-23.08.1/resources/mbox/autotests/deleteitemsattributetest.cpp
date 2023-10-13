/*
  SPDX-FileCopyrightText: 2015-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deleteitemsattributetest.h"
#include "../deleteditemsattribute.h"
#include <QTest>

DeleteItemsAttributeTest::DeleteItemsAttributeTest(QObject *parent)
    : QObject(parent)
{
}

DeleteItemsAttributeTest::~DeleteItemsAttributeTest() = default;

void DeleteItemsAttributeTest::shouldHaveDefaultValue()
{
    DeletedItemsAttribute attr;
    QVERIFY(attr.deletedItemOffsets().isEmpty());
    QCOMPARE(attr.offsetCount(), 0);
}

void DeleteItemsAttributeTest::shouldAssignValue()
{
    DeletedItemsAttribute attr;
    QSet<quint64> lst;
    lst.insert(15);
    attr.addDeletedItemOffset(15);
    lst.insert(154);
    attr.addDeletedItemOffset(154);
    lst.insert(225);
    attr.addDeletedItemOffset(225);
    QVERIFY(!attr.deletedItemOffsets().isEmpty());
    QCOMPARE(attr.offsetCount(), 3);
    QCOMPARE(lst, attr.deletedItemOffsets());
}

void DeleteItemsAttributeTest::shouldDeserializeValue()
{
    DeletedItemsAttribute attr;
    attr.addDeletedItemOffset(15);
    attr.addDeletedItemOffset(154);
    attr.addDeletedItemOffset(225);
    const QByteArray ba = attr.serialized();
    DeletedItemsAttribute result;
    result.deserialize(ba);
    QVERIFY(result == attr);
}

void DeleteItemsAttributeTest::shouldCloneAttribute()
{
    DeletedItemsAttribute attr;
    attr.addDeletedItemOffset(15);
    attr.addDeletedItemOffset(154);
    attr.addDeletedItemOffset(225);
    DeletedItemsAttribute *result = attr.clone();
    QVERIFY(*result == attr);
    delete result;
}

void DeleteItemsAttributeTest::shouldHaveTypeName()
{
    DeletedItemsAttribute attr;
    QCOMPARE(attr.type(), QByteArray("DeletedMboxItems"));
}

QTEST_MAIN(DeleteItemsAttributeTest)
