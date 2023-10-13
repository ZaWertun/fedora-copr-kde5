/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "auth/ewsoauth.h"
#include <QTest>
#include <QTimer>
#include <functional>

#include "ewsoauth_ut_mock.h"

static const QString testEmail = QStringLiteral("joe.bloggs@unknown.com");
static const QString testClientId = QStringLiteral("b43c59cd-dd1c-41fd-bb9a-b0a1d5696a93");
static const QString testReturnUri = QStringLiteral("urn:ietf:wg:oauth:2.0:oob");
// static const QString testReturnUriPercent = QUrl::toPercentEncoding(testReturnUri);
static const QString testState = QStringLiteral("joidsiuhq");
static const QString resource = QStringLiteral("https://outlook.office365.com/");
// static const QString resourcePercent = QUrl::toPercentEncoding(resource);
static const QString authUrl = QStringLiteral("https://login.microsoftonline.com/common/oauth2/authorize");
static const QString tokenUrl = QStringLiteral("https://login.microsoftonline.com/common/oauth2/token");

static const QString accessToken1 = QStringLiteral("IERbOTo5NSdtY5HMntWTH1wgrRt98KmbF7nNloIdZ4SSYOU7pziJJakpHy8r6kxQi+7T9w36mWv9IWLrvEwTsA");
static const QString refreshToken1 = QStringLiteral("YW7lJFWcEISynbraq4NiLLke3rOieFdvoJEDxpjCXorJblIGM56OJSu1PZXMCQL5W3KLxS9ydxqLHxRTSdw");
static const QString idToken1 = QStringLiteral("gz7l0chu9xIi1MMgPkpHGQTmo3W7L1rQbmWAxEL5VSKHeqdIJ7E3K7vmMYTl/C1fWihB5XiLjD2GSVQoOzTfCw");

class UtEwsOAuth : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initialInteractiveSuccessful();
    void initialRefreshSuccessful();
    void refreshSuccessful();

private:
    static QString formatJsonSorted(const QVariantMap &map);
    static int performAuthAction(EwsOAuth &oAuth, int timeout, std::function<bool(EwsOAuth *)> actionFn);
    static void setUpAccessFunction(const QString &refreshToken);
    static void setUpTokenFunction(const QString &accessToken,
                                   const QString &refreshToken,
                                   const QString &idToken,
                                   quint64 time,
                                   int tokenLifetime,
                                   int extTokenLifetime,
                                   QString &tokenReplyData);
    static void dumpEvents(const QStringList &events, const QStringList &expectedEvents);

    void setUpOAuth(EwsOAuth &oAuth, QStringList &events, const QString &password, const QMap<QString, QString> &map);
};

void UtEwsOAuth::initialInteractiveSuccessful()
{
    EwsOAuth oAuth(nullptr, testEmail, testClientId, testReturnUri);

    QVERIFY(Mock::QWebEngineView::instance);
    QVERIFY(Mock::QOAuth2AuthorizationCodeFlow::instance);

    QStringList events;

    setUpOAuth(oAuth, events, QString(), QMap<QString, QString>());

    Mock::QWebEngineView::instance->setRedirectUri(Mock::QOAuth2AuthorizationCodeFlow::instance->redirectUri());
    auto time = QDateTime::currentSecsSinceEpoch();

    constexpr unsigned int tokenLifetime = 86399;
    constexpr unsigned int extTokenLifetime = 345599;
    QString tokenReplyData;

    setUpAccessFunction(refreshToken1);
    setUpTokenFunction(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, tokenReplyData);
    Mock::QOAuth2AuthorizationCodeFlow::instance->setState(testState);

    const auto initStatus = performAuthAction(oAuth, 1000, [](EwsOAuth *oAuth) {
        oAuth->init();
        return true;
    });
    QVERIFY(initStatus == 1);

    const auto authStatus = performAuthAction(oAuth, 2000, [](EwsOAuth *oAuth) {
        return oAuth->authenticate(true);
    });
    QVERIFY(authStatus == 0);

    const auto authUrlString = Mock::authUrlString(authUrl, testClientId, testReturnUri, testEmail, resource, testState);
    const QStringList expectedEvents = {
        Mock::requestWalletMapString(),
        Mock::modifyParamsAuthString(testClientId, testReturnUri, testState),
        Mock::authorizeWithBrowserString(authUrlString),
        Mock::loadWebPageString(authUrlString),
        Mock::interceptRequestString(authUrlString),
        Mock::interceptRequestBlockedString(false),
        Mock::interceptRequestString(testReturnUri + QStringLiteral("?code=") + QString::fromLatin1(QUrl::toPercentEncoding(refreshToken1))),
        Mock::interceptRequestBlockedString(true),
        Mock::authorizationCallbackReceivedString(refreshToken1),
        Mock::modifyParamsTokenString(testClientId, testReturnUri, refreshToken1),
        Mock::networkReplyFinishedString(tokenReplyData),
        Mock::replyDataCallbackString(tokenReplyData),
        Mock::tokenCallbackString(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, resource)};
    dumpEvents(events, expectedEvents);

    QVERIFY(events == expectedEvents);
}

