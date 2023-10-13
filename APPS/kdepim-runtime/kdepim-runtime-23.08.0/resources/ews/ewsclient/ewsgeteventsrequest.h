/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedPointer>

#include "ewseventrequestbase.h"
#include "ewsid.h"
#include "ewstypes.h"

class EwsGetEventsRequest : public EwsEventRequestBase
{
    Q_OBJECT
public:
    EwsGetEventsRequest(EwsClient &client, QObject *parent);
    ~EwsGetEventsRequest() override;

    void setWatermark(const QString &watermark)
    {
        mWatermark = watermark;
    }

    void start() override;

protected:
    QString mWatermark;
};
