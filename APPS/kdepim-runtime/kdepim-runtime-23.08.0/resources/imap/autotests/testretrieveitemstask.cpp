/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

#include "retrieveitemstask.h"
#include "uidnextattribute.h"
#include <highestmodseqattribute.h>
#include <uidvalidityattribute.h>

#include <Akonadi/MessageParts>

#include <Akonadi/CachePolicy>
#include <Akonadi/CollectionStatistics>

#include <QTest>

class TestRetrieveItemsTask : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldIntrospectCollection_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("callNames");

        Akonadi::Collection collection;
        QList<QByteArray> scenario;
        QStringList callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 1:9 (RFC822.SIZE INTERNALDATE "
                    "BODY.PEEK[HEADER] "
                    "FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 7 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[HEADER] {69}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    " )"
                 << "S: A000007 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("first listing, connected IMAP") << collection << scenario << callNames;

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000003 OK [READ-ONLY] select done"
                 << "C: A000004 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000004 OK search done"
                 << "C: A000005 UID FETCH 1:9 (RFC822.SIZE INTERNALDATE "
                    "BODY.PEEK[HEADER] "
                    "FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 7 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[HEADER] {69}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    " )"
                 << "S: A000005 OK fetch done";
        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("retrieval from read-only mailbox (no expunge)") << collection << scenario << callNames;

        Akonadi::CachePolicy policy;
        policy.setLocalParts(QStringList() << QLatin1String(Akonadi::MessagePart::Envelope) << QLatin1String(Akonadi::MessagePart::Header)
                                           << QLatin1String(Akonadi::MessagePart::Body));

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 1:9 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 7 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: A000007 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("first listing, disconnected IMAP") << collection << scenario << callNames;

        Akonadi::CollectionStatistics stats;
        stats.setCount(1);

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.setCachePolicy(policy);
        collection.setStatistics(stats);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 1:9 (FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 7 )"
                 << "S: A000007 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental")
                  << QStringLiteral("itemsRetrievalDone");

        // Disabled test since the flag sync is disabled if CONDSTORE is not supported
        //     QTest::newRow( "second listing, checking for flag changes" ) << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        stats.setCount(1);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 0 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000005 OK select done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("third listing, full sync, empty folder") << collection << scenario << callNames;

        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(8);
        stats.setCount(4);
        collection.setStatistics(stats);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456788);
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 5 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 8:9"
                 << "S: * SEARCH 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 8:9 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 5 FETCH ( FLAGS (\\Seen) UID 9 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: A000007 OK fetch done"
                 << "C: A000008 UID SEARCH UID 1:7"
                 << "S: * SEARCH 1 2 3 4 5 6 7"
                 << "S: A000008 OK search done"
                 << "C: A000009 UID FETCH 1:7 (FLAGS UID)"
                 << "S: * 1 FETCH"
                 << "S: * 2 FETCH"
                 << "S: * 3 FETCH"
                 << "S: * 4 FETCH"
                 << "S: A000009 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental")
                  << QStringLiteral("itemsRetrievalDone");

        // We know no messages have been removed, so we can do an incremental update
        QTest::newRow("uidnext changed, fetch new messages incrementally") << collection << scenario << callNames;

        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(8);
        stats.setCount(5);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 5 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 8:9"
                 << "S: * SEARCH 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 8:9 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 4 FETCH ( FLAGS (\\Seen) UID 8 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: * 5 FETCH ( FLAGS (\\Seen) UID 9 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: A000007 OK fetch done"
                 << "C: A000008 UID SEARCH UID 1:7"
                 << "S: * SEARCH 1 2 3 4 5 6 7"
                 << "S: A000008 OK search done"
                 << "C: A000009 UID FETCH 1:7 (FLAGS UID)"
                 << "S: * 1 FETCH"
                 << "S: * 2 FETCH"
                 << "S: * 3 FETCH"
                 << "S: A000009 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        // A new message has been added and an old one removed, we can't do an incremental update
        QTest::newRow("uidnext changed, fetch new messages and list flags") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456789);
        stats.setCount(5);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario(QList<QByteArray>() << "CONDSTORE") << "C: A000003 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge DONE"
                 << "C: A000005 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 5 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135 ]"
                 << "S: * OK [ UIDNEXT 9 ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done";
        callNames.clear();
        callNames << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("itemsRetrievalDone");

        // No flags have changed
        QTest::newRow("highestmodseq test") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456788);
        stats.setCount(5);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario(QList<QByteArray>() << "CONDSTORE") << "C: A000003 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge DONE"
                 << "C: A000005 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 5 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135 ]"
                 << "S: * OK [ UIDNEXT 9 ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID FETCH 1:9 (FLAGS UID) (CHANGEDSINCE 123456788)"
                 << "S: * 5 FETCH ( UID 8 FLAGS () )"
                 << "S: A000006 OK fetch done";
        callNames.clear();
        callNames << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental")
                  << QStringLiteral("itemsRetrievalDone");

        // fetch only changed flags
        QTest::newRow("changedsince test") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.setCachePolicy(policy);
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456788);
        stats.setCount(5);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario(QList<QByteArray>() << "XYMHIGHESTMODSEQ") << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge DONE"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 5 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135 ]"
                 << "S: * OK [ UIDNEXT 9 ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done";
        // Disabled since the flag sync is disabled if CONDSTORE is not supported
        //              << "C: A000006 UID SEARCH UID 1:9"
        //              << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
        //              << "S: A000006 OK search done"
        //              << "C: A000007 UID FETCH 1:9 (FLAGS UID)"
        //              << "S: * 5 FETCH ( UID 8 FLAGS () )"
        //              << "S: A000007 OK fetch done";
        callNames.clear();

        // Disabled since the flag sync is disabled if CONDSTORE is not supported
        callNames << /*"itemsRetrievedIncremental" << */ QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental")
                  << QStringLiteral("itemsRetrievalDone");

        // Don't rely on yahoos highestmodseq implementation
        QTest::newRow("yahoo highestmodseq test") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(3);
        collection.setCachePolicy(policy);
        stats.setCount(1);
        collection.setStatistics(stats);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 9  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 1:9 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 2321 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: A000007 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("uidvalidity changed") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(105);
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        stats.setCount(104);
        collection.setStatistics(stats);

        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 119 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: * OK [ UIDNEXT 120  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 105:120"
                 // We asked for until 120 but only 119 is available (120 is uidnext)
                 << "S: * SEARCH 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 105:114 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 105 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 // 9 more would follow but are excluded for clarity
                 << "S: A000007 OK fetch done"
                 << "C: A000008 UID FETCH 115:119 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 115 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 // 4 more would follow but are excluded for clarity
                 << "S: A000008 OK fetch done"
                 << "C: A000009 UID SEARCH UID 1:104"
                 << "S: * SEARCH 1 2 99 100"
                 << "S: A000009 OK search done"
                 << "C: A000010 UID FETCH 1:2,99:100 (FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 1 )"
                 // 3 more would follow but are excluded for clarity
                 << "S: A000010 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("itemsRetrievedIncremental")
                  << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievedIncremental") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("test batch processing") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(9);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456789);
        stats.setCount(5);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario(QList<QByteArray>() << "CONDSTORE") << "C: A000003 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge DONE"
                 << "C: A000005 SELECT \"INBOX/Foo\" (CONDSTORE)"
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 4 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135 ]"
                 << "S: * OK [ UIDNEXT 9 ]"
                 << "S: * OK [ HIGHESTMODSEQ 123456789 ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 UID SEARCH UID 1:9"
                 << "S: * SEARCH 1 2 3 4"
                 << "S: A000006 OK search done"
                 << "C: A000007 UID FETCH 1:4 (FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 1 )"
                 << "S: * 2 FETCH ( FLAGS (\\Seen) UID 2 )"
                 << "S: * 3 FETCH ( FLAGS (\\Seen) UID 3 )"
                 << "S: * 4 FETCH ( FLAGS (\\Seen) UID 4 )"
                 << "S: A000007 OK fetch done";
        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        // fetch only changed flags
        QTest::newRow("remote message deleted") << collection << scenario << callNames;

        collection = createCollectionChain(QStringLiteral("/INBOX/Foo"));
        collection.attribute<UidValidityAttribute>(Akonadi::Collection::AddIfMissing)->setUidValidity(1149151135);
        collection.setCachePolicy(policy);
        collection.attribute<UidNextAttribute>(Akonadi::Collection::AddIfMissing)->setUidNext(-1);
        collection.attribute<HighestModSeqAttribute>(Akonadi::Collection::AddIfMissing)->setHighestModSeq(123456789);
        stats.setCount(0);
        collection.setStatistics(stats);
        scenario.clear();
        scenario << defaultPoolConnectionScenario() << "C: A000003 SELECT \"INBOX/Foo\""
                 << "S: A000003 OK select done"
                 << "C: A000004 EXPUNGE"
                 << "S: A000004 OK expunge done"
                 << "C: A000005 SELECT \"INBOX/Foo\""
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [ PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen) ])"
                 << "S: * 1 EXISTS"
                 << "S: * 0 RECENT"
                 << "S: * OK [ UIDVALIDITY 1149151135  ]"
                 << "S: A000005 OK select done"
                 << "C: A000006 STATUS \"INBOX/Foo\" (UIDNEXT)"
                 << "S: * STATUS \"INBOX/Foo\" (UIDNEXT 10)"
                 << "S: A000006 OK status done"
                 << "C: A000007 UID SEARCH UID 1:10"
                 << "S: * SEARCH 1 2 3 4 5 6 7 8 9"
                 << "S: A000007 OK search done"
                 << "C: A000008 UID FETCH 1:9 (RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS (\\Seen) UID 2321 INTERNALDATE \"29-Jun-2010 15:26:42 +0200\" "
                    "RFC822.SIZE 75 BODY[] {75}\r\n"
                    "From: Foo <foo@kde.org>\r\n"
                    "To: Bar <bar@kde.org>\r\n"
                    "Subject: Test Mail\r\n"
                    "\r\n"
                    "Test\r\n"
                    " )"
                 << "S: A000008 OK fetch done";

        callNames.clear();
        callNames << QStringLiteral("itemsRetrieved") << QStringLiteral("applyCollectionChanges") << QStringLiteral("itemsRetrievalDone");

        QTest::newRow("missing uidnext") << collection << scenario << callNames;
    }

    void shouldIntrospectCollection()
    {
        QFETCH(Akonadi::Collection, collection);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, callNames);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(1);

        pool.setPasswordRequester(createDefaultRequester());
        QVERIFY(pool.connect(createDefaultAccount()));
        QVERIFY(waitForSignal(&pool, SIGNAL(connectDone(int, QString))));

        DummyResourceState::Ptr state = DummyResourceState::Ptr(new DummyResourceState);
        state->setServerCapabilities(pool.serverCapabilities());
        state->setCollection(collection);

        auto task = new RetrieveItemsTask(state);
        task->setFetchMissingItemBodies(false);
        task->start(&pool);

        QTRY_COMPARE(state->calls().count(), callNames.size());
        qDebug() << state->calls();
        for (int i = 0; i < callNames.size(); i++) {
            QString command = QString::fromUtf8(state->calls().at(i).first);
            QVariant parameter = state->calls().at(i).second;

            if (command == QLatin1String("cancelTask") && callNames[i] != QLatin1String("cancelTask")) {
                qDebug() << "Got a cancel:" << parameter.toString();
            }

            QCOMPARE(command, callNames[i]);

            if (command == QLatin1String("cancelTask")) {
                QVERIFY(!parameter.toString().isEmpty());
            }
        }

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }
};

QTEST_GUILESS_MAIN(TestRetrieveItemsTask)

#include "testretrieveitemstask.moc"
