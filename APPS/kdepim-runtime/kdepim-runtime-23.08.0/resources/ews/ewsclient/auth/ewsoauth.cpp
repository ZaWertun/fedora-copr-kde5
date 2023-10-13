/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsoauth.h"

#include <QUrlQuery>
#ifdef EWSOAUTH_UNITTEST
#include "ewsoauth_ut_mock.h"
using namespace Mock;
#else
#include "ewspkeyauthjob.h"
#include <QAbstractOAuthReplyHandler>
#include <QDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QPointer>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>
#endif
#include "ewsclient_debug.h"
#include <KLocalizedString>
#include <QJsonDocument>
#include <QTcpServer>
#include <QTcpSocket>

static const auto o365AuthorizationUrl = QUrl(QStringLiteral("https://login.microsoftonline.com/common/oauth2/authorize"));
static const auto o365AccessTokenUrl = QUrl(QStringLiteral("https://login.microsoftonline.com/common/oauth2/token"));
static const auto o365FakeUserAgent =
    QStringLiteral("Mozilla/5.0 (Linux; Android 7.0; SM-G930V Build/NRD90M) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.125 Mobile Safari/537.36");
static const auto o365Resource = QStringLiteral("https%3A%2F%2Foutlook.office365.com%2F");

static const auto pkeyAuthSuffix = QStringLiteral(" PKeyAuth/1.0");
static const auto pkeyRedirectUri = QStringLiteral("urn:http-auth:PKeyAuth");
static const QString pkeyPasswordMapKey = QStringLiteral("pkey-password");

static const QString accessTokenMapKey = QStringLiteral("access-token");
static const QString refreshTokenMapKey = QStringLiteral("refresh-token");

class EwsOAuthUrlSchemeHandler final : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    EwsOAuthUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }

    ~EwsOAuthUrlSchemeHandler() override = default;
    void requestStarted(QWebEngineUrlRequestJob *request) override;
Q_SIGNALS:
    void returnUriReceived(const QUrl &url);
};

class EwsOAuthReplyHandler final : public QAbstractOAuthReplyHandler
{
    Q_OBJECT
public:
    EwsOAuthReplyHandler(QObject *parent, const QString &returnUri)
        : QAbstractOAuthReplyHandler(parent)
        , mReturnUri(returnUri)
    {
    }

    ~EwsOAuthReplyHandler() override = default;

    Q_REQUIRED_RESULT QString callback() const override
    {
        return mReturnUri;
    }

    void networkReplyFinished(QNetworkReply *reply) override;
Q_SIGNALS:
    void replyError(const QString &error);

private:
    const QString mReturnUri;
};

class EwsOAuthRequestInterceptor final : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    EwsOAuthRequestInterceptor(QObject *parent, const QString &redirectUri, const QString &clientId);
    ~EwsOAuthRequestInterceptor() override = default;

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;
public Q_SLOTS:
    void setPKeyAuthInputArguments(const QString &pkeyCertFile, const QString &pkeyKeyFile, const QString &pkeyPassword);
Q_SIGNALS:
    void redirectUriIntercepted(const QUrl &url);

private:
    const QString mRedirectUri;
    const QString mClientId;
    QString mPKeyCertFile;
    QString mPKeyKeyFile;
    QString mPKeyPassword;
    QString mPKeyAuthResponse;
    QString mPKeyAuthSubmitUrl;
    QTcpServer mRedirectServer;
};

class EwsOAuthPrivate final : public QObject
{
    Q_OBJECT
public:
    EwsOAuthPrivate(EwsOAuth *parent, const QString &email, const QString &appId, const QString &redirectUri);
    ~EwsOAuthPrivate() override = default;

    bool authenticate(bool interactive);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void modifyParametersFunction(QAbstractOAuth::Stage stage, QVariantMap *parameters);
#else
    void modifyParametersFunction(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters);
#endif
    void authorizeWithBrowser(const QUrl &url);
    void redirectUriIntercepted(const QUrl &url);
    void granted();
    void error(const QString &error, const QString &errorDescription, const QUrl &uri);
    QVariantMap queryToVarmap(const QUrl &url);
    void pkeyAuthResult(KJob *job);

