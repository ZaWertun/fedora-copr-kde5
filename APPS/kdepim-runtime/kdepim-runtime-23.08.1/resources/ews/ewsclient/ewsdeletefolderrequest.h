/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>

#include "ewsfolder.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class QXmlStreamReader;

class EwsDeleteFolderRequest : public EwsRequest
{
    Q_OBJECT
public:
    enum Type {
        HardDelete = 0,
        SoftDelete,
        MoveToDeletedItems,
    };

    class Response : public EwsRequest::Response
    {
    public:
    protected:
        Response(QXmlStreamReader &reader);

        friend class EwsDeleteFolderRequest;
    };

    EwsDeleteFolderRequest(EwsClient &client, QObject *parent);
    ~EwsDeleteFolderRequest() override;

    void setFolderIds(const EwsId::List &ids)
    {
        mIds = ids;
    }

    void setType(Type type)
    {
        mType = type;
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
    EwsId::List mIds;
    Type mType;
    QList<Response> mResponses;
};
