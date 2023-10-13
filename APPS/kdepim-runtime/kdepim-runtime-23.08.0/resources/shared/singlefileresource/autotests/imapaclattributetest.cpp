/*
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../imapaclattribute.h"
#include <QTest>

using namespace Akonadi;

using ImapAcl = QMap<QByteArray, KIMAP::Acl::Rights>;

Q_DECLARE_METATYPE(ImapAcl)
Q_DECLARE_METATYPE(KIMAP::Acl::Rights)

class ImapAclAttributeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void shouldHaveDefaultValue()
    {
        ImapAclAttribute attr;
        QVERIFY(attr.oldRights().isEmpty());
        QVERIFY(attr.rights().isEmpty());
        QVERIFY(!attr.myRights());
    }

    void shouldBuildAttribute()
    {
        QMap<QByteArray, KIMAP::Acl::Rights> right;
        right.insert("test", KIMAP::Acl::Admin);
        right.insert("foo", KIMAP::Acl::Admin);

        QMap<QByteArray, KIMAP::Acl::Rights> oldright;
        right.insert("test", KIMAP::Acl::Delete);
        right.insert("foo", KIMAP::Acl::Delete);
        ImapAclAttribute attr(right, oldright);
        QCOMPARE(attr.oldRights(), oldright);
        QCOMPARE(attr.rights(), right);
    }

    void shouldAssignValue()
    {
        ImapAclAttribute attr;
        QMap<QByteArray, KIMAP::Acl::Rights> right;
        right.insert("test", KIMAP::Acl::Admin);
        right.insert("foo", KIMAP::Acl::Admin);
        attr.setRights(right);
        QCOMPARE(attr.rights(), right);
    }

    void shouldCloneAttr()
    {
        ImapAclAttribute attr;
        QMap<QByteArray, KIMAP::Acl::Rights> right;
        right.insert("test", KIMAP::Acl::Admin);
        right.insert("foo", KIMAP::Acl::Admin);
        attr.setRights(right);
        ImapAclAttribute *clone = attr.clone();
        QVERIFY(attr == *clone);
        delete clone;
    }

    void shouldSerializedAttribute()
    {
        QMap<QByteArray, KIMAP::Acl::Rights> right;
        right.insert("test", KIMAP::Acl::Admin);
        right.insert("foo", KIMAP::Acl::Admin);

        QMap<QByteArray, KIMAP::Acl::Rights> oldright;
        right.insert("test", KIMAP::Acl::Delete);
        right.insert("foo", KIMAP::Acl::Delete);
        ImapAclAttribute attr(right, oldright);
        const QByteArray ba = attr.serialized();
        ImapAclAttribute result;
        result.deserialize(ba);
        QVERIFY(attr == result);
    }

    void shouldHaveType()
    {
        ImapAclAttribute attr;
        QCOMPARE(attr.type(), QByteArray("imapacl"));
    }

    void testMyRights()
    {
        ImapAclAttribute attr;
        KIMAP::Acl::Rights myRight = KIMAP::Acl::Admin;

        attr.setMyRights(myRight);
        QCOMPARE(attr.myRights(), myRight);

        ImapAclAttribute *clone = attr.clone();
        QCOMPARE(clone->myRights(), myRight);

        QVERIFY(*clone == attr);

        clone->setMyRights(KIMAP::Acl::Custom0);
        QVERIFY(!(*clone == attr));
        delete clone;
    }

    void testDeserialize_data()
    {
        QTest::addColumn<ImapAcl>("rights");
        QTest::addColumn<KIMAP::Acl::Rights>("myRights");
        QTest::addColumn<QByteArray>("serialized");

        KIMAP::Acl::Rights rights = KIMAP::Acl::None;

        {
            ImapAcl acl;
            QTest::newRow("empty") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray(" %% ");
        }

        {
            ImapAcl acl;
            acl.insert("user@host", rights);
            QTest::newRow("none") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host  %% ");
        }

        {
            ImapAcl acl;
            acl.insert("user@host", KIMAP::Acl::Lookup);
            QTest::newRow("lookup") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host l %% ");
        }

        {
            ImapAcl acl;
            acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
            QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host lr %% ");
        }

        {
            ImapAcl acl;
            acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
            acl.insert("otheruser@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
            QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("otheruser@host lr % user@host lr %% ");
        }

        {
            QTest::newRow("myrights") << ImapAcl() << KIMAP::Acl::rightsFromString("lrswipckxtdaen") << QByteArray(" %%  %% lrswipckxtdaen");
        }
    }

    void testDeserialize()
    {
        QFETCH(ImapAcl, rights);
        QFETCH(KIMAP::Acl::Rights, myRights);
        QFETCH(QByteArray, serialized);

        ImapAclAttribute deserializeAttr;
        deserializeAttr.deserialize(serialized);
        QCOMPARE(deserializeAttr.rights(), rights);
        QCOMPARE(deserializeAttr.myRights(), myRights);
    }

    void testSerializeDeserialize_data()
    {
        QTest::addColumn<ImapAcl>("rights");
        QTest::addColumn<KIMAP::Acl::Rights>("myRights");
        QTest::addColumn<QByteArray>("serialized");
        QTest::addColumn<QByteArray>("oldSerialized");

        ImapAcl acl;
        QTest::newRow("empty") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray(" %% ") << QByteArray("testme@host l %% ");

        acl.insert("user@host", KIMAP::Acl::None);
        QTest::newRow("none") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host  %% ") << QByteArray("testme@host l %% user@host ");

        acl.insert("user@host", KIMAP::Acl::Lookup);
        QTest::newRow("lookup") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host l %% ") << QByteArray("testme@host l %% user@host l");

        acl.insert("user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("user@host lr %% ")
                                     << QByteArray("testme@host l %% user@host lr");

        acl.insert("otheruser@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        QTest::newRow("lookup/read") << acl << KIMAP::Acl::Rights(KIMAP::Acl::None) << QByteArray("otheruser@host lr % user@host lr %% ")
                                     << QByteArray("testme@host l %% otheruser@host lr % user@host lr");

        QTest::newRow("myrights") << acl << KIMAP::Acl::rightsFromString("lrswipckxtdaen")
                                  << QByteArray("otheruser@host lr % user@host lr %%  %% lrswipckxtdaen")
                                  << QByteArray("testme@host l %% otheruser@host lr % user@host lr %% lrswipckxtdaen");
    }

    void testSerializeDeserialize()
    {
        QFETCH(ImapAcl, rights);
        QFETCH(KIMAP::Acl::Rights, myRights);
        QFETCH(QByteArray, serialized);
        QFETCH(QByteArray, oldSerialized);

        auto attr = new ImapAclAttribute();
        attr->setRights(rights);
        attr->setMyRights(myRights);
        QCOMPARE(attr->serialized(), serialized);

        ImapAcl acl;
        acl.insert("testme@host", KIMAP::Acl::Lookup);
        attr->setRights(acl);

        QCOMPARE(attr->serialized(), oldSerialized);

        delete attr;

        ImapAclAttribute deserializeAttr;
        deserializeAttr.deserialize(serialized);
        QCOMPARE(deserializeAttr.rights(), rights);
        QCOMPARE(deserializeAttr.myRights(), myRights);
    }

    void testOldRights()
    {
        ImapAcl acls;
        acls.insert("first_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        acls.insert("second_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);
        acls.insert("third_user@host", KIMAP::Acl::Lookup | KIMAP::Acl::Read);

        auto attr = new ImapAclAttribute();
        attr->setRights(acls);

        ImapAcl oldAcls = acls;

        acls.remove("first_user@host");
        acls.remove("third_user@host");

        attr->setRights(acls);

        QCOMPARE(attr->oldRights(), oldAcls);

        attr->setRights(acls);

        QCOMPARE(attr->oldRights(), acls);
        delete attr;
    }
};

QTEST_MAIN(ImapAclAttributeTest)

#include "imapaclattributetest.moc"
