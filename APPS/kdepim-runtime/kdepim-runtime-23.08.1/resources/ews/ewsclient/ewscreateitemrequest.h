/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QSharedPointer>

#include "ewsitem.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class QXmlStreamReader;

class EwsCreateItemRequest : public EwsRequest
{
    Q_OBJECT
public:
    class Response : public EwsRequest::Response
    {
    public:
        const EwsId &itemId() const
        {
            return mId;
        }

    protected:
        Response(QXmlStreamReader &reader);

        EwsId mId;

        friend class EwsCreateItemRequest;
    };

    EwsCreateItemRequest(EwsClient &client, QObject *parent);
    ~EwsCreateItemRequest() override;

    void setItems(const EwsItem::List &items)
    {
        mItems = items;
    }

    void setMessageDisposition(EwsMessageDisposition disp)
    {
        mMessageDisp = disp;
    }

    void setMeetingDisposition(EwsMeetingDisposition disp)
    {
        mMeetingDisp = disp;
    }

    void setSavedFolderId(const EwsId &id)
    {
        mSavedFolderId = id;
    }

    void start() override;

    const QList<Response> &responses() const
    {
        return mResponses;
    }

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseItemsResponse(QXmlStreamReader &reader);

private:
    EwsItem::List mItems;
    EwsId mSavedFolderId;
    EwsMessageDisposition mMessageDisp;
    EwsMeetingDisposition mMeetingDisp;
    QList<Response> mResponses;
};
