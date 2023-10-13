/*
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pop3test.h"

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Control>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/ServerManager>
#include <KMime/Message>
#include <QElapsedTimer>
#include <akonadi/qtest_akonadi.h>

#include <QStandardPaths>

QTEST_AKONADIMAIN(Pop3Test)

using namespace Akonadi;

constexpr int serverSettleTimeout = 200; /* ms */

void Pop3Test::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();
    QVERIFY(Akonadi::Control::start());

    // switch all resources offline to reduce interference from them
    const auto instances{Akonadi::AgentManager::self()->instances()};
    for (Akonadi::AgentInstance agent : instances) {
        agent.setIsOnline(false);
    }

    /*
    qDebug() << "===========================================================";
    qDebug() << "============ Stopping for debugging =======================";
    qDebug() << "===========================================================";
    kill( qApp->applicationPid(), SIGSTOP );
    */

    //
    // Create the maildir and pop3 resources
    //
    AgentType maildirType = AgentManager::self()->type(QStringLiteral("akonadi_maildir_resource"));
    auto agentCreateJob = new AgentInstanceCreateJob(maildirType);
    const bool maildirCreateSuccess = agentCreateJob->exec();
    if (!maildirCreateSuccess) {
        qWarning() << "Failed to create maildir resource:" << agentCreateJob->errorString();
    }
    QVERIFY(maildirCreateSuccess);
    mMaildirIdentifier = agentCreateJob->instance().identifier();

    AgentType popType = AgentManager::self()->type(QStringLiteral("akonadi_pop3_resource"));
    agentCreateJob = new AgentInstanceCreateJob(popType);
    const bool pop3CreateSuccess = agentCreateJob->exec();
    if (!pop3CreateSuccess) {
        qWarning() << "Failed to create pop3 resource:" << agentCreateJob->errorString();
    }
    QVERIFY(pop3CreateSuccess);
    mPop3Identifier = agentCreateJob->instance().identifier();

    //
    // Configure the maildir resource
    //
    QString maildirRootPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("tester");
    mMaildirPath = maildirRootPath + QLatin1String("/new");
    QDir::current().mkpath(mMaildirPath);
    QDir::current().mkpath(maildirRootPath + QLatin1String("/tmp"));

    QString service = QLatin1String("org.freedesktop.Akonadi.Resource.") + mMaildirIdentifier;
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    }

    mMaildirSettingsInterface = new OrgKdeAkonadiMaildirSettingsInterface(service, QStringLiteral("/Settings"), QDBusConnection::sessionBus(), this);
    QDBusReply<void> setPathReply = mMaildirSettingsInterface->setPath(maildirRootPath);
    QVERIFY(setPathReply.isValid());
    mMaildirSettingsInterface->save();
    AgentManager::self()->instance(mMaildirIdentifier).reconfigure();
    QDBusReply<QString> getPathReply = mMaildirSettingsInterface->path();
    QCOMPARE(getPathReply.value(), maildirRootPath);
    AgentManager::self()->instance(mMaildirIdentifier).synchronize();

    //
    // Find the root maildir collection
    //
    bool found = false;
    QElapsedTimer time;
    time.start();
    while (!found) {
        auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive);
        QVERIFY(job->exec());
        const Collection::List collections = job->collections();
        for (const Collection &col : collections) {
            if (col.resource() == AgentManager::self()->instance(mMaildirIdentifier).identifier() && col.remoteId() == maildirRootPath) {
                mMaildirCollection = col;
                found = true;
                break;
            }
        }

        QVERIFY(time.elapsed() < 10 * 1000); // maildir should not need more than 10 secs to sync
    }

    //
    // Start the fake POP3 server
    //
    mFakeServerThread = new FakeServerThread(this);
    mFakeServerThread->start();
    QTest::qWait(100);
    QVERIFY(mFakeServerThread->server() != nullptr);

    //
    // Configure the pop3 resource
    //
    mPOP3SettingsInterface = new OrgKdeAkonadiPOP3SettingsInterface(Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, mPop3Identifier),
                                                                    QStringLiteral("/Settings"),
                                                                    QDBusConnection::sessionBus(),
                                                                    this);

    QDBusReply<uint> reply0 = mPOP3SettingsInterface->port();
    QVERIFY(reply0.isValid());
    QCOMPARE(reply0.value(), 110u);

    mPOP3SettingsInterface->setPort(5989).waitForFinished();
    mPOP3SettingsInterface->save();
    AgentManager::self()->instance(mPop3Identifier).reconfigure();
    QDBusReply<uint> reply = mPOP3SettingsInterface->port();
    QVERIFY(reply.isValid());
    QCOMPARE(reply.value(), 5989u);

    mPOP3SettingsInterface->setHost(QStringLiteral("localhost")).waitForFinished();
    mPOP3SettingsInterface->save();
    AgentManager::self()->instance(mPop3Identifier).reconfigure();
    QDBusReply<QString> reply2 = mPOP3SettingsInterface->host();
    QVERIFY(reply2.isValid());
    QCOMPARE(reply2.value(), QLatin1String("localhost"));
    mPOP3SettingsInterface->setLogin(QStringLiteral("HansWurst")).waitForFinished();
    mPOP3SettingsInterface->save();
    AgentManager::self()->instance(mPop3Identifier).reconfigure();
    QDBusReply<QString> reply3 = mPOP3SettingsInterface->login();
    QVERIFY(reply3.isValid());
    QCOMPARE(reply3.value(), QLatin1String("HansWurst"));

    mPOP3SettingsInterface->setUnitTestPassword(QStringLiteral("Geheim")).waitForFinished();
    mPOP3SettingsInterface->save();
    AgentManager::self()->instance(mPop3Identifier).reconfigure();
    QDBusReply<QString> reply4 = mPOP3SettingsInterface->unitTestPassword();
    QVERIFY(reply4.isValid());
    QCOMPARE(reply4.value(), QLatin1String("Geheim"));

    mPOP3SettingsInterface->setTargetCollection(mMaildirCollection.id()).waitForFinished();
    mPOP3SettingsInterface->save();
    AgentManager::self()->instance(mPop3Identifier).reconfigure();
    QDBusReply<qlonglong> reply5 = mPOP3SettingsInterface->targetCollection();
    QVERIFY(reply5.isValid());
    QCOMPARE(reply5.value(), mMaildirCollection.id());
}

