/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsfolder.h"
#include "ewsfoldershape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsSyncFolderHierarchyRequest : public EwsRequest
{
    Q_OBJECT
public:
    enum ChangeType {
        Create,
        Update,
        Delete,
        Unknown,
    };

    class Response;

    class Change
    {
    public:
        typedef QVector<Change> List;

        ChangeType type() const
        {
            return mType;
        }

        const EwsId &folderId() const
        {
            return mId;
        }

        const EwsFolder &folder() const
        {
            return mFolder;
        }

    protected:
        Change(QXmlStreamReader &reader);
        bool isValid() const
        {
            return mType != Unknown;
        }

        ChangeType mType;
        EwsId mId;
        EwsFolder mFolder;

        friend class Response;
    };

    EwsSyncFolderHierarchyRequest(EwsClient &client, QObject *parent);
    ~EwsSyncFolderHierarchyRequest() override;

    void setFolderId(const EwsId &id);
    void setFolderShape(const EwsFolderShape &shape);
    void setSyncState(const QString &state);

    void start() override;

    bool includesLastItem() const
    {
        return mIncludesLastItem;
    }

    const Change::List &changes() const
    {
        return mChanges;
    }

    const QString &syncState() const
    {
        return mSyncState;
    }

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseItemsResponse(QXmlStreamReader &reader);

private:
    EwsId mFolderId;
    EwsFolderShape mShape;
    QString mSyncState;
    Change::List mChanges;
    bool mIncludesLastItem;
};

Q_DECLARE_METATYPE(EwsSyncFolderHierarchyRequest::Change::List)
