/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsmodifyitemjob.h"

class EwsModifyAbchPersonJob : public EwsModifyItemJob
{
    Q_OBJECT
public:
    EwsModifyAbchPersonJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent);
    ~EwsModifyAbchPersonJob() override;
    void start() override;
};