void Pop3Test::cleanupTestCase()
{
    // test might have failed before thread got created
    if (mFakeServerThread) {
        mFakeServerThread->quit();
        if (!mFakeServerThread->wait(10000)) {
            qWarning() << "The fake server thread has not yet finished, what is wrong!?";
        }
    }
}

static const QByteArray simpleMail1 =
    "From: \"Bill Lumbergh\" <BillLumbergh@initech.com>\r\n"
    "To: \"Peter Gibbons\" <PeterGibbons@initech.com>\r\n"
    "Subject: TPS Reports - New Cover Sheets\r\n"
    "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain\r\n"
    "Date: Mon, 23 Mar 2009 18:04:05 +0300\r\n"
    "\r\n"
    "Hi, Peter. What's happening? We need to talk about your TPS reports.\r\n";

static const QByteArray simpleMail2 =
    "From: \"Amy McCorkell\" <yooper@mtao.net>\r\n"
    "To: gov.palin@yaho.com\r\n"
    "Subject: HI SARAH\r\n"
    "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain\r\n"
    "Date: Mon, 23 Mar 2009 18:04:05 +0300\r\n"
    "\r\n"
    "Hey Sarah,\r\n"
    "bla bla bla bla bla\r\n";

static const QByteArray simpleMail3 =
    "From: chunkylover53@aol.com\r\n"
    "To: tylerdurden@paperstreetsoapcompany.com\r\n"
    "Subject: ILOVEYOU\r\n"
    "MIME-Version: 1.0\r\n"
    "Content-Type: text/plain\r\n"
    "Date: Mon, 23 Mar 2009 18:04:05 +0300\r\n"
    "\r\n"
    "kindly check the attached LOVELETTER coming from me.\r\n";

