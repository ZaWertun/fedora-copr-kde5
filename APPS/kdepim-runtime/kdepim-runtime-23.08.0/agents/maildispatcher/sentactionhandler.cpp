/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sentactionhandler.h"

#include "maildispatcher_debug.h"
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/MessageFlags>

using namespace Akonadi;

SentActionHandler::SentActionHandler(QObject *parent)
    : QObject(parent)
{
}

void SentActionHandler::runAction(const SentActionAttribute::Action &action)
{
    if (action.type() == SentActionAttribute::Action::MarkAsReplied || action.type() == SentActionAttribute::Action::MarkAsForwarded) {
        const Akonadi::Item item(action.value().toLongLong());
        auto job = new Akonadi::ItemFetchJob(item);
        connect(job, &Akonadi::ItemFetchJob::result, this, &SentActionHandler::itemFetchResult);
        job->setProperty("type", static_cast<int>(action.type()));
    }
}

void SentActionHandler::itemFetchResult(KJob *job)
{
    if (job->error()) {
        qCWarning(MAILDISPATCHER_LOG) << job->errorText();
        return;
    }

    auto fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
    if (fetchJob->items().isEmpty()) {
        return;
    }

    Akonadi::Item item = fetchJob->items().at(0);

    const SentActionAttribute::Action::Type type = static_cast<SentActionAttribute::Action::Type>(job->property("type").toInt());
    if (type == SentActionAttribute::Action::MarkAsReplied) {
        item.setFlag(Akonadi::MessageFlags::Replied);
    } else if (type == SentActionAttribute::Action::MarkAsForwarded) {
        item.setFlag(Akonadi::MessageFlags::Forwarded);
    }

    auto modifyJob = new Akonadi::ItemModifyJob(item);
    modifyJob->setIgnorePayload(true);
}
