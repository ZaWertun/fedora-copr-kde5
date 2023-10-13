/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#include "debug.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QNetworkRequest>
#include <QRandomGenerator>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#if QT_VERSION >= 0x050100
#include <QMessageAuthenticationCode>
#endif

#include "o2/o0globals.h"
#include "o2/o0settingsstore.h"
#include "o2/o1.h"
#include "o2/o2replyserver.h"

O1::O1(QObject *parent)
    : O0BaseAuth(parent)
{
    setSignatureMethod(QLatin1String(O2_SIGNATURE_TYPE_HMAC_SHA1));
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, &O2ReplyServer::verificationReceived, this, &O1::onVerificationReceived);
    setCallbackUrl(QLatin1String(O2_CALLBACK_URL));
}

QUrl O1::requestTokenUrl() const
{
    return requestTokenUrl_;
}

void O1::setRequestTokenUrl(const QUrl &v)
{
    requestTokenUrl_ = v;
    Q_EMIT requestTokenUrlChanged();
}

QList<O0RequestParameter> O1::requestParameters()
{
    return requestParameters_;
}

void O1::setRequestParameters(const QList<O0RequestParameter> &v)
{
    requestParameters_ = v;
}

QString O1::callbackUrl() const
{
    return callbackUrl_;
}

void O1::setCallbackUrl(const QString &v)
{
    callbackUrl_ = v;
}

QUrl O1::authorizeUrl() const
{
    return authorizeUrl_;
}

void O1::setAuthorizeUrl(const QUrl &value)
{
    authorizeUrl_ = value;
    Q_EMIT authorizeUrlChanged();
}

QUrl O1::accessTokenUrl() const
{
    return accessTokenUrl_;
}

void O1::setAccessTokenUrl(const QUrl &value)
{
    accessTokenUrl_ = value;
    Q_EMIT accessTokenUrlChanged();
}

QString O1::signatureMethod()
{
    return signatureMethod_;
}

void O1::setSignatureMethod(const QString &value)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::setSignatureMethod: " << value;
    signatureMethod_ = value;
}

void O1::unlink()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::unlink";
    setLinked(false);
    setToken(QString());
    setTokenSecret(QString());
    setExtraTokens(QVariantMap());
    Q_EMIT linkingSucceeded();
}

#if QT_VERSION < 0x050100
/// Calculate the HMAC variant of SHA1 hash.
/// @author     http://qt-project.org/wiki/HMAC-SHA1.
/// @copyright  Creative Commons Attribution-ShareAlike 2.5 Generic.
static QByteArray hmacSha1(QByteArray key, QByteArray baseString)
{
    int blockSize = 64;
    if (key.length() > blockSize) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
    QByteArray innerPadding(blockSize, char(0x36));
    QByteArray outerPadding(blockSize, char(0x5c));
    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i);
        outerPadding[i] = outerPadding[i] ^ key.at(i);
    }
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

#endif

/// Get HTTP operation name.
static QString getOperationName(QNetworkAccessManager::Operation op)
{
    switch (op) {
    case QNetworkAccessManager::GetOperation:
        return QStringLiteral("GET");
    case QNetworkAccessManager::PostOperation:
        return QStringLiteral("POST");
    case QNetworkAccessManager::PutOperation:
        return QStringLiteral("PUT");
    case QNetworkAccessManager::DeleteOperation:
        return QStringLiteral("DEL");
    default:
        return {};
    }
}

/// Build a concatenated/percent-encoded string from a list of headers.
QByteArray O1::encodeHeaders(const QList<O0RequestParameter> &headers)
{
    return QUrl::toPercentEncoding(QString::fromLatin1(createQueryParameters(headers)));
}

/// Build a base string for signing.
QByteArray O1::getRequestBase(const QList<O0RequestParameter> &oauthParams,
                              const QList<O0RequestParameter> &otherParams,
                              const QUrl &url,
                              QNetworkAccessManager::Operation op)
{
    QByteArray base;

    // Initialize base string with the operation name (e.g. "GET") and the base URL
    base.append(getOperationName(op).toUtf8() + "&");
    base.append(QUrl::toPercentEncoding(url.toString(QUrl::RemoveQuery)) + "&");

    // Append a sorted+encoded list of all request parameters to the base string
    QList<O0RequestParameter> headers(oauthParams);
    headers.append(otherParams);
    std::sort(headers.begin(), headers.end());
    base.append(encodeHeaders(headers));

    return base;
}