static const QByteArray simpleMail4 =
    "From: karl@aol.com\r\n"
    "To: lenny@aol.com\r\n"
    "Subject: Who took the donuts?\r\n"
    "\r\n"
    "Hi Lenny, do you know who took all the donuts?\r\n";

static const QByteArray simpleMail5 =
    "From: foo@bar.com\r\n"
    "To: bar@foo.com\r\n"
    "Subject: Hello\r\n"
    "\r\n"
    "Hello World!!\r\n";

void Pop3Test::cleanupMaildir(const Akonadi::Item::List &items)
{
    // Delete all mails so the maildir is clean for the next test
    if (!items.isEmpty()) {
        auto job = new ItemDeleteJob(items);
        QVERIFY(job->exec());
    }

    QElapsedTimer time;
    time.start();
    int lastCount = -1;
    for (;;) {
        QTest::qWait(500);
        QDir maildir(mMaildirPath);
        maildir.refresh();
        const int curCount = maildir.entryList(QDir::Files | QDir::NoDotAndDotDot).count();

        // Restart the timer when a mail arrives, as it shows that the maildir resource is
        // still alive and kicking.
        if (curCount != lastCount) {
            time.restart();
            lastCount = curCount;
        }

        if (curCount == 0) {
            break;
        }

        QVERIFY(time.elapsed() < 60000 || time.elapsed() > 80000000);
    }
}

void Pop3Test::checkMailsInMaildir(const QList<QByteArray> &mails)
{
    // Now, test that all mails actually ended up in the maildir. Since the maildir resource
    // might be slower, give it a timeout so it can write the files to disk
    QElapsedTimer time;
    time.start();
    int lastCount = -1;
    for (;;) {
        QTest::qWait(500);
        QDir maildir(mMaildirPath);
        maildir.refresh();
        const int curCount = maildir.entryList(QDir::Files | QDir::NoDotAndDotDot).count();

        if (curCount == mails.count()) {
            break; // all done
        }

        // Restart the timer when a mail arrives, as it shows that the maildir resource is
        // still alive and kicking.
        if (curCount != lastCount) {
            time.start();
            lastCount = curCount;
        }

        QVERIFY(curCount <= mails.count());
        QVERIFY(time.elapsed() < 60000 || time.elapsed() > 80000000);
    }

    // TODO: check file contents as well or is this overkill?
}

Akonadi::Item::List Pop3Test::checkMailsOnAkonadiServer(const QList<QByteArray> &mails)
{
    // The fake server got disconnected, which means the pop3 resource has entered the QUIT
    // stage. That means all messages should be on the server now, so test that.
    auto job = new ItemFetchJob(mMaildirCollection);
    job->fetchScope().fetchFullPayload();
    const bool ok = job->exec();
    Q_ASSERT(ok);
    if (!ok) {
        return {};
    }
    const Item::List items = job->items();
    Q_ASSERT(mails.size() == items.size());

    QSet<QByteArray> ourMailBodies;
    QSet<QByteArray> itemMailBodies;

    for (const Item &item : items) {
        auto itemMail = item.payload<KMime::Message::Ptr>();
        QByteArray itemMailBody = itemMail->body();

        // For some reason, the body in the maildir has one additional newline.
        // Get rid of this so we can compare them.
        // FIXME: is this a bug? Find out where the newline comes from!
        itemMailBody.chop(1);
        itemMailBodies.insert(itemMailBody);
    }

    for (const QByteArray &mail : mails) {
        KMime::Message::Ptr ourMail(new KMime::Message());
        ourMail->setContent(KMime::CRLFtoLF(mail));
        ourMail->parse();
        QByteArray ourMailBody = ourMail->body();
        ourMailBodies.insert(ourMailBody);
    }

    Q_ASSERT(ourMailBodies == itemMailBodies);
    return items;
}

