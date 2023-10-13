/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QSharedPointer>

#include "ewsfolder.h"
#include "ewsfoldershape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class EwsUpdateFolderRequest : public EwsRequest
{
    Q_OBJECT
public:
    class Update
    {
    public:
        bool write(QXmlStreamWriter &writer, EwsFolderType folderType) const;

    protected:
        enum Type {
            Append = 0,
            Set,
            Delete,
            Unknown,
        };

        Update(const EwsPropertyField &field, const QVariant &val, Type type)
            : mField(field)
            , mValue(val)
            , mType(type)
        {
        }

        EwsPropertyField mField;
        QVariant mValue;
        Type mType;
    };

    class AppendUpdate : public Update
    {
    public:
        AppendUpdate(const EwsPropertyField &field, const QVariant &val)
            : Update(field, val, Append)
        {
        }
    };

    class SetUpdate : public Update
    {
    public:
        SetUpdate(const EwsPropertyField &field, const QVariant &val)
            : Update(field, val, Set)
        {
        }
    };

    class DeleteUpdate : public Update
    {
    public:
        explicit DeleteUpdate(const EwsPropertyField &field)
            : Update(field, QVariant(), Delete)
        {
        }
    };

    class FolderChange
    {
    public:
        FolderChange(const EwsId &folderId, EwsFolderType type)
            : mId(folderId)
            , mType(type)
        {
        }

        void addUpdate(const Update *upd)
        {
            mUpdates.append(QSharedPointer<const Update>(upd));
        }

        bool write(QXmlStreamWriter &writer) const;

    private:
        EwsId mId;
        EwsFolderType mType;
        QList<QSharedPointer<const Update>> mUpdates;
    };

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

        friend class EwsUpdateFolderRequest;
    };

    EwsUpdateFolderRequest(EwsClient &client, QObject *parent);
    ~EwsUpdateFolderRequest() override;

    void addFolderChange(const FolderChange &change)
    {
        mChanges.append(change);
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
    QList<FolderChange> mChanges;
    QList<Response> mResponses;
};
