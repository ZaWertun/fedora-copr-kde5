/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsmodifyitemjob.h"

class EwsModifyTaskJob : public EwsModifyItemJob
{
    Q_OBJECT
public:
    EwsModifyTaskJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent);
    ~EwsModifyTaskJob() override;
    void start() override;
};
