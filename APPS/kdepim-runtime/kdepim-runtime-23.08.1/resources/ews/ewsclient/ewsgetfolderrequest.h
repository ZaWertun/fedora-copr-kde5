/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsfolder.h"
#include "ewsfoldershape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsGetFolderRequest : public EwsRequest
{
    Q_OBJECT
public:
    class Response : public EwsRequest::Response
    {
    public:
        explicit Response(QXmlStreamReader &reader);
        bool parseFolders(QXmlStreamReader &reader);

        const EwsFolder &folder() const
        {
            return mFolder;
        }

    private:
        EwsFolder mFolder;
    };

    EwsGetFolderRequest(EwsClient &client, QObject *parent);
    ~EwsGetFolderRequest() override;

    void setFolderIds(const EwsId::List &ids);
    void setFolderShape(const EwsFolderShape &shape);

    void start() override;

    const QList<Response> &responses() const
    {
        return mResponses;
    }

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseFoldersResponse(QXmlStreamReader &reader);

private:
    EwsId::List mIds;
    EwsFolderShape mShape;
    QList<Response> mResponses;
};
