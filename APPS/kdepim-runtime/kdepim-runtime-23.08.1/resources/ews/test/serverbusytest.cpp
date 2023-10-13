/*
    Copyright (C) 2019 Krzysztof Nowicki <krissn@op.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Control>
#include <Akonadi/Monitor>
#include <Akonadi/SpecialCollectionAttribute>
#include <akonadi/qtest_akonadi.h>

#include "ewsresourceinterface.h"
#include "ewssettings.h"
#include "ewswallet.h"
#include "fakeewsserverthread.h"
#include "isolatedtestbase.h"
#include "statemonitor.h"

class BasicTest : public IsolatedTestBase
{
    Q_OBJECT
public:
    explicit BasicTest(QObject *parent = nullptr);
    ~BasicTest() override;
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void busyInit();

private:
    QScopedPointer<TestAgentInstance> mTestInstance;
};

QTEST_AKONADIMAIN(BasicTest)

using namespace Akonadi;

constexpr int DesiredStateTimeoutMs = 200000;

BasicTest::BasicTest(QObject *parent)
    : IsolatedTestBase(parent)
{
}

BasicTest::~BasicTest() = default;

void BasicTest::initTestCase()
{
    init();

    mTestInstance.reset(new TestAgentInstance(QStringLiteral("http://127.0.0.1:%1/EWS/Exchange.asmx").arg(mFakeServerThread->portNumber())));
    QVERIFY(mTestInstance->isValid());
    mTestInstance->resourceInterface().setInitialReconnectTimeout(2);
}

void BasicTest::cleanupTestCase()
{
    cleanup();
}

void BasicTest::busyInit()
{
    bool requestTriggered = false;
    mFakeServerThread->setOverrideReplyCallback([&](const QString &, QXmlResultItems &, const QXmlNamePool &) {
        requestTriggered = true;
        return FakeEwsServer::DialogEntry::HttpResponse(IsolatedTestBase::loadResourceAsString(QStringLiteral(":/xml/errorserverbusy")), 200);
    });

    QEventLoop loop;

    enum { InitialOffline, InitialOnline, BusyOffline, RetryOnline, RetryOffline } instanceState = InitialOffline;

    connect(AgentManager::self(), &AgentManager::instanceOnline, this, [&](const AgentInstance &_instance, bool online) {
        if (_instance.identifier() == mTestInstance->identifier()) {
            switch (instanceState) {
            case InitialOffline:
                QVERIFY(online);
                if (online) {
                    qDebug() << "Initial resource online state reached";
                    instanceState = InitialOnline;
                }
                break;
            case InitialOnline:
                if (!online && requestTriggered) {
                    qDebug() << "Resource state changed to offline after busy response";
                    instanceState = BusyOffline;
                    requestTriggered = false;
                }
                break;
            case BusyOffline:
                QVERIFY(online);
                if (online) {
                    qDebug() << "Resource online after retry";
                    instanceState = RetryOnline;
                }
                break;
            case RetryOnline:
                if (!online) {
                    qDebug() << "Resource state changed to offline after busy response";
                    instanceState = RetryOffline;
                    loop.exit(0);
                }
                break;
            default:
                break;
            }
        }
    });

    QVERIFY(mTestInstance->setOnline(true, true));

    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [&]() {
        qWarning() << "Timeout waiting for desired resource online state.";
        loop.exit(1);
    });
    timer.start(DesiredStateTimeoutMs);
    QCOMPARE(loop.exec(), 0);
}

#include "serverbusytest.moc"
