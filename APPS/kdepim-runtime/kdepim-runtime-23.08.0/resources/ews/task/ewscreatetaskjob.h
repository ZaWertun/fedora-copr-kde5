/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewscreateitemjob.h"

class EwsCreateTaskJob : public EwsCreateItemJob
{
    Q_OBJECT
public:
    EwsCreateTaskJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent);
    ~EwsCreateTaskJob() override;
    bool setSend(bool send = true) override;

protected:
    void doStart() override;
};
