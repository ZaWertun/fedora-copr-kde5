/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QSharedPointer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <KIO/TransferJob>

#include "ewsclient.h"
#include "ewsjob.h"
#include "ewsserverversion.h"
#include "ewstypes.h"

class QWidget;

class EwsRequest : public EwsJob
{
    Q_OBJECT
public:
    class Response
    {
    public:
        EwsResponseClass responseClass() const
        {
            return mClass;
        }

        bool isSuccess() const
        {
            return mClass == EwsResponseSuccess;
        }

        QString responseCode() const
        {
            return mCode;
        }

        QString responseMessage() const
        {
            return mMessage;
        }

    protected:
        Response(QXmlStreamReader &reader);
        bool readResponseElement(QXmlStreamReader &reader);
        bool setErrorMsg(const QString &msg);

        EwsResponseClass mClass;
        QString mCode;
        QString mMessage;
    };

    EwsRequest(EwsClient &client, QObject *parent);
    ~EwsRequest() override;

    void setMetaData(const KIO::MetaData &md);
    void addMetaData(const QString &key, const QString &value);

    void setServerVersion(const EwsServerVersion &version);
    const EwsServerVersion &serverVersion() const
    {
        return mServerVersion;
    }

    void dump() const;

protected:
    typedef std::function<bool(QXmlStreamReader &reader)> ContentReaderFn;

    void doSend();
    void prepare(const QString &body);
    virtual bool parseResult(QXmlStreamReader &reader) = 0;
    void startSoapDocument(QXmlStreamWriter &writer);
    void endSoapDocument(QXmlStreamWriter &writer);
    bool parseResponseMessage(QXmlStreamReader &reader, const QString &reqName, ContentReaderFn contentReader);
    bool readResponse(QXmlStreamReader &reader);

    KIO::MetaData mMd;
    QString mResponseData;
protected Q_SLOTS:
    void requestResult(KJob *job);
    virtual void requestData(KIO::Job *job, const QByteArray &data);

private:
    bool readSoapBody(QXmlStreamReader &reader);
    bool readSoapFault(QXmlStreamReader &reader);
    bool readHeader(QXmlStreamReader &reader);
    bool readResponseAttr(const QXmlStreamAttributes &attrs, EwsResponseClass &responseClass);
    QString getOAuthToken();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPair<QStringRef, QString> parseNamespacedString(const QString &str, const QXmlStreamNamespaceDeclarations &namespaces);
    EwsResponseCode parseEwsResponseCode(const QPair<QStringRef, QString> &code);
#else
    QPair<QStringView, QString> parseNamespacedString(const QString &str, const QXmlStreamNamespaceDeclarations &namespaces);
    EwsResponseCode parseEwsResponseCode(const QPair<QStringView, QString> &code);
#endif

    QString mBody;
    EwsClient &mClient;
    EwsServerVersion mServerVersion;
    QWidget *mParentWindow;
};
