/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsoauth_ut_mock.h"

#include <QUrlQuery>
Q_LOGGING_CATEGORY(EWSCLI_LOG, "org.kde.pim.ews.client", QtInfoMsg)

namespace Mock
{
QPointer<QWebEngineView> QWebEngineView::instance;
QPointer<QOAuth2AuthorizationCodeFlow> QOAuth2AuthorizationCodeFlow::instance;

QUrl QWebEngineUrlRequestJob::requestUrl() const
{
    return mUrl;
}

QUrl QWebEngineUrlRequestInfo::requestUrl() const
{
    return mUrl;
}

void QWebEngineUrlRequestInfo::block(bool)
{
    mBlocked = true;
}

void QWebEngineUrlRequestInfo::redirect(const QUrl &)
{
}

void QWebEngineUrlRequestInfo::setHttpHeader(const QByteArray &header, const QByteArray &value)
{
}

QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor(QObject *parent)
    : QObject(parent)
{
}

QWebEngineUrlRequestInterceptor::~QWebEngineUrlRequestInterceptor() = default;

QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(QObject *parent)
    : QObject(parent)
{
}

QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler() = default;

QWebEngineProfile::QWebEngineProfile(QObject *parent)
    : QObject(parent)
    , mInterceptor(nullptr)
    , mHandler(nullptr)
{
}

QWebEngineProfile::~QWebEngineProfile() = default;

void QWebEngineProfile::setHttpUserAgent(const QString &ua)
{
    mUserAgent = ua;
}

void QWebEngineProfile::setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    mInterceptor = interceptor;
}

void QWebEngineProfile::installUrlSchemeHandler(QByteArray const &scheme, QWebEngineUrlSchemeHandler *handler)
{
    mScheme = QString::fromLatin1(scheme);
    mHandler = handler;
}

QWebEnginePage::QWebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : QObject(parent)
    , mProfile(profile)
{
    connect(profile, &QWebEngineProfile::logEvent, this, &QWebEnginePage::logEvent);
}

QWebEnginePage::~QWebEnginePage() = default;

QWebEngineView::QWebEngineView(QWidget *parent)
    : QWidget(parent)
    , mPage(nullptr)
{
    if (!instance) {
        instance = this;
    } else {
        qDebug() << "QWebEngineView instance already exists!";
    }
}

QWebEngineView::~QWebEngineView() = default;

void QWebEngineView::load(const QUrl &url)
{
    Q_EMIT logEvent(QStringLiteral("LoadWebPage:") + url.toString());

    simulatePageLoad(url);

    QVariantMap params;
    if (mAuthFunction) {
        mAuthFunction(url, params);

        simulatePageLoad(QUrl(mRedirectUri + QStringLiteral("?") + QOAuth2AuthorizationCodeFlow::mapToSortedQuery(params).toString()));
    } else {
        qWarning() << "No authentication callback defined";
    }
}

void QWebEngineView::simulatePageLoad(const QUrl &url)
{
    if (mPage && mPage->mProfile && mPage->mProfile->mInterceptor) {
        QWebEngineUrlRequestInfo info(url, this);
        Q_EMIT logEvent(QStringLiteral("InterceptRequest:") + url.toString());
        mPage->mProfile->mInterceptor->interceptRequest(info);
        Q_EMIT logEvent(QStringLiteral("InterceptRequestBlocked:%1").arg(info.mBlocked));
    } else {
        qWarning() << "Cannot reach to request interceptor";
    }
}

void QWebEngineView::setPage(QWebEnginePage *page)
{
    mPage = page;

    connect(page, &QWebEnginePage::logEvent, this, &QWebEngineView::logEvent);
}

void QWebEngineView::stop()
{
}

void QWebEngineView::setAuthFunction(const AuthFunc &func)
{
    mAuthFunction = func;
}

void QWebEngineView::setRedirectUri(const QString &uri)
{
    mRedirectUri = uri;
}

QNetworkReply::NetworkError QNetworkReply::error() const
{
    return NoError;
}

QVariant QNetworkReply::header(QNetworkRequest::KnownHeaders header) const
{
    return mHeaders[header];
}

QAbstractOAuthReplyHandler::QAbstractOAuthReplyHandler(QObject *parent)
    : QObject(parent)
{
}

QAbstractOAuthReplyHandler::~QAbstractOAuthReplyHandler() = default;

QAbstractOAuth::QAbstractOAuth(QObject *parent)
    : QObject(parent)
    , mStatus(Status::NotAuthenticated)
{
}

void QAbstractOAuth::setReplyHandler(QAbstractOAuthReplyHandler *handler)
{
    mReplyHandler = handler;
}

void QAbstractOAuth::setAuthorizationUrl(const QUrl &url)
{
    mAuthUrl = url;
}

void QAbstractOAuth::setClientIdentifier(const QString &identifier)
{
    mClientId = identifier;
}

void QAbstractOAuth::setModifyParametersFunction(const std::function<void(QAbstractOAuth::Stage, QMap<QString, QVariant> *)> &func)
{
    mModifyParamsFunc = func;
}