void Pop3Test::syncAndWaitForFinish()
{
    AgentManager::self()->instance(mPop3Identifier).synchronize();

    // The pop3 resource, ioslave and the fakeserver are all in different processes or threads.
    // We simply wait until the FakeServer got disconnected or until a timeout.
    // Since POP3 fetching can take longer, we reset the timeout timer when the FakeServer
    // does some processing.
    QElapsedTimer time;
    time.start();
    int lastProgress = -1;
    for (;;) {
        qApp->processEvents();

        // Finish correctly when the connection got closed
        if (mFakeServerThread->server()->gotDisconnected()) {
            break;
        }

        // Reset the timeout when the server is working
        const int newProgress = mFakeServerThread->server()->progress();
        if (newProgress != lastProgress) {
            time.restart();
            lastProgress = newProgress;
        }

        // Assert when nothing happens for a certain timeout, that indicates something went
        // wrong and is stuck somewhere
        if (time.elapsed() >= 60000) {
            Q_ASSERT_X(false, "poptest", "FakeServer timed out.");
            break;
        }
    }

    // Once the messages are processed give the Akonadi server and the maildir resource some time to
    // process the item operations. Do this by running a monitor together with a timer. Each captured
    // item operation bumps the timer to wait longer. After 200ms of inactivity the state is considered
    // stable and the test case can proceed.
    Akonadi::Monitor mon(this);
    mon.setResourceMonitored(mPop3Identifier.toLatin1());
    mon.setResourceMonitored(mMaildirIdentifier.toLatin1());
    QEventLoop settleLoop;
    QTimer settleTimer;
    settleTimer.setSingleShot(true);
    connect(&mon, &Akonadi::Monitor::itemAdded, this, [&](const Akonadi::Item &, const Akonadi::Collection &) {
        settleTimer.start(serverSettleTimeout);
    });
    connect(&mon, &Akonadi::Monitor::itemChanged, this, [&](const Akonadi::Item &, const QSet<QByteArray> &) {
        settleTimer.start(serverSettleTimeout);
    });
    connect(&mon, &Akonadi::Monitor::itemRemoved, this, [&](const Akonadi::Item &) {
        settleTimer.start(serverSettleTimeout);
    });

    settleTimer.start(serverSettleTimeout);
    connect(&settleTimer, &QTimer::timeout, this, [&]() {
        settleLoop.exit(0);
    });
    settleLoop.exec();
}

QString Pop3Test::loginSequence() const
{
    return QStringLiteral(
        "C: USER HansWurst\r\n"
        "S: +OK May I have your password, please?\r\n"
        "C: PASS Geheim\r\n"
        "S: +OK Mailbox locked and ready\r\n");
}

QString Pop3Test::retrieveSequence(const QList<QByteArray> &mails, const QList<int> &exceptions) const
{
    QString result;
    for (int i = 1; i <= mails.size(); i++) {
        if (!exceptions.contains(i)) {
            result += QLatin1String(
                "C: RETR %RETR%\r\n"
                "S: +OK Here is your spam\r\n"
                "%MAIL%\r\n"
                ".\r\n");
        }
    }
    return result;
}

QString Pop3Test::deleteSequence(int numToDelete) const
{
    QString result;
    for (int i = 0; i < numToDelete; i++) {
        result += QLatin1String(
            "C: DELE %DELE%\r\n"
            "S: +OK message sent to /dev/null\r\n");
    }
    return result;
}

QString Pop3Test::quitSequence() const
{
    return QStringLiteral(
        "C: QUIT\r\n"
        "S: +OK Have a nice day.\r\n");
}

QString Pop3Test::listSequence(const QList<QByteArray> &mails) const
{
    QString result = QStringLiteral(
        "C: LIST\r\n"
        "S: +OK You got new spam\r\n");
    for (int i = 1; i <= mails.size(); i++) {
        result += QStringLiteral("%1 %MAILSIZE%\r\n").arg(i);
    }
    result += QLatin1String(".\r\n");
    return result;
}

QString Pop3Test::uidSequence(const QStringList &uids) const
{
    QString result = QStringLiteral(
        "C: UIDL\r\n"
        "S: +OK\r\n");
    for (int i = 1; i <= uids.size(); i++) {
        result += QStringLiteral("%1 %2\r\n").arg(i).arg(uids[i - 1]);
    }
    result += QLatin1String(".\r\n");
    return result;
}

