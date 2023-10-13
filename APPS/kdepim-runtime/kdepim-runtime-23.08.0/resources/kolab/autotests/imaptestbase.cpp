/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imaptestbase.h"

ImapTestBase::ImapTestBase(QObject *parent)
    : QObject(parent)
{
}

QString ImapTestBase::defaultUserName() const
{
    return QLatin1String("test@kdab.com");
}

QString ImapTestBase::defaultPassword() const
{
    return QLatin1String("foobar");
}

ImapAccount *ImapTestBase::createDefaultAccount() const
{
    ImapAccount *account = new ImapAccount;

    account->setServer(QLatin1String("127.0.0.1"));
    account->setPort(5989);
    account->setUserName(defaultUserName());
    account->setSubscriptionEnabled(true);
    account->setEncryptionMode(KIMAP::LoginJob::Unencrypted);
    account->setAuthenticationMode(KIMAP::LoginJob::ClearText);

    return account;
}

DummyPasswordRequester *ImapTestBase::createDefaultRequester()
{
    DummyPasswordRequester *requester = new DummyPasswordRequester(this);
    requester->setPassword(defaultPassword());
    return requester;
}

void ImapTestBase::setupTestCase()
{
    qRegisterMetaType<ImapAccount *>();
    qRegisterMetaType<DummyPasswordRequester *>();
    qRegisterMetaType<DummyResourceState::Ptr>();
    qRegisterMetaType<KIMAP::Session *>();
}

QList<QByteArray> ImapTestBase::defaultAuthScenario() const
{
    QList<QByteArray> scenario;

    scenario << FakeServer::greeting() << "C: A000001 LOGIN \"test@kdab.com\" \"foobar\""
             << "S: A000001 OK User Logged in";

    return scenario;
}

QList<QByteArray> ImapTestBase::defaultPoolConnectionScenario(const QList<QByteArray> &customCapabilities) const
{
    QList<QByteArray> scenario;

    QByteArray caps = "S: * CAPABILITY IMAP4 IMAP4rev1 UIDPLUS IDLE";
    for (const QByteArray &cap : customCapabilities) {
        caps += " " + cap;
    }

    scenario << defaultAuthScenario() << "C: A000002 CAPABILITY" << caps << "S: A000002 OK Completed";

    return scenario;
}

bool ImapTestBase::waitForSignal(QObject *obj, const char *member, int timeout) const
{
    QEventLoop loop;
    QTimer timer;

    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    QSignalSpy spy(obj, member);
    connect(obj, member, &loop, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout);
    loop.exec();
    timer.stop();

    return spy.count() == 1;
}

Akonadi::Collection ImapTestBase::createCollectionChain(const QString &remoteId) const
{
    QChar separator = remoteId.length() > 0 ? remoteId.at(0) : QLatin1Char('/');

    Akonadi::Collection parent(1);
    parent.setRemoteId(QLatin1String("root-id"));
    parent.setParentCollection(Akonadi::Collection::root());
    Akonadi::Entity::Id id = 2;

    Akonadi::Collection collection = parent;

    const QStringList collections = remoteId.split(separator, Qt::SkipEmptyParts);
    for (const QString &colId : collections) {
        collection = Akonadi::Collection(id);
        collection.setRemoteId(separator + colId);
        collection.setParentCollection(parent);

        parent = collection;
        id++;
    }

    return collection;
}
