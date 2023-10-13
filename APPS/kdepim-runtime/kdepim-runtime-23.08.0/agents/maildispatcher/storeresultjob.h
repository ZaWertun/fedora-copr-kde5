/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/TransactionSequence>

#include <Akonadi/Item>
#include <QString>
/**
 * This class stores the result of a StoreResultJob in an item.
 * First, it removes the 'queued' flag.
 * After that, if the result was success, it stores the 'sent' flag.
 * If the result was failure, it stores the 'error' flag and an ErrorAttribute.
 */
class StoreResultJob : public Akonadi::TransactionSequence
{
    Q_OBJECT

public:
    /**
     * Creates a new store result job.
     *
     * @param item The item to store.
     * @param success Whether the mail could be dispatched or not.
     * @param message An error message in case the mail could not be dispatched.
     * @param parent The parent object.
     */
    explicit StoreResultJob(const Akonadi::Item &item, bool success, const QString &message, QObject *parent = nullptr);

    /**
     * Destroys the store result job.
     */
    ~StoreResultJob() override;

    Q_REQUIRED_RESULT bool success() const;
    Q_REQUIRED_RESULT QString message() const;

protected:
    // reimpl from TransactionSequence
    void doStart() override;

private:
    // Q_SLOTS:
    void fetchDone(KJob *job);
    void modifyDone(KJob *job);

    Akonadi::Item mItem;
    bool mSuccess = false;
    QString mMessage;
};
