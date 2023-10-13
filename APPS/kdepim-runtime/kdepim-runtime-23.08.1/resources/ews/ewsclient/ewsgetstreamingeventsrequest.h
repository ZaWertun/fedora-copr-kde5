/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedPointer>
#include <QTimer>

#include "ewseventrequestbase.h"
#include "ewsid.h"
#include "ewstypes.h"

class EwsGetStreamingEventsRequest : public EwsEventRequestBase
{
    Q_OBJECT
public:
    EwsGetStreamingEventsRequest(EwsClient &client, QObject *parent);
    ~EwsGetStreamingEventsRequest() override;

    void setTimeout(uint timeout)
    {
        mTimeout = timeout;
    }

    void start() override;
public Q_SLOTS:
    void eventsProcessed(const EwsEventRequestBase::Response &response);
Q_SIGNALS:
    void eventsReceived(KJob *job);
protected Q_SLOTS:
    void requestData(KIO::Job *job, const QByteArray &data) override;
    void requestDataTimeout();

protected:
    uint mTimeout;
    QTimer mRespTimer;
};
