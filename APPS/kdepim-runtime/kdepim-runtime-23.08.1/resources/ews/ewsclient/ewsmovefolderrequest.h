/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QSharedPointer>

#include "ewsfolder.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class QXmlStreamReader;

class EwsMoveFolderRequest : public EwsRequest
{
    Q_OBJECT
public:
    class Response : public EwsRequest::Response
    {
    public:
        const EwsId &folderId() const
        {
            return mId;
        }

    protected:
        explicit Response(QXmlStreamReader &reader);

        EwsId mId;

        friend class EwsMoveFolderRequest;
    };

    EwsMoveFolderRequest(EwsClient &client, QObject *parent);
    ~EwsMoveFolderRequest() override;

    void setFolderIds(const EwsId::List &ids)
    {
        mIds = ids;
    }

    void setDestinationFolderId(const EwsId &id)
    {
        mDestFolderId = id;
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
    EwsId mDestFolderId;
    QList<Response> mResponses;
};
