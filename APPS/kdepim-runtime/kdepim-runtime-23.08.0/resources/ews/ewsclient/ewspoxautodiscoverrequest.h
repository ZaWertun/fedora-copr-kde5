/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QHash>
#include <QString>
#include <QUrl>

#include "ewsjob.h"
#include "ewsserverversion.h"
#include "ewstypes.h"

namespace KIO
{
class Job;
}

class EwsPoxAutodiscoverRequest : public EwsJob
{
    Q_OBJECT
public:
    enum Action {
        RedirectUrl = 0,
        RedirectAddr,
        Settings,
    };

    enum ProtocolType {
        ExchangeProto,
        ExchangeProxyProto,
        ExchangeWebProto,
        UnknownProto,
    };

    class Protocol
    {
    public:
        Protocol()
            : mType(UnknownProto)
        {
        }

        bool isValid() const
        {
            return mType != UnknownProto;
        }

        ProtocolType type() const
        {
            return mType;
        }

        const QString &ewsUrl() const
        {
            return mEwsUrl;
        }

        const QString &oabUrl() const
        {
            return mOabUrl;
        }

    private:
        ProtocolType mType;
        QString mEwsUrl;
        QString mOabUrl;
        friend class EwsPoxAutodiscoverRequest;
    };

    EwsPoxAutodiscoverRequest(const QUrl &url, const QString &email, const QString &userAgent, bool useNTLMv2, QObject *parent);
    ~EwsPoxAutodiscoverRequest() override;

    const EwsServerVersion &serverVersion() const
    {
        return mServerVersion;
    }

    void dump() const;

    void start() override;

    Action action() const
    {
        return mAction;
    }

    const Protocol protocol(ProtocolType type) const
    {
        return mProtocols.value(type);
    }

    const QString &redirectAddr() const
    {
        return mRedirectAddr;
    }

    const QString &redirectUrl() const
    {
        return mRedirectUrl;
    }

    const QUrl &lastHttpUrl() const
    {
        return mLastUrl;
    }

protected:
    void doSend();
    void prepare(const QString &body);
    bool readResponse(QXmlStreamReader &reader);

protected Q_SLOTS:
    void requestResult(KJob *job);
    void requestData(KIO::Job *job, const QByteArray &data);
    void requestRedirect(KIO::Job *job, const QUrl &url);

private:
    bool readAccount(QXmlStreamReader &reader);
    bool readProtocol(QXmlStreamReader &reader);

    QString mResponseData;
    QString mBody;
    QUrl mUrl;
    QString mEmail;
    QString mUserAgent;
    bool mUseNTLMv2;
    EwsServerVersion mServerVersion;
    Action mAction;
    QString mRedirectUrl;
    QString mRedirectAddr;
    QHash<ProtocolType, Protocol> mProtocols;
    QUrl mLastUrl;
};