static bool sortedEqual(const QStringList &list1, const QStringList &list2)
{
    QStringList sorted1 = list1;
    sorted1.sort();
    QStringList sorted2 = list2;
    sorted2.sort();

    return std::equal(sorted1.begin(), sorted1.end(), sorted2.begin());
}

void Pop3Test::lowerTimeOfSeenMail(const QString &uidOfMail, int secondsToLower)
{
    const int index = mPOP3SettingsInterface->seenUidList().value().indexOf(uidOfMail);
    QList<int> seenTimeList = mPOP3SettingsInterface->seenUidTimeList().value();
    int msgTime = seenTimeList.at(index);
    msgTime -= secondsToLower;
    seenTimeList.replace(index, msgTime);
    mPOP3SettingsInterface->setSeenUidTimeList(seenTimeList).waitForFinished();
}

void Pop3Test::testSimpleDownload()
{
    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    const QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails)
                                                     + deleteSequence(mails.size()) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
}

void Pop3Test::testBigFetch()
{
    QList<QByteArray> mails;
    QStringList uids;
    QString allowedRetrs;
    mails.reserve(1000);
    uids.reserve(1000);
    for (int i = 0; i < 1000; i++) {
        QByteArray newMail = simpleMail1;
        newMail.append(QString::number(i + 1).toLatin1());
        mails << newMail;
        uids << QStringLiteral("UID%1").arg(i + 1);
        allowedRetrs += QString::number(i + 1) + QLatin1Char(',');
    }
    allowedRetrs.chop(1);

    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(allowedRetrs);
    mFakeServerThread->server()->setAllowedDeletions(allowedRetrs);
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails)
                                                     + deleteSequence(mails.size()) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
}

void Pop3Test::testSeenUIDCleanup()
{
    //
    // First, fetch 3 normal mails, but leave them on the server.
    //
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    const QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setAllowedDeletions(QString());
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);

    QVERIFY(sortedEqual(uids, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    //
    // Now, pretend that the messages were removed from the server in the meantime
    // by having no mails on the fake server.
    //
    mFakeServerThread->server()->setMails(QList<QByteArray>());
    mFakeServerThread->server()->setAllowedRetrieves(QString());
    mFakeServerThread->server()->setAllowedDeletions(QString());
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(QList<QByteArray>()) + uidSequence(QStringList()) + quitSequence());
    syncAndWaitForFinish();
    items = checkMailsOnAkonadiServer(QList<QByteArray>());
    checkMailsInMaildir(QList<QByteArray>());
    cleanupMaildir(items);

    QVERIFY(mPOP3SettingsInterface->seenUidList().value().isEmpty());
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
}

void Pop3Test::testSimpleLeaveOnServer()
{
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();

    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    const QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);

    // The resource should have saved the UIDs of the seen messages
    QVERIFY(sortedEqual(uids, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());
    const auto seenUidTimeListValue{mPOP3SettingsInterface->seenUidTimeList().value()};
    for (int seenTime : seenUidTimeListValue) {
        // Those message were just downloaded from the fake server, so they are at maximum
        // 10 minutes old (for slooooow running tests)
        QVERIFY(seenTime >= time(nullptr) - 10 * 60);
    }

    //
    // OK, next mail check: We have to check that the old seen messages are not downloaded again,
    // only new mails.
    //
    QList<QByteArray> newMails(mails);
    newMails << simpleMail4;
    QStringList newUids(uids);
    newUids << QStringLiteral("newUID");
    const QList<int> idsToNotDownload = {1, 2, 3};
    mFakeServerThread->server()->setMails(newMails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("4"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(newMails) + uidSequence(newUids)
                                                         + retrieveSequence(newMails, idsToNotDownload) + quitSequence(),
                                                     idsToNotDownload);

    syncAndWaitForFinish();
    items = checkMailsOnAkonadiServer(newMails);
    checkMailsInMaildir(newMails);
    QVERIFY(sortedEqual(newUids, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    //
    // Ok, next test: When turning off leaving on the server, all mails should be deleted, but
    // none downloaded.
    //
    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();

    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("1,2,3,4"));
    mFakeServerThread->server()->setAllowedRetrieves(QString());
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(newMails) + uidSequence(newUids) + deleteSequence(newMails.size())
                                                     + quitSequence());

    syncAndWaitForFinish();
    items = checkMailsOnAkonadiServer(newMails);
    checkMailsInMaildir(newMails);
    cleanupMaildir(items);
    QVERIFY(mPOP3SettingsInterface->seenUidList().value().isEmpty());
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
}

void Pop3Test::testTimeBasedLeaveRule()
{
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerDays(2).waitForFinished();

    //
    // First download 3 mails and leave them on the server
    //
    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);

    QVERIFY(sortedEqual(uids, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    //
    // Now, modify the seenUidTimeList on the server for UID2 to pretend it
    // was downloaded 3 days ago, which means it should be deleted.
    //
    lowerTimeOfSeenMail(QStringLiteral("UID2"), 60 * 60 * 24 * 3);

    const QList<int> idsToNotDownload = {1, 2, 3};
    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("2"));
    mFakeServerThread->server()->setAllowedRetrieves(QString());
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + deleteSequence(1) + quitSequence(),
                                                     idsToNotDownload);
    syncAndWaitForFinish();
    items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);

    uids.removeAll(QStringLiteral("UID2"));
    QVERIFY(sortedEqual(uids, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());
    const auto seenUidTimeListValue{mPOP3SettingsInterface->seenUidTimeList().value()};
    for (int seenTime : seenUidTimeListValue) {
        QVERIFY(seenTime >= time(nullptr) - 10 * 60);
    }

    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerDays(0).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
}

