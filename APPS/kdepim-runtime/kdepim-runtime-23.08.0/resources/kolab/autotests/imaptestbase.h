/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <qtest_kde.h>

#include <kimaptest/fakeserver.h>

#include "dummypasswordrequester.h"
#include "dummyresourcestate.h"
#include "imapaccount.h"
#include "resourcetask.h"
#include "sessionpool.h"

Q_DECLARE_METATYPE(ImapAccount *)
Q_DECLARE_METATYPE(DummyPasswordRequester *)
Q_DECLARE_METATYPE(DummyResourceState::Ptr)
Q_DECLARE_METATYPE(KIMAP::Session *)
Q_DECLARE_METATYPE(QVariant)

class ImapTestBase : public QObject
{
    Q_OBJECT

public:
    explicit ImapTestBase(QObject *parent = nullptr);

protected:
    QString defaultUserName() const;
    QString defaultPassword() const;
    ImapAccount *createDefaultAccount() const;
    DummyPasswordRequester *createDefaultRequester();
    QList<QByteArray> defaultAuthScenario() const;
    QList<QByteArray> defaultPoolConnectionScenario(const QList<QByteArray> &customCapabilities = QList<QByteArray>()) const;

    bool waitForSignal(QObject *obj, const char *member, int timeout = 500) const;

    Akonadi::Collection createCollectionChain(const QString &remoteId) const;

private slots:
    void setupTestCase();
};
