/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "fakehttppost.h"

#include "ewsunsubscriberequest.h"

class UtEwsUnsibscribeRequest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void simple();

private:
    void verifier(FakeTransferJob *job, const QByteArray &req, const QByteArray &expReq, const QByteArray &resp);

    EwsClient mClient;
};

void UtEwsUnsibscribeRequest::simple()
{
    static const QByteArray request =
        "<?xml version=\"1.0\"?>"
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
        "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
        "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
        "<soap:Header><t:RequestServerVersion Version=\"Exchange2007_SP1\"/></soap:Header>"
        "<soap:Body>"
        "<m:Unsubscribe>"
        "<m:SubscriptionId>dwzVKTlwXxBZtQRMucP5Mg==</m:SubscriptionId>"
        "</m:Unsubscribe></soap:Body></soap:Envelope>\n";
    static const QByteArray response =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "<s:Header><h:ServerVersionInfo xmlns:h=\"http://schemas.microsoft.com/exchange/services/2006/types\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "MajorVersion=\"15\" MinorVersion=\"1\" MajorBuildNumber=\"409\" "
        "MinorBuildNumber=\"24\" Version=\"V2016_01_06\"/>"
        "</s:Header>"
        "<s:Body>"
        "<m:UnsubscribeResponse xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\" "
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">"
        "<m:ResponseMessages>"
        "<m:UnsubscribeResponseMessage ResponseClass=\"Success\">"
        "<m:ResponseCode>NoError</m:ResponseCode>"
        "</m:UnsubscribeResponseMessage>"
        "</m:ResponseMessages>"
        "</m:UnsubscribeResponse>"
        "</s:Body></s:Envelope>";

    FakeTransferJob::addVerifier(this, [this](FakeTransferJob *job, const QByteArray &req) {
        verifier(job, req, request, response);
    });
    QScopedPointer<EwsUnsubscribeRequest> req(new EwsUnsubscribeRequest(mClient, this));
    req->setSubscriptionId(QStringLiteral("dwzVKTlwXxBZtQRMucP5Mg=="));

    req->exec();

    QCOMPARE(req->error(), 0);
}

void UtEwsUnsibscribeRequest::verifier(FakeTransferJob *job, const QByteArray &req, const QByteArray &expReq, const QByteArray &response)
{
    bool fail = true;
    auto f = finally([&fail, &job] {
        if (fail) {
            job->postResponse("");
        }
    });
    QCOMPARE(req, expReq);
    fail = false;
    job->postResponse(response);
}

QTEST_MAIN(UtEwsUnsibscribeRequest)

#include "ewsunsubscriberequest_ut.moc"
