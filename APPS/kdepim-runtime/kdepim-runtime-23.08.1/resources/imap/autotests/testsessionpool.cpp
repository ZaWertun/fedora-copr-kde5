/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"
#include <KIMAP/CapabilitiesJob>
#include <QSignalSpy>
#include <QTest>
class TestSessionPool : public ImapTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void shouldPrepareFirstSessionOnConnect_data()
    {
        QTest::addColumn<ImapAccount *>("account");
        QTest::addColumn<DummyPasswordRequester *>("requester");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("password");
        QTest::addColumn<int>("errorCode");
        QTest::addColumn<QStringList>("capabilities");

        ImapAccount *account = nullptr;
        DummyPasswordRequester *requester = nullptr;
        QList<QByteArray> scenario;
        QString password;
        QStringList capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 NAMESPACE UIDPLUS IDLE"
                 << "S: A000002 OK Completed"
                 << "C: A000003 NAMESPACE"
                 << R"(S: * NAMESPACE ( ("INBOX/" "/") ) ( ("user/" "/") ) ( ("" "/") ))"
                 << "S: A000003 OK Completed";
        password = QStringLiteral("foobar");
        int errorCode = SessionPool::NoError;
        capabilities.clear();
        capabilities << QStringLiteral("IMAP4") << QStringLiteral("IMAP4REV1") << QStringLiteral("NAMESPACE") << QStringLiteral("UIDPLUS")
                     << QStringLiteral("IDLE");
        QTest::newRow("normal case") << account << requester << scenario << password << errorCode << capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                 << "S: A000002 OK Completed";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::NoError;
        capabilities.clear();
        capabilities << QStringLiteral("IMAP4") << QStringLiteral("IMAP4REV1") << QStringLiteral("UIDPLUS") << QStringLiteral("IDLE");
        QTest::newRow("no NAMESPACE support") << account << requester << scenario << password << errorCode << capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IDLE"
                 << "S: A000002 OK Completed"
                 << "C: A000003 LOGOUT";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::IncompatibleServerError;
        capabilities.clear();
        QTest::newRow("incompatible server") << account << requester << scenario << password << errorCode << capabilities;

        QList<DummyPasswordRequester::RequestType> requests;
        QList<DummyPasswordRequester::ResultType> results;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        requests.clear();
        results.clear();
        requests << DummyPasswordRequester::StandardRequest << DummyPasswordRequester::WrongPasswordRequest;
        results << DummyPasswordRequester::PasswordRetrieved << DummyPasswordRequester::UserRejected;
        requester->setScenario(requests, results);
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 NO Login failed"
                 << "C: A000002 LOGOUT";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::LoginFailError;
        capabilities.clear();
        QTest::newRow("login fail, user reject password entry") << account << requester << scenario << password << errorCode << capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        requests.clear();
        results.clear();
        requests << DummyPasswordRequester::StandardRequest << DummyPasswordRequester::WrongPasswordRequest;
        results << DummyPasswordRequester::PasswordRetrieved << DummyPasswordRequester::PasswordRetrieved;
        requester->setScenario(requests, results);
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 NO Login failed"
                 << R"(C: A000002 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000002 OK Login succeeded"
                 << "C: A000003 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                 << "S: A000003 OK Completed";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::NoError;
        capabilities.clear();
        capabilities << QStringLiteral("IMAP4") << QStringLiteral("IMAP4REV1") << QStringLiteral("UIDPLUS") << QStringLiteral("IDLE");
        QTest::newRow("login fail, user provide new password") << account << requester << scenario << password << errorCode << capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        requests.clear();
        results.clear();
        requests << DummyPasswordRequester::StandardRequest << DummyPasswordRequester::WrongPasswordRequest;
        results << DummyPasswordRequester::PasswordRetrieved << DummyPasswordRequester::EmptyPasswordEntered;
        requester->setScenario(requests, results);
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 NO Login failed"
                 << "C: A000002 LOGOUT";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::LoginFailError;
        capabilities.clear();
        QTest::newRow("login fail, user provided empty password") << account << requester << scenario << password << errorCode << capabilities;

        account = createDefaultAccount();
        requester = createDefaultRequester();
        requests.clear();
        results.clear();
        requests << DummyPasswordRequester::StandardRequest << DummyPasswordRequester::WrongPasswordRequest;
        results << DummyPasswordRequester::PasswordRetrieved << DummyPasswordRequester::ReconnectNeeded;
        requester->setScenario(requests, results);
        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 NO Login failed"
                 << "C: A000002 LOGOUT";
        password = QStringLiteral("foobar");
        errorCode = SessionPool::ReconnectNeededError;
        capabilities.clear();
        QTest::newRow("login fail, user change the settings") << account << requester << scenario << password << errorCode << capabilities;
    }

    void shouldPrepareFirstSessionOnConnect()
    {
        QFETCH(ImapAccount *, account);
        QFETCH(DummyPasswordRequester *, requester);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, password);
        QFETCH(int, errorCode);
        QFETCH(QStringList, capabilities);

        QSignalSpy requesterSpy(requester, &PasswordRequesterInterface::done);

        FakeServer server;
        server.setScenario(scenario);
        server.startAndWait();

        SessionPool pool(2);

        QVERIFY(!pool.isConnected());

        QSignalSpy poolSpy(&pool, &SessionPool::connectDone);

        pool.setPasswordRequester(requester);
        QVERIFY(pool.connect(account));

        QTest::qWait(200);
        QVERIFY(requesterSpy.count() > 0);
        if (requesterSpy.count() == 1) {
            QCOMPARE(requesterSpy.at(0).at(0).toInt(), 0);
            QCOMPARE(requesterSpy.at(0).at(1).toString(), password);
        }

        QCOMPARE(poolSpy.count(), 1);
        QCOMPARE(poolSpy.at(0).at(0).toInt(), errorCode);
        if (errorCode == SessionPool::NoError) {
            QVERIFY(pool.isConnected());
        } else {
            QVERIFY(!pool.isConnected());
        }

        QCOMPARE(pool.serverCapabilities(), capabilities);
        QVERIFY(pool.serverNamespaces().isEmpty());

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldManageSeveralSessions()
    {
        FakeServer server;
        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 NAMESPACE UIDPLUS IDLE"
                                               << "S: A000002 OK Completed"
                                               << "C: A000003 NAMESPACE"
                                               << R"(S: * NAMESPACE ( ("INBOX/" "/") ) ( ("user/" "/") ) ( ("" "/") ))"
                                               << "S: A000003 OK Completed");

        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in");

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        QSignalSpy requesterSpy(requester, &PasswordRequesterInterface::done);

        SessionPool pool(2);
        pool.setPasswordRequester(requester);

        QSignalSpy connectSpy(&pool, &SessionPool::connectDone);
        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);

        // Before connect we can't get any session
        qint64 requestId = pool.requestSession();
        QCOMPARE(requestId, qint64(-1));

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 1);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 0);

        // Requesting a first session shouldn't create a new one,
        // only sessionRequestDone is emitted right away
        requestId = pool.requestSession();
        QCOMPARE(requestId, qint64(1));
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 1);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 1);

        QCOMPARE(sessionSpy.at(0).at(0).toLongLong(), requestId);
        QVERIFY(sessionSpy.at(0).at(1).value<KIMAP::Session *>() != nullptr);
        QCOMPARE(sessionSpy.at(0).at(2).toInt(), 0);
        QCOMPARE(sessionSpy.at(0).at(3).toString(), QString());

        // Requesting an extra session should create a new one
        // So for instance password will be requested
        requestId = pool.requestSession();
        QCOMPARE(requestId, qint64(2));
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 2);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 2);

        QCOMPARE(sessionSpy.at(1).at(0).toLongLong(), requestId);
        QVERIFY(sessionSpy.at(1).at(1).value<KIMAP::Session *>() != nullptr);
        // Should be different sessions...
        QVERIFY(sessionSpy.at(0).at(1).value<KIMAP::Session *>() != sessionSpy.at(1).at(1).value<KIMAP::Session *>());
        QCOMPARE(sessionSpy.at(1).at(2).toInt(), 0);
        QCOMPARE(sessionSpy.at(1).at(3).toString(), QString());

        // Requesting yet another session should fail as we reached the
        // maximum pool size, and they're all reserved
        requestId = pool.requestSession();
        QCOMPARE(requestId, qint64(3));
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 2);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 3);

        QCOMPARE(sessionSpy.at(2).at(0).toLongLong(), requestId);
        QVERIFY(sessionSpy.at(2).at(1).value<KIMAP::Session *>() == nullptr);
        QCOMPARE(sessionSpy.at(2).at(2).toInt(), (int)SessionPool::NoAvailableSessionError);
        QVERIFY(!sessionSpy.at(2).at(3).toString().isEmpty());

        // OTOH, if we release one now, and then request another one
        // it should succeed without even creating a new session
        auto session = sessionSpy.at(0).at(1).value<KIMAP::Session *>();
        pool.releaseSession(session);
        requestId = pool.requestSession();
        QCOMPARE(requestId, qint64(4));
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 2);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 4);

        QCOMPARE(sessionSpy.at(3).at(0).toLongLong(), requestId);
        // Only one session was available, so that should be the one we get gack
        QVERIFY(sessionSpy.at(3).at(1).value<KIMAP::Session *>() == session);
        QCOMPARE(sessionSpy.at(3).at(2).toInt(), 0);
        QCOMPARE(sessionSpy.at(3).at(3).toString(), QString());

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldNotifyConnectionLost()
    {
        FakeServer server;
        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "S: A000002 OK Completed"
                                               << "C: A000003 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "X");

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(1);
        pool.setPasswordRequester(requester);

        QSignalSpy connectSpy(&pool, &SessionPool::connectDone);
        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 0);

        qint64 requestId = pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 1);

        QCOMPARE(sessionSpy.at(0).at(0).toLongLong(), requestId);
        auto s = sessionSpy.at(0).at(1).value<KIMAP::Session *>();

        auto job = new KIMAP::CapabilitiesJob(s);
        job->start();
        QTest::qWait(100);
        QCOMPARE(lostSpy.count(), 1);
        // FIXME extracting the pointer value form QVariant crashes
        // QCOMPARE(lostSpy.at(0).at(0).value<KIMAP::Session *>(), s);

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldNotifyOnDisconnect_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<int>("termination");

        QList<QByteArray> scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                 << "S: A000002 OK Completed"
                 << "C: A000003 LOGOUT";

        QTest::newRow("logout session") << scenario << (int)SessionPool::LogoutSession;

        scenario.clear();
        scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                 << "S: A000001 OK User Logged in"
                 << "C: A000002 CAPABILITY"
                 << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                 << "S: A000002 OK Completed";

        QTest::newRow("close session") << scenario << (int)SessionPool::CloseSession;
    }

    void shouldNotifyOnDisconnect()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(int, termination);

        FakeServer server;
        server.addScenario(scenario);

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(1);
        pool.setPasswordRequester(requester);

        QSignalSpy disconnectSpy(&pool, &SessionPool::disconnectDone);

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);

        QCOMPARE(disconnectSpy.count(), 0);
        pool.disconnect((SessionPool::SessionTermination)termination);
        QTest::qWait(100);
        QCOMPARE(disconnectSpy.count(), 1);

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldCleanupOnClosingDuringLogin_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")";

            QTest::newRow("during login") << scenario;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                     << "S: A000001 OK User Logged in"
                     << "C: A000002 CAPABILITY";

            QTest::newRow("during capability") << scenario;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                     << "S: A000001 OK User Logged in"
                     << "C: A000002 CAPABILITY"
                     << "S: * CAPABILITY IMAP4 IMAP4rev1 NAMESPACE UIDPLUS IDLE"
                     << "S: A000002 OK Completed"
                     << "C: A000003 NAMESPACE";
            QTest::newRow("during namespace") << scenario;
        }
    }

    void shouldCleanupOnClosingDuringLogin()
    {
        QFETCH(QList<QByteArray>, scenario);

        FakeServer server;
        server.addScenario(scenario);

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(1);
        pool.setPasswordRequester(requester);

        QSignalSpy connectSpy(&pool, &SessionPool::connectDone);
        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);
        QCOMPARE(connectSpy.count(), 0); // Login not done yet
        QPointer<KIMAP::Session> session = pool.findChild<KIMAP::Session *>();
        QVERIFY(session.data());
        QCOMPARE(sessionSpy.count(), 0);

        pool.disconnect(SessionPool::CloseSession);

        QTest::qWait(100);
        QCOMPARE(connectSpy.count(), 1); // We're informed that connect failed
        QCOMPARE(connectSpy.at(0).at(0).toInt(), int(SessionPool::CancelledError));
        QCOMPARE(lostSpy.count(), 0); // We're not supposed to know the session pointer, so no connectionLost emitted

        // Make the session->deleteLater work, it can't happen in qWait (nested event loop)
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        QVERIFY(session.isNull());

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldHonorCancelRequest()
    {
        FakeServer server;
        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "S: A000002 OK Completed"
                                               << "C: A000003 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "X");

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(1);
        pool.setPasswordRequester(requester);

        QSignalSpy connectSpy(&pool, &SessionPool::connectDone);
        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);
        QCOMPARE(connectSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 0);

        qint64 requestId = pool.requestSession();

        // Cancel the request
        pool.cancelSessionRequest(requestId);

        // The request should not be processed anymore
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 0);
    }

    void shouldBeDisconnectedOnAllSessionLost()
    {
        FakeServer server;
        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 IDLE"
                                               << "S: A000002 OK Completed"
                                               << "C: A000003 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "X");

        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "X");

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(2);
        pool.setPasswordRequester(requester);

        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);

        // Initial connect should trigger only a password request and a connect
        QVERIFY(pool.connect(account));
        QTest::qWait(100);

        // We should be connected now
        QVERIFY(pool.isConnected());

        // Ask for a session
        pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 1);
        QVERIFY(sessionSpy.at(0).at(1).value<KIMAP::Session *>() != nullptr);

        // Still connected obviously
        QVERIFY(pool.isConnected());

        // Ask for a second session
        pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 2);
        QVERIFY(sessionSpy.at(1).at(1).value<KIMAP::Session *>() != nullptr);

        // Still connected of course
        QVERIFY(pool.isConnected());

        auto session1 = sessionSpy.at(0).at(1).value<KIMAP::Session *>();
        auto session2 = sessionSpy.at(1).at(1).value<KIMAP::Session *>();

        // Prepare for session disconnects
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        // Make the first session drop
        auto job = new KIMAP::CapabilitiesJob(session1);
        job->start();
        QTest::qWait(100);
        QCOMPARE(lostSpy.count(), 1);
        // FIXME extracting the pointer value form QVariant crashes
        // QCOMPARE(lostSpy.at(0).at(0).value<KIMAP::Session *>(), session1);

        // We're still connected (one session being alive)
        QVERIFY(pool.isConnected());

        // Make the second session drop
        job = new KIMAP::CapabilitiesJob(session2);
        job->start();
        QTest::qWait(100);
        QCOMPARE(lostSpy.count(), 2);
        // FIXME extracting the pointer value form QVariant crashes
        // QCOMPARE(lostSpy.at(1).at(0).value<KIMAP::Session *>(), session2);

        // We're not connected anymore! All sessions dropped!
        QVERIFY(!pool.isConnected());

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldHandleDisconnectDuringPasswordRequest()
    {
        ImapAccount *account = createDefaultAccount();

        // This requester will delay the second reply by a second
        DummyPasswordRequester *requester = createDefaultRequester();
        requester->setDelays(QList<int>() << 20 << 1000);
        QSignalSpy requesterSpy(requester, &PasswordRequesterInterface::done);

        FakeServer server;

        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 IDLE"
                                               << "S: A000002 OK Completed"
                                               << "C: A000003 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE"
                                               << "X");

        server.startAndWait();

        // The connect should work nicely
        SessionPool pool(2);

        QVERIFY(!pool.isConnected());

        QSignalSpy poolSpy(&pool, &SessionPool::connectDone);
        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        pool.setPasswordRequester(requester);
        QVERIFY(pool.connect(account));

        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 1);

        QCOMPARE(poolSpy.count(), 1);
        QCOMPARE(poolSpy.at(0).at(0).toInt(), (int)SessionPool::NoError);
        QVERIFY(pool.isConnected());

        // Ask for the session we just created
        pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 1);
        QVERIFY(sessionSpy.at(0).at(1).value<KIMAP::Session *>() != nullptr);

        auto session = sessionSpy.at(0).at(1).value<KIMAP::Session *>();

        // Ask for the second session, the password requested will never reply
        // and we'll get a disconnect in parallel (by triggering the capability
        // job on the first session
        // Done this way to simulate a disconnect during a password request

        // Ask for the extra session, and make sure the call is placed by waiting
        // just a bit (but not too long so that the requester didn't reply yet,
        // we set the reply timeout to 1000 earlier in this test)
        pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(requesterSpy.count(), 1); // Requester didn't reply yet
        QCOMPARE(sessionSpy.count(), 1);

        // Make the first (and only) session drop while we wait for the requester
        // to reply
        auto job = new KIMAP::CapabilitiesJob(session);
        job->start();
        QTest::qWait(100);
        QCOMPARE(lostSpy.count(), 1);
        // FIXME extracting the pointer value form QVariant crashes
        // QCOMPARE(lostSpy.at(0).at(0).value<KIMAP::Session *>(), session);

        // The requester didn't reply yet
        QCOMPARE(requesterSpy.count(), 1);
        QCOMPARE(sessionSpy.count(), 1);

        // Now wait the remaining time to get the session creation to fail
        QTest::qWait(1000);
        QCOMPARE(requesterSpy.count(), 2);
        QCOMPARE(sessionSpy.count(), 2);
        QCOMPARE(sessionSpy.at(1).at(2).toInt(), (int)SessionPool::LoginFailError);

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldHandleDisconnectionDuringSecondLogin_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");

        QTest::newRow("immediate_disconnect") << QList<QByteArray>{};
        QTest::newRow("disconnect_after_greeting") << QList<QByteArray>{FakeServer::greeting()};
        QTest::newRow("disconnect_after_login_command") << QList<QByteArray>{FakeServer::greeting(), R"(C: A000001 LOGIN "test@kdab.com" "foobar")"};
    }

    void shouldHandleDisconnectionDuringSecondLogin()
    {
        QFETCH(QList<QByteArray>, scenario);

        FakeServer server;
        server.addScenario(QList<QByteArray>() << FakeServer::greeting() << R"(C: A000001 LOGIN "test@kdab.com" "foobar")"
                                               << "S: A000001 OK User Logged in"
                                               << "C: A000002 CAPABILITY"
                                               << "S: * CAPABILITY IMAP4 IMAP4rev1 IDLE"
                                               << "S: A000002 OK Completed");

        server.addScenario(scenario << "X");

        server.startAndWait();

        ImapAccount *account = createDefaultAccount();
        DummyPasswordRequester *requester = createDefaultRequester();

        SessionPool pool(2);
        pool.setPasswordRequester(requester);

        QSignalSpy sessionSpy(&pool, &SessionPool::sessionRequestDone);
        QVERIFY(pool.connect(account));

        // We should be connected now
        QTRY_VERIFY(pool.isConnected());

        // Ask for a session
        pool.requestSession();
        QTRY_COMPARE(sessionSpy.count(), 1);
        QVERIFY(sessionSpy.at(0).at(1).value<KIMAP::Session *>() != nullptr);

        // Prepare for session disconnects
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);

        // Ask for a second session, where we'll lose the connection during the Login job.
        pool.requestSession();
        QTest::qWait(100);
        QCOMPARE(sessionSpy.count(), 2);
        QVERIFY(sessionSpy.at(1).at(1).value<KIMAP::Session *>() == nullptr);
        QCOMPARE(lostSpy.count(), 1);

        // The pool itself is still connected
        QVERIFY(pool.isConnected());

        QVERIFY(server.isAllScenarioDone());

        server.quit();
    }

    void shouldNotifyFailureToConnect()
    {
        // This tests what happens when we can't connect to the server, e.g. due to being offline.
        // In this test we just use 0.0.0.0 as an invalid server IP, instead.
        ImapAccount *account = createDefaultAccount();
        account->setServer(QStringLiteral("0.0.0.0")); // so that the connection fails
        DummyPasswordRequester *requester = createDefaultRequester();
        QList<DummyPasswordRequester::RequestType> requests;
        QList<DummyPasswordRequester::ResultType> results;
        // I don't want to see "WrongPasswordRequest". A password popup is annoying when we're offline or the server is down.
        requests << DummyPasswordRequester::StandardRequest;
        results << DummyPasswordRequester::PasswordRetrieved;
        requester->setScenario(requests, results);

        QSignalSpy requesterSpy(requester, &PasswordRequesterInterface::done);
        SessionPool pool(2);
        QSignalSpy connectDoneSpy(&pool, &SessionPool::connectDone);
        QSignalSpy lostSpy(&pool, &SessionPool::connectionLost);
        QVERIFY(!pool.isConnected());
        pool.setPasswordRequester(requester);
        pool.connect(account);
        QVERIFY(!pool.isConnected());
        QTRY_COMPARE(requesterSpy.count(), requests.count());
        QTRY_COMPARE(connectDoneSpy.count(), 1);
        QCOMPARE(connectDoneSpy.at(0).at(0).toInt(), (int)SessionPool::CouldNotConnectError);
        QCOMPARE(lostSpy.count(), 0); // don't want this, it makes the resource reconnect immediately (and fail, and reconnect, and so on...)
    }
};

QTEST_GUILESS_MAIN(TestSessionPool)

#include "testsessionpool.moc"