void Pop3Test::testCountBasedLeaveRule()
{
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerCount(3).waitForFinished();

    //
    // First download 3 mails and leave them on the server
    //
    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    const QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);

    // Make the 3 just downloaded mails appear older than they are
    lowerTimeOfSeenMail(QStringLiteral("UID1"), 60 * 60 * 24 * 2);
    lowerTimeOfSeenMail(QStringLiteral("UID2"), 60 * 60 * 24 * 1);
    lowerTimeOfSeenMail(QStringLiteral("UID3"), 60 * 60 * 24 * 3);

    //
    // Now, download 2 more mails. Since only 3 mails are allowed to be left
    // on the server, the oldest ones, UID1 and UID3, should be deleted
    //
    const QList<QByteArray> moreMails = {simpleMail4, simpleMail5};
    const QStringList moreUids = {QStringLiteral("UID4"), QStringLiteral("UID5")};
    mFakeServerThread->server()->setMails(mails + moreMails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("4,5"));
    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("1,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails + moreMails) + uidSequence(uids + moreUids)
                                                         + retrieveSequence(moreMails) + deleteSequence(2) + quitSequence(),
                                                     {1, 2, 3});

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails + moreMails);
    checkMailsInMaildir(mails + moreMails);
    cleanupMaildir(items);

    const QStringList uidsLeft = {QStringLiteral("UID2"), QStringLiteral("UID4"), QStringLiteral("UID5")};
    QVERIFY(sortedEqual(uidsLeft, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerCount(0).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
}

void Pop3Test::testSizeBasedLeaveRule()
{
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerSize(10).waitForFinished(); // 10 MB

    //
    // First download 3 mails and leave them on the server.
    //
    const QList<QByteArray> mails = {simpleMail1, simpleMail2, simpleMail3};
    const QStringList uids = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QStringLiteral("1,2,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);

    // Make the 3 just downloaded mails appear older than they are
    lowerTimeOfSeenMail(QStringLiteral("UID1"), 60 * 60 * 24 * 2);
    lowerTimeOfSeenMail(QStringLiteral("UID2"), 60 * 60 * 24 * 1);
    lowerTimeOfSeenMail(QStringLiteral("UID3"), 60 * 60 * 24 * 3);

    // Now, do another mail check, but with no new mails on the server.
    // Instead we let the server pretend that the mails have a fake size,
    // each 7 MB. That means the two oldest get deleted, because the total
    // mail size is over 10 MB with them.
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QString());
    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("1,3"));
    mFakeServerThread->server()->setNextConversation(loginSequence()
                                                     + QLatin1String("C: LIST\r\n"
                                                                     "S: +OK You got new spam\r\n"
                                                                     "1 7340032\r\n"
                                                                     "2 7340032\r\n"
                                                                     "3 7340032\r\n"
                                                                     ".\r\n")
                                                     + uidSequence(uids) + deleteSequence(2) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);

    const QStringList uidsLeft = {QStringLiteral("UID2")};
    QVERIFY(sortedEqual(uidsLeft, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerCount(0).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerSize(0).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
}

void Pop3Test::testMixedLeaveRules()
{
    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    //
    // Generate 10 mails
    //
    QList<QByteArray> mails;
    mails.reserve(10);
    QStringList uids;
    uids.reserve(10);
    QString allowedRetrs;
    for (int i = 0; i < 10; i++) {
        QByteArray newMail = simpleMail1;
        newMail.append(QString::number(i + 1).toLatin1());
        mails << newMail;
        uids << QStringLiteral("UID%1").arg(i + 1);
        allowedRetrs += QString::number(i + 1) + QLatin1Char(',');
    }
    allowedRetrs.chop(1);

    //
    // Now, download these 10 mails
    //
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(allowedRetrs);
    mFakeServerThread->server()->setNextConversation(loginSequence() + listSequence(mails) + uidSequence(uids) + retrieveSequence(mails) + quitSequence());

    syncAndWaitForFinish();
    checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);

    // Fake the time of the messages, UID1 is one day old, UID2 is two days old, etc
    for (int i = 1; i <= 10; i++) {
        lowerTimeOfSeenMail(QStringLiteral("UID%1").arg(i), 60 * 60 * 24 * i);
    }

    mPOP3SettingsInterface->setLeaveOnServer(true).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerSize(25).waitForFinished(); // UID 4, 5 oldest here
    mPOP3SettingsInterface->setLeaveOnServerCount(5).waitForFinished(); // UID 6, 7 oldest here
    mPOP3SettingsInterface->setLeaveOnServerDays(7).waitForFinished(); // UID 8, 9 and 10 too old

    // Ok, now we do another mail check that only deletes stuff from the server.
    // Above are the UIDs that should be deleted.
    mFakeServerThread->server()->setMails(mails);
    mFakeServerThread->server()->setAllowedRetrieves(QString());
    mFakeServerThread->server()->setAllowedDeletions(QStringLiteral("4,5,6,7,8,9,10"));
    mFakeServerThread->server()->setNextConversation(loginSequence()
                                                     + QLatin1String("C: LIST\r\n"
                                                                     "S: +OK You got new spam\r\n"
                                                                     "1 7340032\r\n"
                                                                     "2 7340032\r\n"
                                                                     "3 7340032\r\n"
                                                                     "4 7340032\r\n"
                                                                     "5 7340032\r\n"
                                                                     "6 7340032\r\n"
                                                                     "7 7340032\r\n"
                                                                     "8 7340032\r\n"
                                                                     "9 7340032\r\n"
                                                                     "10 7340032\r\n"
                                                                     ".\r\n")
                                                     + uidSequence(uids) + deleteSequence(7) + quitSequence());

    syncAndWaitForFinish();
    Akonadi::Item::List items = checkMailsOnAkonadiServer(mails);
    checkMailsInMaildir(mails);
    cleanupMaildir(items);

    const QStringList uidsLeft = {QStringLiteral("UID1"), QStringLiteral("UID2"), QStringLiteral("UID3")};
    QVERIFY(sortedEqual(uidsLeft, mPOP3SettingsInterface->seenUidList().value()));
    QVERIFY(mPOP3SettingsInterface->seenUidTimeList().value().size() == mPOP3SettingsInterface->seenUidList().value().size());

    mPOP3SettingsInterface->setLeaveOnServer(false).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerCount(0).waitForFinished();
    mPOP3SettingsInterface->setLeaveOnServerSize(0).waitForFinished();
    mPOP3SettingsInterface->setSeenUidTimeList(QList<int>()).waitForFinished();
    mPOP3SettingsInterface->setSeenUidList(QStringList()).waitForFinished();
}
