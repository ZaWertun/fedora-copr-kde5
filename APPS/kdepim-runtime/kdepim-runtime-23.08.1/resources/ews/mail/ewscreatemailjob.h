/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewscreateitemjob.h"

class EwsCreateMailJob : public EwsCreateItemJob
{
    Q_OBJECT
public:
    EwsCreateMailJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent);
    ~EwsCreateMailJob() override;
    bool setSend(bool send = true) override;

protected:
    void doStart() override;
private Q_SLOTS:
    void mailCreateFinished(KJob *job);
    void mailCreateWorkaroundFinished(KJob *job);
    void mailMoveWorkaroundFinished(KJob *job);

private:
    bool mSend = false;
};
