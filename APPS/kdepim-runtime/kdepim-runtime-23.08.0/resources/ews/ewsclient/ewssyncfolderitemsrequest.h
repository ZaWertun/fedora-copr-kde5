/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsitem.h"
#include "ewsitemshape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsSyncFolderItemsRequest : public EwsRequest
{
    Q_OBJECT
public:
    enum ChangeType {
        Create,
        Update,
        Delete,
        ReadFlagChange,
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

        const EwsId &itemId() const
        {
            return mId;
        }

        const EwsItem &item() const
        {
            return mItem;
        }

        bool isRead() const
        {
            return mIsRead;
        }

    protected:
        Change(QXmlStreamReader &reader);
        bool isValid() const
        {
            return mType != Unknown;
        }

        ChangeType mType;
        EwsId mId;
        EwsItem mItem;
        bool mIsRead;

        friend class Response;
    };

    EwsSyncFolderItemsRequest(EwsClient &client, QObject *parent);
    ~EwsSyncFolderItemsRequest() override;

    void setFolderId(const EwsId &id);
    void setItemShape(const EwsItemShape &shape);
    void setSyncState(const QString &state);
    void setMaxChanges(uint max);

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
    EwsItemShape mShape;
    QString mSyncState;
    uint mMaxChanges;
    Change::List mChanges;
    bool mIncludesLastItem;
};

Q_DECLARE_METATYPE(EwsSyncFolderItemsRequest::Change::List)