QString QAbstractOAuth::token() const
{
    return mToken;
}

void QAbstractOAuth::setToken(const QString &token)
{
    mToken = token;
}

QAbstractOAuth::Status QAbstractOAuth::status() const
{
    return mStatus;
}

QAbstractOAuth2::QAbstractOAuth2(QObject *parent)
    : QAbstractOAuth(parent)
{
}

QString QAbstractOAuth2::refreshToken() const
{
    return mRefreshToken;
}

void QAbstractOAuth2::setRefreshToken(const QString &token)
{
    mRefreshToken = token;
}

QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(QObject *parent)
    : QAbstractOAuth2(parent)
{
    if (!instance) {
        instance = this;
    } else {
        qDebug() << "QOAuth2AuthorizationCodeFlow instance already exists!";
    }
}

QOAuth2AuthorizationCodeFlow::~QOAuth2AuthorizationCodeFlow() = default;

void QOAuth2AuthorizationCodeFlow::setAccessTokenUrl(const QUrl &url)
{
    mTokenUrl = url;
}

void QOAuth2AuthorizationCodeFlow::grant()
{
    QMap<QString, QVariant> map;
    map[QStringLiteral("response_type")] = QStringLiteral("code");
    map[QStringLiteral("client_id")] = QUrl::toPercentEncoding(mClientId);
    map[QStringLiteral("redirect_uri")] = QUrl::toPercentEncoding(mReplyHandler->callback());
    map[QStringLiteral("scope")] = QString();
    map[QStringLiteral("state")] = mState;

    Q_EMIT logEvent(QStringLiteral("ModifyParams:RequestingAuthorization:") + mapToSortedQuery(map).toString());

    if (mModifyParamsFunc) {
        mModifyParamsFunc(Stage::RequestingAuthorization, &map);
    }

    mResource = QUrl::fromPercentEncoding(map[QStringLiteral("resource")].toByteArray());

    QUrl url(mAuthUrl);
    url.setQuery(mapToSortedQuery(map));

    Q_EMIT logEvent(QStringLiteral("AuthorizeWithBrowser:") + url.toString());

    connect(this, &QAbstractOAuth2::authorizationCallbackReceived, this, &QOAuth2AuthorizationCodeFlow::authCallbackReceived, Qt::UniqueConnection);

    Q_EMIT authorizeWithBrowser(url);
}

void QOAuth2AuthorizationCodeFlow::refreshAccessToken()
{
    mStatus = Status::RefreshingToken;

    doRefreshAccessToken();
}

void QOAuth2AuthorizationCodeFlow::doRefreshAccessToken()
{
    QMap<QString, QVariant> map;
    map[QStringLiteral("grant_type")] = QStringLiteral("authorization_code");
    map[QStringLiteral("code")] = QUrl::toPercentEncoding(mRefreshToken);
    map[QStringLiteral("client_id")] = QUrl::toPercentEncoding(mClientId);
    map[QStringLiteral("redirect_uri")] = QUrl::toPercentEncoding(mReplyHandler->callback());

    Q_EMIT logEvent(QStringLiteral("ModifyParams:RequestingAccessToken:") + mapToSortedQuery(map).toString());

    if (mModifyParamsFunc) {
        mModifyParamsFunc(Stage::RequestingAccessToken, &map);
    }

    connect(mReplyHandler, &QAbstractOAuthReplyHandler::tokensReceived, this, &QOAuth2AuthorizationCodeFlow::tokenCallbackReceived, Qt::UniqueConnection);
    connect(mReplyHandler,
            &QAbstractOAuthReplyHandler::replyDataReceived,
            this,
            &QOAuth2AuthorizationCodeFlow::replyDataCallbackReceived,
            Qt::UniqueConnection);

    if (mTokenFunc) {
        QNetworkReply reply(this);

        QString data;
        reply.mError = mTokenFunc(data, reply.mHeaders);

        reply.setData(data.toUtf8());
        reply.open(QIODevice::ReadOnly);

        Q_EMIT logEvent(QStringLiteral("NetworkReplyFinished:") + data);

        mReplyHandler->networkReplyFinished(&reply);
    } else {
        qWarning() << "No token function defined";
    }
}

QUrlQuery QOAuth2AuthorizationCodeFlow::mapToSortedQuery(QMap<QString, QVariant> const &map)
{
    QUrlQuery query;
    QStringList keys = map.keys();
    keys.sort();
    for (const auto &key : std::as_const(keys)) {
        query.addQueryItem(key, map[key].toString());
    }
    return query;
}

void QOAuth2AuthorizationCodeFlow::authCallbackReceived(QMap<QString, QVariant> const &params)
{
    Q_EMIT logEvent(QStringLiteral("AuthorizatioCallbackReceived:") + mapToSortedQuery(params).toString());

    mRefreshToken = params[QStringLiteral("code")].toString();
    if (!mRefreshToken.isEmpty()) {
        mStatus = Status::TemporaryCredentialsReceived;
        doRefreshAccessToken();
    } else {
        Q_EMIT error(QString(), QString(), QUrl());
    }
}

