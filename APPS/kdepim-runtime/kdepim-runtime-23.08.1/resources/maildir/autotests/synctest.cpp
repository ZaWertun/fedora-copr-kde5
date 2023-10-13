/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "synctest.h"

#include "maildirsettings.h" // generated

#include <QDBusInterface>

#include <QDebug>

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/Control>
#include <Akonadi/ServerManager>
#include <QSignalSpy>
#include <akonadi/qtest_akonadi.h>

#define TIMES 100 // How many times to sync.
#define TIMEOUT 10 // How many seconds to wait before declaring the resource dead.

using namespace Akonadi;

void SyncTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();
    AgentType maildirType = AgentManager::self()->type(QStringLiteral("akonadi_maildir_resource"));
    auto agentCreateJob = new AgentInstanceCreateJob(maildirType);
    QVERIFY(agentCreateJob->exec());
    mMaildirIdentifier = agentCreateJob->instance().identifier();

    QString service = QLatin1String("org.freedesktop.Akonadi.Resource.") + mMaildirIdentifier;
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    }

    OrgKdeAkonadiMaildirSettingsInterface interface(service, QStringLiteral("/"), QDBusConnection::sessionBus());
    QVERIFY(interface.isValid());
    const QString mailPath = QFINDTESTDATA("maildir");
    QVERIFY(!mailPath.isEmpty());
    QVERIFY(QFile::exists(mailPath));
    interface.setPath(mailPath);
}

void SyncTest::testSync()
{
    AgentInstance instance = AgentManager::self()->instance(mMaildirIdentifier);
    QVERIFY(instance.isValid());

    for (int i = 0; i < TIMES; ++i) {
        QDBusInterface interface(Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, mMaildirIdentifier),
                                 QStringLiteral("/"),
                                 QStringLiteral("org.freedesktop.Akonadi.Resource"),
                                 QDBusConnection::sessionBus(),
                                 this);
        QVERIFY(interface.isValid());
        QElapsedTimer t;
        t.start();
        instance.synchronize();
        QSignalSpy spy(&interface, SIGNAL(synchronized()));
        QVERIFY(spy.isValid());
        QVERIFY(spy.wait(TIMEOUT * 1000));
        qDebug() << "Sync attempt" << i << "in" << t.elapsed() << "ms.";
    }
}

QTEST_AKONADIMAIN(SyncTest)
