/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsfolder.h"
#include "ewsfoldershape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsFindFolderRequest : public EwsRequest
{
    Q_OBJECT
public:
    explicit EwsFindFolderRequest(EwsClient &client, QObject *parent);
    ~EwsFindFolderRequest() override;

    void setParentFolderId(const EwsId &id);
    void setFolderShape(const EwsFolderShape &shape);
    void setTraversal(EwsTraversalType traversal)
    {
        mTraversal = traversal;
    }

    void start() override;

    const QList<EwsFolder> folders() const
    {
        return mFolders;
    }

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseFoldersResponse(QXmlStreamReader &reader);

private:
    EwsId mParentId;
    EwsFolderShape mShape;
    EwsTraversalType mTraversal;
    QList<EwsFolder> mFolders;
};
