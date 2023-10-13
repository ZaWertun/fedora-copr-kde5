/*
    SPDX-FileCopyrightText: 2011 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Job>

#include "maildir.h"

class QDirIterator;
namespace Akonadi
{
class TransactionSequence;
}

/**
 * Used to implement ResourceBase::retrieveItems() for Maildirs.
 * This completely bypasses ItemSync in order to achieve maximum performance.
 */
class RetrieveItemsJob : public Akonadi::Job
{
    Q_OBJECT
public:
    RetrieveItemsJob(const Akonadi::Collection &collection, const KPIM::Maildir &md, QObject *parent = nullptr);
    void setMimeType(const QString &mimeType);

protected:
    void doStart() override;

private:
    void entriesProcessed();
    Akonadi::TransactionSequence *transaction();

private Q_SLOTS:
    void localListDone(KJob *job);
    void transactionDone(KJob *job);
    void processEntry();
    void processEntryDone(KJob *);

private:
    const Akonadi::Collection m_collection;
    KPIM::Maildir m_maildir;
    QHash<QString, Akonadi::Item> m_localItems;
    QString m_mimeType;
    Akonadi::TransactionSequence *m_transaction = nullptr;
    int m_transactionSize = 0;
    QDirIterator *m_entryIterator = nullptr;
    qint64 m_previousMtime = 0;
    qint64 m_highestMtime = 0;
    QString m_listingPath;
};