QByteArray O1::sign(const QList<O0RequestParameter> &oauthParams,
                    const QList<O0RequestParameter> &otherParams,
                    const QUrl &url,
                    QNetworkAccessManager::Operation op,
                    const QString &consumerSecret,
                    const QString &tokenSecret)
{
    QByteArray baseString = getRequestBase(oauthParams, otherParams, url, op);
    QByteArray secret = QUrl::toPercentEncoding(consumerSecret) + "&" + QUrl::toPercentEncoding(tokenSecret);
#if QT_VERSION >= 0x050100
    return QMessageAuthenticationCode::hash(baseString, secret, QCryptographicHash::Sha1).toBase64();
#else
    return hmacSha1(secret, baseString);
#endif
}

QByteArray O1::buildAuthorizationHeader(const QList<O0RequestParameter> &oauthParams)
{
    bool first = true;
    QByteArray ret("OAuth ");
    QList<O0RequestParameter> headers(oauthParams);
    std::sort(headers.begin(), headers.end());
    for (const O0RequestParameter &h : std::as_const(headers)) {
        if (first) {
            first = false;
        } else {
            ret.append(",");
        }
        ret.append(h.name);
        ret.append("=\"");
        ret.append(QUrl::toPercentEncoding(QString::fromLatin1(h.value)));
        ret.append("\"");
    }
    return ret;
}

QByteArray O1::generateSignature(const QList<O0RequestParameter> &headers,
                                 const QNetworkRequest &req,
                                 const QList<O0RequestParameter> &signingParameters,
                                 QNetworkAccessManager::Operation operation)
{
    QByteArray signature;
    if (signatureMethod() == QLatin1String(O2_SIGNATURE_TYPE_HMAC_SHA1)) {
        signature = sign(headers, signingParameters, req.url(), operation, clientSecret(), tokenSecret());
    } else if (signatureMethod() == QLatin1String(O2_SIGNATURE_TYPE_PLAINTEXT)) {
        signature = clientSecret().toLatin1() + "&" + tokenSecret().toLatin1();
    }
    return signature;
}

void O1::link()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::link";
    if (linked()) {
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::link: Linked already";
        Q_EMIT linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken(QString());
    setTokenSecret(QString());
    setExtraTokens(QVariantMap());

    // Start reply server
    replyServer_->listen(QHostAddress::Any, localPort());

    // Get any query parameters for the request
    QUrlQuery requestData;
    const auto params{requestParameters()};
    for (const O0RequestParameter &param : params) {
        requestData.addQueryItem(QString::fromLatin1(param.name), QString::fromLatin1(QUrl::toPercentEncoding(QString::fromLatin1(param.value))));
    }

    // Get the request url and add parameters
    QUrl requestUrl = requestTokenUrl();
    requestUrl.setQuery(requestData);

    // Create request
    QNetworkRequest request(requestUrl);

    // Create initial token request
    QList<O0RequestParameter> headers;
    headers.append(O0RequestParameter(O2_OAUTH_CALLBACK, callbackUrl().arg(replyServer_->serverPort()).toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_NONCE, nonce()));
    headers.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()).toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
    headers.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(headers, request, requestParameters(), QNetworkAccessManager::PostOperation)));

    // Clear request token
    requestToken_.clear();
    requestTokenSecret_.clear();

    // Post request
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(headers));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
    QNetworkReply *reply = manager_->post(request, QByteArray());
    connect(reply, &QNetworkReply::errorOccurred, this, &O1::onTokenRequestError);
    connect(reply, &QNetworkReply::finished, this, &O1::onTokenRequestFinished);
}

void O1::onTokenRequestError(QNetworkReply::NetworkError error)
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenRequestError:" << (int)error << reply->errorString() << reply->readAll();
    Q_EMIT linkingFailed();
}