    QWebEngineView mWebView;
    QWebEngineProfile mWebProfile;
    QWebEnginePage mWebPage;
    QOAuth2AuthorizationCodeFlow mOAuth2;
    EwsOAuthReplyHandler mReplyHandler;
    EwsOAuthRequestInterceptor mRequestInterceptor;
    EwsOAuthUrlSchemeHandler mSchemeHandler;
    QString mToken;
    const QString mEmail;
    const QString mRedirectUri;
    bool mAuthenticated;
    QPointer<QDialog> mWebDialog;
    QString mPKeyPassword;

    EwsOAuth *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(EwsOAuth)
};

void EwsOAuthUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    Q_EMIT returnUriReceived(request->requestUrl());
}

void EwsOAuthReplyHandler::networkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT replyError(reply->errorString());
        return;
    } else if (reply->header(QNetworkRequest::ContentTypeHeader).isNull()) {
        Q_EMIT replyError(QStringLiteral("Empty or no Content-type header"));
        return;
    }
    const auto cth = reply->header(QNetworkRequest::ContentTypeHeader);
    const auto ct = cth.isNull() ? QStringLiteral("text/html") : cth.toString();
    const auto data = reply->readAll();
    if (data.isEmpty()) {
        Q_EMIT replyError(QStringLiteral("No data received"));
        return;
    }
    Q_EMIT replyDataReceived(data);
    QVariantMap tokens;
    if (ct.startsWith(QLatin1String("text/html")) || ct.startsWith(QLatin1String("application/x-www-form-urlencoded"))) {
        QUrlQuery q(QString::fromUtf8(data));
        const auto items = q.queryItems(QUrl::FullyDecoded);
        for (const auto &it : items) {
            tokens.insert(it.first, it.second);
        }
    } else if (ct.startsWith(QLatin1String("application/json")) || ct.startsWith(QLatin1String("text/javascript"))) {
        const auto document = QJsonDocument::fromJson(data);
        if (!document.isObject()) {
            Q_EMIT replyError(QStringLiteral("Invalid JSON data received"));
            return;
        }
        const auto object = document.object();
        if (object.isEmpty()) {
            Q_EMIT replyError(QStringLiteral("Empty JSON data received"));
            return;
        }
        tokens = object.toVariantMap();
    } else {
        Q_EMIT replyError(QStringLiteral("Unknown content type"));
        return;
    }

    const auto error = tokens.value(QStringLiteral("error"));
    if (error.isValid()) {
        Q_EMIT replyError(QStringLiteral("Received error response: ") + error.toString());
        return;
    }
    const auto accessToken = tokens.value(QStringLiteral("access_token"));
    if (!accessToken.isValid() || accessToken.toString().isEmpty()) {
        Q_EMIT replyError(QStringLiteral("Received empty or no access token"));
        return;
    }

    Q_EMIT tokensReceived(tokens);
}

EwsOAuthRequestInterceptor::EwsOAuthRequestInterceptor(QObject *parent, const QString &redirectUri, const QString &clientId)
    : QWebEngineUrlRequestInterceptor(parent)
    , mRedirectUri(redirectUri)
    , mClientId(clientId)
{
    /* Workaround for QTBUG-88861 - start a trivial HTTP server to serve the redirect.
     * The redirection must be done using JavaScript as HTTP-protocol redirections (301, 302)
     * do not cause QWebEngineUrlRequestInterceptor::interceptRequest() to fire. */
    connect(&mRedirectServer, &QTcpServer::newConnection, this, [this]() {
        const auto socket = mRedirectServer.nextPendingConnection();
        if (socket) {
            connect(socket, &QIODevice::readyRead, this, [this, socket]() {
                const auto response = QStringLiteral(
                                          "HTTP/1.1 200 OK\n\n<!DOCTYPE html>\n<html><body><p>You will be redirected "
                                          "shortly.</p><script>window.location.href=\"%1\";</script></body></html>\n")
                                          .arg(mPKeyAuthSubmitUrl);
                socket->write(response.toLocal8Bit());
            });
            connect(socket, &QIODevice::bytesWritten, this, [socket]() {
                socket->deleteLater();
            });
        }
    });
    mRedirectServer.listen(QHostAddress::LocalHost);
}

void EwsOAuthRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    const auto url = info.requestUrl();

    qCDebugNC(EWSCLI_LOG) << QStringLiteral("Intercepted browser navigation to ") << url;

    if (url.toString(QUrl::RemoveQuery) == pkeyRedirectUri) {
        qCDebugNC(EWSCLI_LOG) << QStringLiteral("Found PKeyAuth URI");

        auto pkeyAuthJob = new EwsPKeyAuthJob(url, mPKeyCertFile, mPKeyKeyFile, mPKeyPassword, this);
        mPKeyAuthResponse = pkeyAuthJob->getAuthHeader();
        QUrlQuery query(url.query());
        if (!mPKeyAuthResponse.isEmpty() && query.hasQueryItem(QStringLiteral("SubmitUrl"))) {
            mPKeyAuthSubmitUrl = query.queryItemValue(QStringLiteral("SubmitUrl"), QUrl::FullyDecoded);
            /* Workaround for QTBUG-88861
             * When the PKey authentication starts, the server issues a request for a "special" PKey URL
             * containing the challenge arguments and expects that a response is composed and the browser
             * then redirected to the URL found in the SubmitUrl argument with the response passed using
             * the HTTP Authorization header.
             * Unfortunately the Qt WebEngine request interception mechanism will ignore custom HTTP headers
             * when issuing a redirect.
             * To work around that the EWS Resource launches a minimalistic HTTP server to serve a
             * simple webpage with redirection. This way the redirection happens externally to the
             * Qt Web Engine and the submit URL can be captured by the request interceptor again, this time
             * only to add the missing Authorization header. */
            qCDebugNC(EWSCLI_LOG) << QStringLiteral("Redirecting to PKey SubmitUrl via QTBUG-88861 workaround");
            info.redirect(QUrl(QStringLiteral("http://localhost:%1/").arg(mRedirectServer.serverPort())));
        } else {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to retrieve PKey authorization header");
        }
    } else if (url.toString(QUrl::RemoveQuery) == mPKeyAuthSubmitUrl) {
        info.setHttpHeader(QByteArray("Authorization"), mPKeyAuthResponse.toLocal8Bit());
    } else if (url.toString(QUrl::RemoveQuery) == mRedirectUri) {
        qCDebug(EWSCLI_LOG) << QStringLiteral("Found redirect URI - blocking request");

        Q_EMIT redirectUriIntercepted(url);
        info.block(true);
    }
}

void EwsOAuthRequestInterceptor::setPKeyAuthInputArguments(const QString &pkeyCertFile, const QString &pkeyKeyFile, const QString &pkeyPassword)
{
    mPKeyCertFile = pkeyCertFile;
    mPKeyKeyFile = pkeyKeyFile;
    mPKeyPassword = pkeyPassword;
}

EwsOAuthPrivate::EwsOAuthPrivate(EwsOAuth *parent, const QString &email, const QString &appId, const QString &redirectUri)
    : QObject(nullptr)
    , mWebView((QWidget *)nullptr)
    , mWebProfile()
    , mWebPage(&mWebProfile)
    , mReplyHandler(this, redirectUri)
    , mRequestInterceptor(this, redirectUri, appId)
    , mEmail(email)
    , mRedirectUri(redirectUri)
    , mAuthenticated(false)
    , q_ptr(parent)
{
    mOAuth2.setReplyHandler(&mReplyHandler);
    mOAuth2.setAuthorizationUrl(o365AuthorizationUrl);
    mOAuth2.setAccessTokenUrl(o365AccessTokenUrl);
    mOAuth2.setClientIdentifier(appId);
    mWebProfile.setUrlRequestInterceptor(&mRequestInterceptor);
    mWebProfile.installUrlSchemeHandler("urn", &mSchemeHandler);

    mWebView.setPage(&mWebPage);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mOAuth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        modifyParametersFunction(stage, parameters);
    });
#else
    mOAuth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
        modifyParametersFunction(stage, parameters);
    });
#endif
    connect(&mOAuth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &EwsOAuthPrivate::authorizeWithBrowser);
    connect(&mOAuth2, &QOAuth2AuthorizationCodeFlow::granted, this, &EwsOAuthPrivate::granted);
    connect(&mOAuth2, &QOAuth2AuthorizationCodeFlow::error, this, &EwsOAuthPrivate::error);
    connect(&mRequestInterceptor, &EwsOAuthRequestInterceptor::redirectUriIntercepted, this, &EwsOAuthPrivate::redirectUriIntercepted, Qt::QueuedConnection);
    connect(&mReplyHandler, &EwsOAuthReplyHandler::replyError, this, [this](const QString &err) {
        error(QStringLiteral("Network reply error"), err, QUrl());
    });
}