void UtEwsOAuth::initialRefreshSuccessful()
{
    EwsOAuth oAuth(nullptr, testEmail, testClientId, testReturnUri);

    QVERIFY(Mock::QWebEngineView::instance);
    QVERIFY(Mock::QOAuth2AuthorizationCodeFlow::instance);

    QStringList events;

    QMap<QString, QString> map = {{QStringLiteral("refresh-token"), refreshToken1}};

    setUpOAuth(oAuth, events, QString(), map);

    Mock::QWebEngineView::instance->setRedirectUri(Mock::QOAuth2AuthorizationCodeFlow::instance->redirectUri());
    auto time = QDateTime::currentSecsSinceEpoch();

    constexpr unsigned int tokenLifetime = 86399;
    constexpr unsigned int extTokenLifetime = 345599;
    QString tokenReplyData;

    setUpAccessFunction(refreshToken1);
    setUpTokenFunction(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, tokenReplyData);
    Mock::QOAuth2AuthorizationCodeFlow::instance->setState(testState);

    const auto initStatus = performAuthAction(oAuth, 1000, [](EwsOAuth *oAuth) {
        oAuth->init();
        return true;
    });
    QVERIFY(initStatus == 1);

    const auto authStatus = performAuthAction(oAuth, 2000, [](EwsOAuth *oAuth) {
        return oAuth->authenticate(true);
    });
    QVERIFY(authStatus == 0);

    const auto authUrlString = Mock::authUrlString(authUrl, testClientId, testReturnUri, testEmail, resource, testState);
    const QStringList expectedEvents = {Mock::requestWalletMapString(),
                                        Mock::modifyParamsTokenString(testClientId, testReturnUri, refreshToken1),
                                        Mock::networkReplyFinishedString(tokenReplyData),
                                        Mock::replyDataCallbackString(tokenReplyData),
                                        Mock::tokenCallbackString(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, resource)};
    dumpEvents(events, expectedEvents);

    QVERIFY(events == expectedEvents);
}

void UtEwsOAuth::refreshSuccessful()
{
    EwsOAuth oAuth(nullptr, testEmail, testClientId, testReturnUri);

    QVERIFY(Mock::QWebEngineView::instance);
    QVERIFY(Mock::QOAuth2AuthorizationCodeFlow::instance);

    QStringList events;

    setUpOAuth(oAuth, events, QString(), QMap<QString, QString>());

    Mock::QWebEngineView::instance->setRedirectUri(Mock::QOAuth2AuthorizationCodeFlow::instance->redirectUri());
    auto time = QDateTime::currentSecsSinceEpoch();

    constexpr unsigned int tokenLifetime = 86399;
    constexpr unsigned int extTokenLifetime = 345599;
    QString tokenReplyData;

    setUpAccessFunction(refreshToken1);
    setUpTokenFunction(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, tokenReplyData);
    Mock::QOAuth2AuthorizationCodeFlow::instance->setState(testState);

    const auto initStatus = performAuthAction(oAuth, 1000, [](EwsOAuth *oAuth) {
        oAuth->init();
        return true;
    });
    QVERIFY(initStatus == 1);

    const auto authStatus = performAuthAction(oAuth, 2000, [](EwsOAuth *oAuth) {
        return oAuth->authenticate(true);
    });
    QVERIFY(authStatus == 0);

    const auto authUrlString = Mock::authUrlString(authUrl, testClientId, testReturnUri, testEmail, resource, testState);
    const QStringList expectedEvents = {
        Mock::requestWalletMapString(),
        Mock::modifyParamsAuthString(testClientId, testReturnUri, testState),
        Mock::authorizeWithBrowserString(authUrlString),
        Mock::loadWebPageString(authUrlString),
        Mock::interceptRequestString(authUrlString),
        Mock::interceptRequestBlockedString(false),
        Mock::interceptRequestString(testReturnUri + QStringLiteral("?code=") + QString::fromLatin1(QUrl::toPercentEncoding(refreshToken1))),
        Mock::interceptRequestBlockedString(true),
        Mock::authorizationCallbackReceivedString(refreshToken1),
        Mock::modifyParamsTokenString(testClientId, testReturnUri, refreshToken1),
        Mock::networkReplyFinishedString(tokenReplyData),
        Mock::replyDataCallbackString(tokenReplyData),
        Mock::tokenCallbackString(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, resource)};
    dumpEvents(events, expectedEvents);

    QVERIFY(events == expectedEvents);

    events.clear();

    oAuth.notifyRequestAuthFailed();

    const auto reauthStatus = performAuthAction(oAuth, 2000, [](EwsOAuth *oAuth) {
        return oAuth->authenticate(false);
    });
    QVERIFY(reauthStatus == 0);

    const QStringList expectedEventsRefresh = {
        Mock::modifyParamsTokenString(testClientId, testReturnUri, refreshToken1),
        Mock::networkReplyFinishedString(tokenReplyData),
        Mock::replyDataCallbackString(tokenReplyData),
        Mock::tokenCallbackString(accessToken1, refreshToken1, idToken1, time, tokenLifetime, extTokenLifetime, resource)};
    dumpEvents(events, expectedEvents);

    QVERIFY(events == expectedEventsRefresh);
}