void O1::onTokenRequestFinished()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenRequestFinished";
    auto reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenRequestFinished: " << reply->errorString();
        return;
    }

    // Get request token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    requestToken_ = response.value(QLatin1String(O2_OAUTH_TOKEN), QString());
    requestTokenSecret_ = response.value(QLatin1String(O2_OAUTH_TOKEN_SECRET), QString());
    setToken(requestToken_);
    setTokenSecret(requestTokenSecret_);

    // Checking for "oauth_callback_confirmed" is present and set to true
    QString oAuthCbConfirmed = response.value(QLatin1String(O2_OAUTH_CALLBACK_CONFIRMED), QStringLiteral("false"));
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty() || (oAuthCbConfirmed == QLatin1String("false"))) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenRequestFinished: No oauth_token, oauth_token_secret or oauth_callback_confirmed in response :"
                                           << data;
        Q_EMIT linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    url.addQueryItem(O2_OAUTH_CALLBACK, callbackUrl().arg(replyServer_->serverPort()).toLatin1());
#else
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String(O2_OAUTH_TOKEN), requestToken_);
    query.addQueryItem(QLatin1String(O2_OAUTH_CALLBACK), QString::fromLatin1(callbackUrl().arg(replyServer_->serverPort()).toLatin1()));
    url.setQuery(query);
#endif
    Q_EMIT openBrowser(url);
}

void O1::onVerificationReceived(const QMultiMap<QString, QString> &params)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::onVerificationReceived";
    Q_EMIT closeBrowser();
    verifier_ = params.value(QLatin1String(O2_OAUTH_VERFIER), QString());
    if (params.value(QLatin1String(O2_OAUTH_TOKEN)) == requestToken_) {
        // Exchange request token for access token
        exchangeToken();
    } else {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onVerificationReceived: oauth_token missing or doesn't match";
        Q_EMIT linkingFailed();
    }
}

void O1::exchangeToken()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::exchangeToken";

    // Create token exchange request
    QNetworkRequest request(accessTokenUrl());
    QList<O0RequestParameter> oauthParams;
    oauthParams.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()).toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TOKEN, requestToken_.toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERFIER, verifier_.toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    oauthParams.append(
        O0RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(oauthParams, request, QList<O0RequestParameter>(), QNetworkAccessManager::PostOperation)));

    // Post request
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(oauthParams));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
    QNetworkReply *reply = manager_->post(request, QByteArray());
    connect(reply, &QNetworkReply::errorOccurred, this, &O1::onTokenExchangeError);
    connect(reply, &QNetworkReply::finished, this, &O1::onTokenExchangeFinished);
}

void O1::onTokenExchangeError(QNetworkReply::NetworkError error)
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenExchangeError:" << (int)error << reply->errorString() << reply->readAll();
    Q_EMIT linkingFailed();
}

void O1::onTokenExchangeFinished()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenExchangeFinished";

    auto reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenExchangeFinished: " << reply->errorString();
        return;
    }

    // Get access token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    if (response.contains(QLatin1String(O2_OAUTH_TOKEN)) && response.contains(QLatin1String(O2_OAUTH_TOKEN_SECRET))) {
        setToken(response.take(QLatin1String(O2_OAUTH_TOKEN)));
        setTokenSecret(response.take(QLatin1String(O2_OAUTH_TOKEN_SECRET)));
        // Set extra tokens if any
        if (!response.isEmpty()) {
            QVariantMap extraTokens;
            for (const QString &key : std::as_const(response)) {
                extraTokens.insert(key, response.value(key));
            }
            setExtraTokens(extraTokens);
        }
        setLinked(true);
        Q_EMIT linkingSucceeded();
    } else {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O1::onTokenExchangeFinished: oauth_token or oauth_token_secret missing from response" << data;
        Q_EMIT linkingFailed();
    }
}

QMap<QString, QString> O1::parseResponse(const QByteArray &response)
{
    QMap<QString, QString> ret;
    const auto responses{response.split('&')};
    for (const QByteArray &param : responses) {
        const QList<QByteArray> kv = param.split('=');
        if (kv.length() == 2) {
            ret.insert(QUrl::fromPercentEncoding(kv[0]), QUrl::fromPercentEncoding(kv[1]));
        }
    }
    return ret;
}

QByteArray O1::nonce()
{
    QString u = QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    u.append(QString::number(QRandomGenerator::global()->bounded(RAND_MAX)));
    return u.toLatin1();
}
