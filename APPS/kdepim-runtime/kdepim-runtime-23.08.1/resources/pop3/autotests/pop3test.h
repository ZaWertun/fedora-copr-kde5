/*
   SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "fakeserver/fakeserver.h"
#include "maildirsettings.h"
#include "pop3settings.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QList>
#include <QObject>

class Pop3Test : public QObject
{
    Q_OBJECT

    void replymMaildirSettingsInterface(QString arg1);
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSimpleDownload();
    void testSimpleLeaveOnServer();
    void testBigFetch();
    void testSeenUIDCleanup();
    void testTimeBasedLeaveRule();
    void testCountBasedLeaveRule();
    void testSizeBasedLeaveRule();
    void testMixedLeaveRules();

private:
    void lowerTimeOfSeenMail(const QString &uidOfMail, int secondsToLower);
    void cleanupMaildir(const Akonadi::Item::List &items);
    void checkMailsInMaildir(const QList<QByteArray> &mails);
    Akonadi::Item::List checkMailsOnAkonadiServer(const QList<QByteArray> &mails);
    void syncAndWaitForFinish();
    QString loginSequence() const;
    QString retrieveSequence(const QList<QByteArray> &mails, const QList<int> &exceptions = QList<int>()) const;
    QString deleteSequence(int numToDelete) const;
    QString quitSequence() const;
    QString listSequence(const QList<QByteArray> &mails) const;
    QString uidSequence(const QStringList &uids) const;

    FakeServerThread *mFakeServerThread = nullptr;

    OrgKdeAkonadiPOP3SettingsInterface *mPOP3SettingsInterface = nullptr;
    OrgKdeAkonadiMaildirSettingsInterface *mMaildirSettingsInterface = nullptr;
    Akonadi::Collection mMaildirCollection;
    QString mPop3Identifier;
    QString mMaildirIdentifier;
    QString mMaildirPath;
};
