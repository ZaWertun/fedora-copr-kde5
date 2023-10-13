/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcesendjob_p.h"
#include <MailTransport/Transport>

#include <KMime/KMimeMessage>

#include <Akonadi/AddressAttribute>
#include <Akonadi/Collection>
#include <Akonadi/MessageQueueJob>

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;

ResourceSendJob::ResourceSendJob(Transport *transport, QObject *parent)
    : TransportJob(transport, parent)
{
}

ResourceSendJob::~ResourceSendJob() = default;

void ResourceSendJob::doStart()
{
    Message::Ptr msg = Message::Ptr(new Message);
    msg->setContent(data());
    auto job = new MessageQueueJob;
    job->setMessage(msg);
    job->transportAttribute().setTransportId(transport()->id());
    // Default dispatch mode (send now).
    // Move to default sent-mail collection.
    job->addressAttribute().setFrom(sender());
    job->addressAttribute().setTo(to());
    job->addressAttribute().setCc(cc());
    job->addressAttribute().setBcc(bcc());
    addSubjob(job);
    // Once the item is in the outbox, there is nothing more we can do.
    connect(job, &KJob::result, this, &ResourceSendJob::slotEmitResult);
    job->start();
}

void ResourceSendJob::slotEmitResult()
{
    // KCompositeJob took care of the error.
    emitResult();
}

#include "moc_resourcesendjob_p.cpp"