bool EwsOAuthPrivate::authenticate(bool interactive)
{
    // Q_Q(EwsOAuth);

    qCInfoNC(EWSCLI_LOG) << QStringLiteral("Starting OAuth2 authentication");

    if (!mOAuth2.refreshToken().isEmpty()) {
        mOAuth2.refreshAccessToken();
        return true;
    } else if (interactive) {
        mOAuth2.grant();
        return true;
    } else {
        return false;
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void EwsOAuthPrivate::modifyParametersFunction(QAbstractOAuth::Stage stage, QVariantMap *parameters)
#else
void EwsOAuthPrivate::modifyParametersFunction(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters)
#endif
{
    switch (stage) {
    case QAbstractOAuth::Stage::RequestingAccessToken:
        parameters->insert(QStringLiteral("resource"), o365Resource);
        break;
    case QAbstractOAuth::Stage::RequestingAuthorization:
        parameters->insert(QStringLiteral("prompt"), QStringLiteral("login"));
        parameters->insert(QStringLiteral("login_hint"), mEmail);
        parameters->insert(QStringLiteral("resource"), o365Resource);
        break;
    default:
        break;
    }
}

void EwsOAuthPrivate::authorizeWithBrowser(const QUrl &url)
{
    Q_Q(EwsOAuth);

    qCInfoNC(EWSCLI_LOG) << QStringLiteral("Launching browser for authentication");

    /* Bad bad Microsoft...
     * When Conditional Access is enabled on the server the OAuth2 authentication server only supports Windows,
     * MacOSX, Android and iOS. No option to include Linux. Support (i.e. guarantee that it works)
     * is one thing, but blocking unsupported browsers completely is just wrong.
     * Fortunately enough this can be worked around by faking the user agent to something "supported".
     */
    auto userAgent = o365FakeUserAgent;
    if (!q->mPKeyCertFile.isNull() && !q->mPKeyKeyFile.isNull()) {
        qCInfoNC(EWSCLI_LOG) << QStringLiteral("Found PKeyAuth certificates");
        userAgent += pkeyAuthSuffix;
    } else {
        qCInfoNC(EWSCLI_LOG) << QStringLiteral("PKeyAuth certificates not found");
    }
    mWebProfile.setHttpUserAgent(userAgent);

    mRequestInterceptor.setPKeyAuthInputArguments(q->mPKeyCertFile, q->mPKeyKeyFile, mPKeyPassword);

    mWebDialog = new QDialog(q->mAuthParentWidget);
    mWebDialog->setObjectName(QStringLiteral("Akonadi EWS Resource - Authentication"));
    mWebDialog->setWindowIcon(QIcon(QStringLiteral("akonadi-ews")));
    mWebDialog->resize(400, 500);
    auto layout = new QHBoxLayout(mWebDialog);
    layout->setContentsMargins({});
    layout->addWidget(&mWebView);
    mWebView.show();

    connect(mWebDialog.data(), &QDialog::rejected, this, [this]() {
        error(QStringLiteral("User cancellation"), QStringLiteral("The authentication browser was closed"), QUrl());
    });

    mWebView.load(url);
    mWebDialog->show();
}

QVariantMap EwsOAuthPrivate::queryToVarmap(const QUrl &url)
{
    QUrlQuery query(url);
    QVariantMap varmap;
    const auto items = query.queryItems();
    for (const auto &item : items) {
        varmap[item.first] = item.second;
    }
    return varmap;
}

void EwsOAuthPrivate::redirectUriIntercepted(const QUrl &url)
{
    qCDebugNC(EWSCLI_LOG) << QStringLiteral("Intercepted redirect URI from browser: ") << url;

    mWebView.stop();
    mWebDialog->hide();

    Q_Q(EwsOAuth);
    if (url.toString(QUrl::RemoveQuery) == pkeyRedirectUri) {
        qCDebugNC(EWSCLI_LOG) << QStringLiteral("Found PKeyAuth URI");

        auto pkeyAuthJob = new EwsPKeyAuthJob(url, q->mPKeyCertFile, q->mPKeyKeyFile, mPKeyPassword, this);

        connect(pkeyAuthJob, &KJob::result, this, &EwsOAuthPrivate::pkeyAuthResult);

        pkeyAuthJob->start();

        return;
    }
    Q_EMIT mOAuth2.authorizationCallbackReceived(queryToVarmap(url));
}

void EwsOAuthPrivate::pkeyAuthResult(KJob *j)
{
    auto job = qobject_cast<EwsPKeyAuthJob *>(j);

    qCDebugNC(EWSCLI_LOG) << QStringLiteral("PKeyAuth result: %1").arg(job->error());
    QVariantMap varmap;
    if (job->error() == 0) {
        varmap = queryToVarmap(job->resultUri());
    } else {
        varmap[QStringLiteral("error")] = job->errorString();
    }
    Q_EMIT mOAuth2.authorizationCallbackReceived(varmap);
}

void EwsOAuthPrivate::granted()
{
    Q_Q(EwsOAuth);

    qCInfoNC(EWSCLI_LOG) << QStringLiteral("Authentication succeeded");

    mAuthenticated = true;

    QMap<QString, QString> map;
    map[accessTokenMapKey] = mOAuth2.token();
    map[refreshTokenMapKey] = mOAuth2.refreshToken();
    Q_EMIT q->setWalletMap(map);

    Q_EMIT q->authSucceeded();
}

void EwsOAuthPrivate::error(const QString &error, const QString &errorDescription, const QUrl &uri)
{
    Q_Q(EwsOAuth);

    Q_UNUSED(uri)

    mAuthenticated = false;

    mOAuth2.setRefreshToken(QString());
    qCInfoNC(EWSCLI_LOG) << QStringLiteral("Authentication failed: ") << error << errorDescription;

    Q_EMIT q->authFailed(error);
}

EwsOAuth::EwsOAuth(QObject *parent, const QString &email, const QString &appId, const QString &redirectUri)
    : EwsAbstractAuth(parent)
    , d_ptr(new EwsOAuthPrivate(this, email, appId, redirectUri))
{
}

EwsOAuth::~EwsOAuth()
{
}

void EwsOAuth::init()
{
    Q_EMIT requestWalletMap();
}

bool EwsOAuth::getAuthData(QString &username, QString &password, QStringList &customHeaders)
{
    Q_D(const EwsOAuth);

    Q_UNUSED(username)
    Q_UNUSED(password)

    if (d->mAuthenticated) {
        customHeaders.append(QStringLiteral("Authorization: Bearer ") + d->mOAuth2.token());
        return true;
    } else {
        return false;
    }
}

void EwsOAuth::notifyRequestAuthFailed()
{
    Q_D(EwsOAuth);

    d->mOAuth2.setToken(QString());
    d->mAuthenticated = false;

    EwsAbstractAuth::notifyRequestAuthFailed();
}

bool EwsOAuth::authenticate(bool interactive)
{
    Q_D(EwsOAuth);

    return d->authenticate(interactive);
}

const QString &EwsOAuth::reauthPrompt() const
{
    static const QString prompt =
        xi18nc("@info",
               "Microsoft Exchange credentials for the account <b>%1</b> are no longer valid. You need to authenticate in order to continue using it.",
               QStringLiteral("%1"));
    return prompt;
}

const QString &EwsOAuth::authFailedPrompt() const
{
    static const QString prompt =
        xi18nc("@info",
               "Failed to obtain credentials for Microsoft Exchange account <b>%1</b>. Please update it in the account settings page.",
               QStringLiteral("%1"));
    return prompt;
}

void EwsOAuth::walletPasswordRequestFinished(const QString &password)
{
    Q_UNUSED(password)
}

void EwsOAuth::walletMapRequestFinished(const QMap<QString, QString> &map)
{
    Q_D(EwsOAuth);

    if (map.contains(pkeyPasswordMapKey)) {
        d->mPKeyPassword = map[pkeyPasswordMapKey];
    }
    if (map.contains(refreshTokenMapKey)) {
        d->mOAuth2.setRefreshToken(map[refreshTokenMapKey]);
    }
    if (map.contains(accessTokenMapKey)) {
        d->mOAuth2.setToken(map[accessTokenMapKey]);
        d->mAuthenticated = true;
        Q_EMIT authSucceeded();
    } else {
        Q_EMIT authFailed(QStringLiteral("Access token request failed"));
    }
}

#include "ewsoauth.moc"
