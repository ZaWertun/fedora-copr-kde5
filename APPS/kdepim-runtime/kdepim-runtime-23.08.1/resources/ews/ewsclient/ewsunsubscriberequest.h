/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedPointer>

#include "ewsid.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsUnsubscribeRequest : public EwsRequest
{
    Q_OBJECT
public:
    enum Type {
        PullSubscription = 0,
        PushSubscription,
        StreamingSubscription,
    };

    class Response : public EwsRequest::Response
    {
    protected:
        explicit Response(QXmlStreamReader &reader);

        friend class EwsUnsubscribeRequest;
    };

    EwsUnsubscribeRequest(EwsClient &client, QObject *parent);
    ~EwsUnsubscribeRequest() override;

    void setSubscriptionId(const QString &id)
    {
        mSubscriptionId = id;
    }

    const Response &response() const
    {
        return *mResponse;
    }

    void start() override;

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseUnsubscribeResponse(QXmlStreamReader &reader);

private:
    QString mSubscriptionId;

    QSharedPointer<Response> mResponse;
};
