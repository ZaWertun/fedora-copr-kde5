/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kolabobjecttest.h"

#include <QTest>

#include "kolabformat/kolabobject.h"
#include <kolabformat/errorhandler.h>

void KolabObjectTest::preserveLatin1()
{
    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(QDate(2014, 1, 1), {}));
    event->setAllDay(true);
    const QString summary(QLatin1String("äöü%@$£é¤¼²°"));
    event->setSummary(summary);
    QCOMPARE(event->summary(), summary);
    // std::cout << event->summary().toStdString() << std::endl;
    KMime::Message::Ptr msg = Kolab::KolabObjectWriter::writeEvent(event);
    //     qDebug() << msg->encodedContent();
    KCalendarCore::Event::Ptr readEvent = Kolab::KolabObjectReader(msg).getEvent();
    QVERIFY(readEvent);
    //     std::cout << readEvent->summary().toStdString() << std::endl;
    QCOMPARE(readEvent->summary(), summary);
}

void KolabObjectTest::preserveUnicode()
{
    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(QDate(2014, 1, 1), {}));
    event->setAllDay(true);
    QString summary(QStringLiteral("€Š�ـأبـ☺"));
    event->setSummary(summary);
    QCOMPARE(event->summary(), summary);
    //     std::cout << event->summary().toStdString() << std::endl;
    KMime::Message::Ptr msg = Kolab::KolabObjectWriter::writeEvent(event);
    //     qDebug() << msg->encodedContent();
    KCalendarCore::Event::Ptr readEvent = Kolab::KolabObjectReader(msg).getEvent();
    QVERIFY(readEvent);
    //     std::cout << readEvent->summary().toStdString() << std::endl;
    QCOMPARE(readEvent->summary(), summary);
}

void KolabObjectTest::dontCrashWithEmptyOrganizer()
{
    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(QDate(2012, 11, 11), {}));
    event->setAllDay(true);
    Kolab::KolabObjectWriter::writeEvent(event, Kolab::KolabV2);
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
    Kolab::KolabObjectWriter::writeEvent(event);
    qDebug() << Kolab::ErrorHandler::instance().error();
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Debug);
}

void KolabObjectTest::dontCrashWithEmptyIncidence()
{
    Kolab::KolabObjectWriter::writeEvent(KCalendarCore::Event::Ptr());
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Critical);
    Kolab::KolabObjectWriter::writeTodo(KCalendarCore::Todo::Ptr());
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Critical);
    Kolab::KolabObjectWriter::writeJournal(KCalendarCore::Journal::Ptr());
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Critical);
    Kolab::KolabObjectWriter::writeIncidence(KCalendarCore::Event::Ptr());
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Critical);
    Kolab::KolabObjectWriter::writeNote(KMime::Message::Ptr());
    QCOMPARE(Kolab::ErrorHandler::instance().error(), Kolab::ErrorHandler::Critical);
}

void KolabObjectTest::parseRelationMembers()
{
    {
        QString memberString(QStringLiteral(
            "imap:/user/jan.aachen%40lhm.klab.cc/INBOX/"
            "20?message-id=%3Cf06aa3345a25005380b47547ad161d36%40lhm.klab.cc%3E&subject=Re%3A+test&date=Tue%2C+12+Aug+2014+20%3A42%3A59+%2B0200"));
        Kolab::RelationMember member = Kolab::parseMemberUrl(memberString);

        QString result = Kolab::generateMemberUrl(member);
        qDebug() << result;
        result.replace(QLatin1String("%20"), QLatin1String("+"));
        QEXPECT_FAIL("", "This is currently failing, probably a bug in the recent changes regarding the encoding.", Continue);
        QCOMPARE(result, memberString);
    }

    // user namespace by uid
    {
        Kolab::RelationMember member;
        member.uid = 20;
        member.mailbox = QList<QByteArray>() << "INBOX";
        member.user = QStringLiteral("john.doe@example.org");
        member.messageId = QStringLiteral("messageid");
        member.date = QStringLiteral("date");
        member.subject = QStringLiteral("subject");
        QString url = Kolab::generateMemberUrl(member);
        qDebug() << url;
        Kolab::RelationMember result = Kolab::parseMemberUrl(url);
        QCOMPARE(result.uid, member.uid);
        QCOMPARE(result.mailbox, member.mailbox);
        QEXPECT_FAIL("", "This is currently failing, probably a bug in the recent changes regarding the encoding.", Continue);
        QCOMPARE(result.user, member.user);
        QCOMPARE(result.messageId, member.messageId);
        QCOMPARE(result.date, member.date);
        QCOMPARE(result.subject, member.subject);
    }

    // shared namespace by uid
    {
        Kolab::RelationMember member;
        member.uid = 20;
        member.mailbox = QList<QByteArray>() << "foo"
                                             << "bar";
        member.messageId = QStringLiteral("messageid");
        member.date = QStringLiteral("date");
        member.subject = QStringLiteral("subject");
        QString url = Kolab::generateMemberUrl(member);
        qDebug() << url;
        Kolab::RelationMember result = Kolab::parseMemberUrl(url);
        QCOMPARE(result.uid, member.uid);
        QCOMPARE(result.mailbox, member.mailbox);
        QVERIFY(result.user.isEmpty());
        QCOMPARE(result.messageId, member.messageId);
        QCOMPARE(result.date, member.date);
        QCOMPARE(result.subject, member.subject);
    }

    // by uuid/gid
    {
        Kolab::RelationMember member;
        member.gid = QStringLiteral("fooobar");
        QString url = Kolab::generateMemberUrl(member);
        qDebug() << url;
        Kolab::RelationMember result = Kolab::parseMemberUrl(url);
        QCOMPARE(result.gid, member.gid);
    }

    // chars to en/decode
    {
        Kolab::RelationMember member;
        member.uid = 20;
        member.mailbox = QList<QByteArray>() << "spaces in folders"
                                             << "+^,:@";
        member.user = QStringLiteral("john.doe:^@example.org");
        member.messageId = QStringLiteral("messageid+^,:@");
        member.date = QStringLiteral("date+^,:@");
        member.subject = QStringLiteral("subject+^,:@");

        QString url = Kolab::generateMemberUrl(member);
        qDebug() << url;
        Kolab::RelationMember result = Kolab::parseMemberUrl(url);
        QCOMPARE(result.uid, member.uid);
        QEXPECT_FAIL("", "This is currently failing, probably a bug in the recent changes regarding the encoding.", Continue);
        QCOMPARE(result.mailbox, member.mailbox);
        QEXPECT_FAIL("", "This is currently failing, probably a bug in the recent changes regarding the encoding.", Continue);
        QCOMPARE(result.user, member.user);
        QCOMPARE(result.messageId, member.messageId);
        QCOMPARE(result.date, member.date);
        QCOMPARE(result.subject, member.subject);
    }
}

QTEST_MAIN(KolabObjectTest)
