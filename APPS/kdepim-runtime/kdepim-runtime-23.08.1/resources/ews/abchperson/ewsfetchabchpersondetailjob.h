/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsfetchitemdetailjob.h"

class EwsFetchAbchContactDetailsJob : public EwsFetchItemDetailJob
{
    Q_OBJECT
public:
    EwsFetchAbchContactDetailsJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection);
    ~EwsFetchAbchContactDetailsJob() override;

protected:
    void processItems(const EwsGetItemRequest::Response::List &responses) override;
};
