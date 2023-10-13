/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <MailTransport/TransportJob>

#include <Akonadi/Item>

namespace MailTransport
{
/**
  Mail transport job for an Akonadi resource-based transport.

  This is a wrapper job that makes old applications work with resource-based
  transports.  It calls the appropriate methods in MessageQueueJob, and emits
  result() as soon as the item is placed in the outbox, since there is no way
  of monitoring the progress from here.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class ResourceSendJob : public TransportJob
{
    Q_OBJECT
public:
    /**
      Creates an ResourceSendJob.
      @param transport The transport object to use.
      @param parent The parent object.
    */
    explicit ResourceSendJob(Transport *transport, QObject *parent = nullptr);

    /**
      Destroys this job.
    */
    ~ResourceSendJob() override;

protected:
    void doStart() override;

private:
    void slotEmitResult();
};
} // namespace MailTransport