void QOAuth2AuthorizationCodeFlow::tokenCallbackReceived(const QVariantMap &tokens)
{
    Q_EMIT logEvent(QStringLiteral("TokenCallback:") + mapToSortedQuery(tokens).toString());

    mToken = tokens[QStringLiteral("access_token")].toString();
    mRefreshToken = tokens[QStringLiteral("refresh_token")].toString();

    mStatus = Status::Granted;

    Q_EMIT granted();
}

void QOAuth2AuthorizationCodeFlow::replyDataCallbackReceived(const QByteArray &data)
{
    Q_EMIT logEvent(QStringLiteral("ReplyDataCallback:") + QString::fromLatin1(data));
}

QString QOAuth2AuthorizationCodeFlow::redirectUri() const
{
    return mReplyHandler->callback();
}

void QOAuth2AuthorizationCodeFlow::setTokenFunction(const TokenFunc &func)
{
    mTokenFunc = func;
}

void QOAuth2AuthorizationCodeFlow::setState(const QString &state)
{
    mState = state;
}

QString browserDisplayRequestString()
{
    return QStringLiteral("BrowserDisplayRequest");
}

QString modifyParamsAuthString(const QString &clientId, const QString &returnUri, const QString &state)
{
    return QStringLiteral("ModifyParams:RequestingAuthorization:client_id=%1&redirect_uri=%2&response_type=code&scope&state=%3")
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(clientId)),
             QString::fromLatin1(QUrl::toPercentEncoding(returnUri)),
             QString::fromLatin1(QUrl::toPercentEncoding(state)));
}

QString
authUrlString(const QString &authUrl, const QString &clientId, const QString &returnUri, const QString &email, const QString &resource, const QString &state)
{
    return QStringLiteral("%1?client_id=%2&login_hint=%3&prompt=login&redirect_uri=%4&resource=%5&response_type=code&scope&state=%6")
        .arg(authUrl,
             QString::fromLatin1(QUrl::toPercentEncoding(clientId)),
             email,
             QString::fromLatin1(QUrl::toPercentEncoding(returnUri)),
             QString::fromLatin1(QUrl::toPercentEncoding(resource)),
             QString::fromLatin1(QUrl::toPercentEncoding(state)));
}

QString authorizeWithBrowserString(const QString &url)
{
    return QStringLiteral("AuthorizeWithBrowser:") + url;
}

QString loadWebPageString(const QString &url)
{
    return QStringLiteral("LoadWebPage:") + url;
}

QString interceptRequestString(const QString &url)
{
    return QStringLiteral("InterceptRequest:") + url;
}

QString interceptRequestBlockedString(bool blocked)
{
    return QStringLiteral("InterceptRequestBlocked:%1").arg(blocked);
}

QString authorizationCallbackReceivedString(const QString &code)
{
    return QStringLiteral("AuthorizatioCallbackReceived:code=%1").arg(code);
}

QString modifyParamsTokenString(const QString &clientId, const QString &returnUri, const QString &code)
{
    return QStringLiteral("ModifyParams:RequestingAccessToken:client_id=%1&code=%2&grant_type=authorization_code&redirect_uri=%3")
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(clientId)),
             QString::fromLatin1(QUrl::toPercentEncoding(code)),
             QString::fromLatin1(QUrl::toPercentEncoding(returnUri)));
}

QString networkReplyFinishedString(const QString &data)
{
    return QStringLiteral("NetworkReplyFinished:") + data;
}

QString replyDataCallbackString(const QString &data)
{
    return QStringLiteral("ReplyDataCallback:") + data;
}

QString tokenCallbackString(const QString &accessToken,
                            const QString &refreshToken,
                            const QString &idToken,
                            quint64 time,
                            unsigned int tokenLifetime,
                            unsigned int extTokenLifetime,
                            const QString &resource)
{
    return QStringLiteral(
               "TokenCallback:access_token=%1&expires_in=%2&expires_on=%3&ext_expires_in=%4&foci=1&id_token=%5&not_before=%6&refresh_token=%7&resource=%8&"
               "scope=ReadWrite.All&token_type=Bearer")
        .arg(accessToken)
        .arg(tokenLifetime)
        .arg(time + tokenLifetime)
        .arg(extTokenLifetime)
        .arg(idToken)
        .arg(time)
        .arg(refreshToken, resource);
}

QString requestWalletMapString()
{
    return QStringLiteral("RequestWalletMap");
}

const QString &KJob::errorString() const
{
    static const QString empty;
    return empty;
}

EwsPKeyAuthJob::EwsPKeyAuthJob(const QUrl &pkeyUri, const QString &certFile, const QString &keyFile, const QString &keyPassword, QObject *parent)
    : KJob(parent)
{
    Q_UNUSED(pkeyUri)
    Q_UNUSED(certFile)
    Q_UNUSED(keyFile)
    Q_UNUSED(keyPassword)
}

const QUrl &EwsPKeyAuthJob::resultUri() const
{
    static const QUrl empty;
    return empty;
}

QString EwsPKeyAuthJob::getAuthHeader()
{
    return {};
}
}
