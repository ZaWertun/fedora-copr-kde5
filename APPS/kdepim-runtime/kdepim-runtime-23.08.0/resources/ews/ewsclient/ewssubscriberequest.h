/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QSharedPointer>

#include "ewsid.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsSubscribeRequest : public EwsRequest
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
    public:
        const QString &subscriptionId() const
        {
            return mId;
        }

        const QString &watermark() const
        {
            return mWatermark;
        }

    protected:
        Response(QXmlStreamReader &reader);

        QString mId;
        QString mWatermark;

        friend class EwsSubscribeRequest;
    };

    EwsSubscribeRequest(EwsClient &client, QObject *parent);
    ~EwsSubscribeRequest() override;

    void setType(Type t)
    {
        mType = t;
    }

    void setFolderIds(const EwsId::List &folders)
    {
        mFolderIds = folders;
    }

    void setAllFolders(bool allFolders)
    {
        mAllFolders = allFolders;
    }

    void setEventTypes(const QList<EwsEventType> &types)
    {
        mEventTypes = types;
    }

    void setWatermark(const QString &watermark)
    {
        mWatermark = watermark;
    }

    void setTimeout(uint timeout)
    {
        mTimeout = timeout;
    }

    const Response &response() const
    {
        return *mResponse;
    }

    void start() override;

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseSubscribeResponse(QXmlStreamReader &reader);

private:
    // QSharedPointer<EwsSubscription> mSubscription;
    Type mType;
    EwsId::List mFolderIds;
    QList<EwsEventType> mEventTypes;
    bool mAllFolders;
    QString mWatermark;
    uint mTimeout;

    QSharedPointer<Response> mResponse;
};
