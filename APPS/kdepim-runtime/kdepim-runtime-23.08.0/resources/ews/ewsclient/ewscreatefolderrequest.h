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

class EwsCreateFolderRequest : public EwsRequest
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
        Response(QXmlStreamReader &reader);

        EwsId mId;

        friend class EwsCreateFolderRequest;
    };

    EwsCreateFolderRequest(EwsClient &client, QObject *parent);
    ~EwsCreateFolderRequest() override;

    void setFolders(const EwsFolder::List &folders)
    {
        mFolders = folders;
    }

    void setParentFolderId(const EwsId &id)
    {
        mParentFolderId = id;
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
    EwsFolder::List mFolders;
    EwsId mParentFolderId;
    QList<Response> mResponses;
};
