/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QList>
#include <QSharedPointer>

#include "ewsid.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class QXmlStreamReader;

class EwsEventRequestBase : public EwsRequest
{
    Q_OBJECT
public:
    class Notification;
    class Response;

    class Event
    {
    public:
        typedef QVector<Event> List;

        EwsEventType type() const
        {
            return mType;
        }

        const QString &watermark() const
        {
            return mWatermark;
        }

        const QDateTime &timestamp() const
        {
            return mTimestamp;
        }

        const EwsId &itemId() const
        {
            return mId;
        }

        const EwsId &parentFolderId() const
        {
            return mParentFolderId;
        }

        uint unreadCount() const
        {
            return mUnreadCount;
        }

        const EwsId &oldItemId() const
        {
            return mOldId;
        }

        const EwsId &oldParentFolderId() const
        {
            return mOldParentFolderId;
        }

        bool itemIsFolder() const
        {
            return mIsFolder;
        }

        bool operator==(const Event &other) const;

    protected:
        Event(QXmlStreamReader &reader);
        bool isValid() const
        {
            return mType != EwsUnknownEvent;
        }

        EwsEventType mType;
        QString mWatermark;
        QDateTime mTimestamp;
        EwsId mId;
        EwsId mParentFolderId;
        uint mUnreadCount = 0;
        EwsId mOldId;
        EwsId mOldParentFolderId;
        bool mIsFolder;

        friend class EwsEventRequestBase::Notification;
    };

    class Notification
    {
    public:
        typedef QList<Notification> List;

        const QString &subscriptionId() const
        {
            return mSubscriptionId;
        }

        const QString &previousWatermark() const
        {
            return mWatermark;
        }

        bool hasMoreEvents() const
        {
            return mMoreEvents;
        }

        const Event::List &events() const
        {
            return mEvents;
        }

        bool operator==(const Notification &other) const;

    protected:
        Notification(QXmlStreamReader &reader);

        bool isValid() const
        {
            return !mSubscriptionId.isNull();
        }

        static bool eventsReader(QXmlStreamReader &reader, QVariant &val);

        QString mSubscriptionId;
        QString mWatermark;
        bool mMoreEvents;
        Event::List mEvents;

        friend class EwsEventRequestBase::Response;
    };

    class Response : public EwsRequest::Response
    {
    public:
        const Notification::List &notifications() const
        {
            return mNotifications;
        }

        bool operator==(const Response &other) const;

    protected:
        Response(QXmlStreamReader &reader);

        Notification::List mNotifications;

        friend class EwsEventRequestBase;
    };

    ~EwsEventRequestBase() override;

    void setSubscriptionId(const QString &id)
    {
        mSubscriptionId = id;
    }

    const QList<Response> &responses() const
    {
        return mResponses;
    }

protected:
    EwsEventRequestBase(EwsClient &client, const QString &reqName, QObject *parent);
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseNotificationsResponse(QXmlStreamReader &reader);

    QString mSubscriptionId;
    QList<Response> mResponses;
    const QString mReqName;
};

Q_DECLARE_METATYPE(EwsEventRequestBase::Event::List)
