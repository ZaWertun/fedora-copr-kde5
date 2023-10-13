/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "storeresultjob.h"

#include "maildispatcher_debug.h"
#include <Akonadi/DispatchModeAttribute>
#include <Akonadi/ErrorAttribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/MessageFlags>
#include <KLocalizedString>

using namespace Akonadi;

StoreResultJob::StoreResultJob(const Item &item, bool success, const QString &message, QObject *parent)
    : TransactionSequence(parent)
    , mItem(item)
    , mSuccess(success)
    , mMessage(message)
{
}

StoreResultJob::~StoreResultJob() = default;

void StoreResultJob::doStart()
{
    // Fetch item in case it was modified elsewhere.
    auto job = new ItemFetchJob(mItem, this);
    connect(job, &ItemFetchJob::result, this, &StoreResultJob::fetchDone);
}

bool StoreResultJob::success() const
{
    return mSuccess;
}

QString StoreResultJob::message() const
{
    return mMessage;
}

void StoreResultJob::fetchDone(KJob *job)
{
    if (job->error()) {
        return;
    }

    qCDebug(MAILDISPATCHER_LOG);

    const ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob *>(job);
    Q_ASSERT(fetchJob);
    if (fetchJob->items().count() != 1) {
        qCCritical(MAILDISPATCHER_LOG) << "Fetched" << fetchJob->items().count() << "items, expected 1.";
        setError(Unknown);
        setErrorText(i18n("Failed to fetch item."));
        commit();
        return;
    }

    // Store result in item.
    Item item = fetchJob->items().at(0);
    if (mSuccess) {
        item.clearFlag(Akonadi::MessageFlags::Queued);
        item.setFlag(Akonadi::MessageFlags::Sent);
        item.setFlag(Akonadi::MessageFlags::Seen);
        item.removeAttribute<ErrorAttribute>();
    } else {
        item.setFlag(Akonadi::MessageFlags::HasError);
        auto errorAttribute = new ErrorAttribute(mMessage);
        item.addAttribute(errorAttribute);

        // If dispatch failed, set the DispatchModeAttribute to Manual.
        // Otherwise, the user will *never* be able to try sending the mail again,
        // as Send Queued Messages will ignore it.
        if (item.hasAttribute<DispatchModeAttribute>()) {
            item.attribute<DispatchModeAttribute>()->setDispatchMode(Akonadi::DispatchModeAttribute::Manual);
        } else {
            item.addAttribute(new DispatchModeAttribute(Akonadi::DispatchModeAttribute::Manual));
        }
    }

    auto modifyJob = new ItemModifyJob(item, this);
    QObject::connect(modifyJob, &ItemModifyJob::result, this, &StoreResultJob::modifyDone);
}

void StoreResultJob::modifyDone(KJob *job)
{
    if (job->error()) {
        return;
    }

    qCDebug(MAILDISPATCHER_LOG);

    commit();
}