QString UtEwsOAuth::formatJsonSorted(const QVariantMap &map)
{
    QStringList keys = map.keys();
    keys.sort();
    QStringList elems;
    for (const auto &key : std::as_const(keys)) {
        QString val = map[key].toString();
        val.replace(QLatin1Char('"'), QStringLiteral("\\\""));
        elems.append(QStringLiteral("\"%1\":\"%2\"").arg(key, val));
    }
    return QStringLiteral("{") + elems.join(QLatin1Char(',')) + QStringLiteral("}");
}

int UtEwsOAuth::performAuthAction(EwsOAuth &oAuth, int timeout, std::function<bool(EwsOAuth *)> actionFn)
{
    QEventLoop loop;
    int status = -1;
    QTimer timer;
    connect(&oAuth, &EwsOAuth::authSucceeded, &timer, [&]() {
        qDebug() << "succeeded";
        loop.exit(0);
        status = 0;
    });
    connect(&oAuth, &EwsOAuth::authFailed, &timer, [&](const QString &msg) {
        qDebug() << "failed" << msg;
        loop.exit(1);
        status = 1;
    });
    connect(&timer, &QTimer::timeout, &timer, [&]() {
        qDebug() << "timeout";
        loop.exit(1);
        status = 1;
    });
    timer.setSingleShot(true);
    timer.start(timeout);

    if (!actionFn(&oAuth)) {
        return -1;
    }

    if (status == -1) {
        status = loop.exec();
    }

    return status;
}

void UtEwsOAuth::setUpAccessFunction(const QString &refreshToken)
{
    Mock::QWebEngineView::instance->setAuthFunction([&](const QUrl &, QVariantMap &map) {
        map[QStringLiteral("code")] = QUrl::toPercentEncoding(refreshToken);
    });
}

void UtEwsOAuth::setUpTokenFunction(const QString &accessToken,
                                    const QString &refreshToken,
                                    const QString &idToken,
                                    quint64 time,
                                    int tokenLifetime,
                                    int extTokenLifetime,
                                    QString &tokenReplyData)
{
    Mock::QOAuth2AuthorizationCodeFlow::instance->setTokenFunction(
        [=, &tokenReplyData](QString &data, QMap<Mock::QNetworkRequest::KnownHeaders, QVariant> &headers) {
            QVariantMap map;
            map[QStringLiteral("token_type")] = QStringLiteral("Bearer");
            map[QStringLiteral("scope")] = QStringLiteral("ReadWrite.All");
            map[QStringLiteral("expires_in")] = QString::number(tokenLifetime);
            map[QStringLiteral("ext_expires_in")] = QString::number(extTokenLifetime);
            map[QStringLiteral("expires_on")] = QString::number(time + tokenLifetime);
            map[QStringLiteral("not_before")] = QString::number(time);
            map[QStringLiteral("resource")] = resource;
            map[QStringLiteral("access_token")] = accessToken;
            map[QStringLiteral("refresh_token")] = refreshToken;
            map[QStringLiteral("foci")] = QStringLiteral("1");
            map[QStringLiteral("id_token")] = idToken;
            tokenReplyData = formatJsonSorted(map);
            data = tokenReplyData;
            headers[Mock::QNetworkRequest::ContentTypeHeader] = QStringLiteral("application/json; charset=utf-8");

            return Mock::QNetworkReply::NoError;
        });
}

void UtEwsOAuth::dumpEvents(const QStringList &events, const QStringList &expectedEvents)
{
    for (const auto &event : events) {
        qDebug() << "Got event:" << event;
    }
    if (events != expectedEvents) {
        for (const auto &event : expectedEvents) {
            qDebug() << "Expected event:" << event;
        }
    }
}

void UtEwsOAuth::setUpOAuth(EwsOAuth &oAuth, QStringList &events, const QString &password, const QMap<QString, QString> &map)
{
    connect(Mock::QWebEngineView::instance.data(), &Mock::QWebEngineView::logEvent, this, [&events](const QString &event) {
        events.append(event);
    });
    connect(Mock::QOAuth2AuthorizationCodeFlow::instance.data(), &Mock::QOAuth2AuthorizationCodeFlow::logEvent, this, [&events](const QString &event) {
        events.append(event);
    });
    connect(&oAuth, &EwsOAuth::requestWalletPassword, this, [&oAuth, &events, password](bool) {
        events.append(QStringLiteral("RequestWalletPassword"));
        oAuth.walletPasswordRequestFinished(password);
    });
    connect(&oAuth, &EwsOAuth::requestWalletMap, this, [&oAuth, &events, map]() {
        events.append(QStringLiteral("RequestWalletMap"));
        oAuth.walletMapRequestFinished(map);
    });
}

QTEST_MAIN(UtEwsOAuth)

#include "ewsoauth_ut.moc"
