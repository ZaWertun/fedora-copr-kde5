/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QNetworkRequest>
#include <QPair>
#include <QVariantMap>
#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#include <QUrlQuery>
#else
#include <QScriptEngine>
#include <QScriptValueIterator>
#endif

#include "debug.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2.h"
#include "o2replyserver.h"

/// Parse JSON data into a QVariantMap
static QVariantMap parseTokenResponse(const QByteArray &data)
{
#if QT_VERSION >= 0x050000
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "parseTokenResponse: Failed to parse token response due to err:" << err.errorString();
        return {};
    }

    if (!doc.isObject()) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "parseTokenResponse: Token response is not an object";
        return {};
    }

    return doc.object().toVariantMap();
#else
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + QString(data) + QLatin1Char(')'));
    QScriptValueIterator it(value);
    QVariantMap map;

    while (it.hasNext()) {
        it.next();
        map.insert(it.name(), it.value().toVariant());
    }

    return map;
#endif
}

/// Add query parameters to a query
static void addQueryParametersToUrl(QUrl &url, const QList<QPair<QString, QString>> &parameters)
{
#if QT_VERSION < 0x050000
    url.setQueryItems(parameters);
#else
    QUrlQuery query(url);
    query.setQueryItems(parameters);
    url.setQuery(query);
#endif
}

O2::O2(QObject *parent)
    : O0BaseAuth(parent)
{
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    grantFlow_ = GrantFlowAuthorizationCode;
    localhostPolicy_ = QLatin1String(O2_CALLBACK_URL);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, &O2ReplyServer::verificationReceived, this, &O2::onVerificationReceived);
}

O2::GrantFlow O2::grantFlow() const
{
    return grantFlow_;
}

void O2::setGrantFlow(O2::GrantFlow value)
{
    grantFlow_ = value;
    Q_EMIT grantFlowChanged();
}

QString O2::username() const
{
    return username_;
}

void O2::setUsername(const QString &value)
{
    username_ = value;
    Q_EMIT usernameChanged();
}

QString O2::password() const
{
    return password_;
}

void O2::setPassword(const QString &value)
{
    password_ = value;
    Q_EMIT passwordChanged();
}

QString O2::scope() const
{
    return scope_;
}

void O2::setScope(const QString &value)
{
    scope_ = value;
    Q_EMIT scopeChanged();
}

QString O2::requestUrl() const
{
    return requestUrl_.toString();
}

void O2::setRequestUrl(const QString &value)
{
    requestUrl_ = QUrl(value);
    Q_EMIT requestUrlChanged();
}

QString O2::tokenUrl()
{
    return tokenUrl_.toString();
}

void O2::setTokenUrl(const QString &value)
{
    tokenUrl_ = QUrl(value);
    Q_EMIT tokenUrlChanged();
}

QString O2::refreshTokenUrl()
{
    return refreshTokenUrl_.toString();
}

void O2::setRefreshTokenUrl(const QString &value)
{
    refreshTokenUrl_ = QUrl(value);
    Q_EMIT refreshTokenUrlChanged();
}

void O2::link()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::link";

    if (linked()) {
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::link: Linked already";
        Q_EMIT linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken(QLatin1String(""));
    setTokenSecret(QLatin1String(""));
    setExtraTokens(QVariantMap());
    setRefreshToken(QString());
    setExpires(0);

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Start listening to authentication replies
        replyServer_->listen(QHostAddress::Any, localPort_);

        // Save redirect URI, as we have to reuse it when requesting the access token
        redirectUri_ = localhostPolicy_.arg(replyServer_->serverPort());

        // Assemble initial authentication URL
        QList<QPair<QString, QString>> parameters;
        parameters.append(
            qMakePair(QLatin1String(O2_OAUTH2_RESPONSE_TYPE),
                      (grantFlow_ == GrantFlowAuthorizationCode) ? QLatin1String(O2_OAUTH2_GRANT_TYPE_CODE) : QLatin1String(O2_OAUTH2_GRANT_TYPE_TOKEN)));
        parameters.append(qMakePair(QLatin1String(O2_OAUTH2_CLIENT_ID), clientId_));
        parameters.append(qMakePair(QLatin1String(O2_OAUTH2_REDIRECT_URI), redirectUri_));
        parameters.append(qMakePair(QLatin1String(O2_OAUTH2_SCOPE), scope_));
        parameters.append(qMakePair(QLatin1String(O2_OAUTH2_API_KEY), apiKey_));

        // Show authentication URL with a web browser
        QUrl url(requestUrl_);
        addQueryParametersToUrl(url, parameters);
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::link: Emit openBrowser" << url.toString();
        Q_EMIT openBrowser(url);
    } else if (grantFlow_ == GrantFlowResourceOwnerPasswordCredentials) {
        QList<O0RequestParameter> parameters;
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_ID, clientId_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_SECRET, clientSecret_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_USERNAME, username_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_PASSWORD, password_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_GRANT_TYPE_PASSWORD));
        parameters.append(O0RequestParameter(O2_OAUTH2_SCOPE, scope_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_API_KEY, apiKey_.toUtf8()));
        QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

        QUrl url(tokenUrl_);
        QNetworkRequest tokenRequest(url);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
        QNetworkReply *tokenReply = manager_->post(tokenRequest, payload);

        connect(tokenReply, &QNetworkReply::finished, this, &O2::onTokenReplyFinished, Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    }
}

void O2::unlink()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::unlink";
    setLinked(false);
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    setExtraTokens(QVariantMap());
    Q_EMIT linkingSucceeded();
}

void O2::onVerificationReceived(const QMultiMap<QString, QString> &response)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onVerificationReceived:" << response;
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onVerificationReceived: Emitting closeBrowser()";
    Q_EMIT closeBrowser();

    if (response.contains(QStringLiteral("error"))) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::onVerificationReceived: Verification failed: " << response;
        Q_EMIT linkingFailed();
        return;
    }

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Save access code
        setCode(response.value(QLatin1String(O2_OAUTH2_GRANT_TYPE_CODE)));

        // Exchange access code for access/refresh tokens
        QString query;
        if (!apiKey_.isEmpty()) {
            query = QString(QLatin1String("?") + QLatin1String(O2_OAUTH2_API_KEY) + QLatin1String("=") + apiKey_);
        }
        QNetworkRequest tokenRequest(QUrl(tokenUrl_.toString() + query));
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
        QMap<QString, QString> parameters;
        parameters.insert(QLatin1String(O2_OAUTH2_GRANT_TYPE_CODE), code());
        parameters.insert(QLatin1String(O2_OAUTH2_CLIENT_ID), clientId_);
        parameters.insert(QLatin1String(O2_OAUTH2_CLIENT_SECRET), clientSecret_);
        parameters.insert(QLatin1String(O2_OAUTH2_REDIRECT_URI), redirectUri_);
        parameters.insert(QLatin1String(O2_OAUTH2_GRANT_TYPE), QLatin1String(O2_AUTHORIZATION_CODE));
        QByteArray data = buildRequestBody(parameters);
        QNetworkReply *tokenReply = manager_->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply, &QNetworkReply::finished, this, &O2::onTokenReplyFinished, Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    } else {
        setToken(response.value(QLatin1String(O2_OAUTH2_ACCESS_TOKEN)));
        setRefreshToken(response.value(QLatin1String(O2_OAUTH2_REFRESH_TOKEN)));
    }
}

QString O2::code() const
{
    QString key = QString::fromLatin1(O2_KEY_CODE).arg(clientId_);
    return store_->value(key);
}

void O2::setCode(const QString &c)
{
    QString key = QString::fromLatin1(O2_KEY_CODE).arg(clientId_);
    store_->setValue(key, c);
}

void O2::onTokenReplyFinished()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onTokenReplyFinished";
    auto tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();
        QVariantMap tokens = parseTokenResponse(replyData);

        // Check for mandatory tokens
        if (tokens.contains(QLatin1String(O2_OAUTH2_ACCESS_TOKEN))) {
            setToken(tokens.take(QLatin1String(O2_OAUTH2_ACCESS_TOKEN)).toString());
            bool ok = false;
            int expiresIn = tokens.take(QLatin1String(O2_OAUTH2_EXPIRES_IN)).toInt(&ok);
            if (ok) {
                qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onTokenReplyFinished: Token expires in" << expiresIn << "seconds";
                setExpires(QDateTime::currentSecsSinceEpoch() + expiresIn);
            }
            setRefreshToken(tokens.take(QLatin1String(O2_OAUTH2_REFRESH_TOKEN)).toString());
            setExtraTokens(tokens);
            timedReplies_.remove(tokenReply);
            setLinked(true);
            Q_EMIT linkingSucceeded();
        } else {
            qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::onTokenReplyFinished: oauth_token missing from response" << replyData;
            Q_EMIT linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error)
{
    auto tokenReply = qobject_cast<QNetworkReply *>(sender());
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::onTokenReplyError: " << error << ": " << tokenReply->errorString();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onTokenReplyError: " << tokenReply->readAll();
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(tokenReply);
    Q_EMIT linkingFailed();
}

QByteArray O2::buildRequestBody(const QMap<QString, QString> &parameters)
{
    QByteArray body;
    bool first = true;
    for (const QString &key : parameters.keys()) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        QString value = parameters.value(key);
        body.append(QUrl::toPercentEncoding(key) + QStringLiteral("=").toUtf8() + QUrl::toPercentEncoding(value));
    }
    return body;
}

int O2::expires()
{
    const QString key = QString::fromLatin1(O2_KEY_EXPIRES).arg(clientId_);
    return store_->value(key).toInt();
}

void O2::setExpires(int v)
{
    const QString key = QString::fromLatin1(O2_KEY_EXPIRES).arg(clientId_);
    store_->setValue(key, QString::number(v));
}

QString O2::refreshToken()
{
    const QString key = QString::fromLatin1(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2::setRefreshToken(const QString &v)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::setRefreshToken" << v.left(4) << "...";
    QString key = QString::fromLatin1(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    store_->setValue(key, v);
}

void O2::refresh()
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::refresh: Token: ..." << refreshToken().right(7);

    if (refreshToken().isEmpty()) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::refresh: No refresh token";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }
    if (refreshTokenUrl_.isEmpty()) {
        qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::refresh: Refresh token URL not set";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }

    QNetworkRequest refreshRequest(refreshTokenUrl_);
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
    QMap<QString, QString> parameters;
    parameters.insert(QLatin1String(O2_OAUTH2_CLIENT_ID), clientId_);
    parameters.insert(QLatin1String(O2_OAUTH2_CLIENT_SECRET), clientSecret_);
    parameters.insert(QLatin1String(O2_OAUTH2_REFRESH_TOKEN), refreshToken());
    parameters.insert(QLatin1String(O2_OAUTH2_GRANT_TYPE), QLatin1String(O2_OAUTH2_REFRESH_TOKEN));

    QByteArray data = buildRequestBody(parameters);
    QNetworkReply *refreshReply = manager_->post(refreshRequest, data);
    timedReplies_.add(refreshReply);
    connect(refreshReply, &QNetworkReply::finished, this, &O2::onRefreshFinished, Qt::QueuedConnection);
    connect(refreshReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2::onRefreshFinished()
{
    auto refreshReply = qobject_cast<QNetworkReply *>(sender());
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O2::onRefreshFinished: Error" << (int)refreshReply->error() << refreshReply->errorString();
    if (refreshReply->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply->readAll();
        QVariantMap tokens = parseTokenResponse(reply);
        setToken(tokens.value(QLatin1String(O2_OAUTH2_ACCESS_TOKEN)).toString());
        setExpires(QDateTime::currentSecsSinceEpoch() + tokens.value(QLatin1String(O2_OAUTH2_EXPIRES_IN)).toInt());
        setRefreshToken(tokens.value(QLatin1String(O2_OAUTH2_REFRESH_TOKEN)).toString());
        timedReplies_.remove(refreshReply);
        setLinked(true);
        Q_EMIT linkingSucceeded();
        Q_EMIT refreshFinished(QNetworkReply::NoError);
        qCDebug(TOMBOYNOTESRESOURCE_LOG) << " New token expires in" << expires() << "seconds";
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error)
{
    auto refreshReply = qobject_cast<QNetworkReply *>(sender());
    qCWarning(TOMBOYNOTESRESOURCE_LOG) << "O2::onRefreshError: " << error;
    unlink();
    timedReplies_.remove(refreshReply);
    Q_EMIT refreshFinished(error);
}

QString O2::localhostPolicy() const
{
    return localhostPolicy_;
}

void O2::setLocalhostPolicy(const QString &value)
{
    localhostPolicy_ = value;
}

QString O2::apiKey() const
{
    return apiKey_;
}

void O2::setApiKey(const QString &value)
{
    apiKey_ = value;
}

QByteArray O2::replyContent() const
{
    return replyServer_->replyContent();
}

void O2::setReplyContent(const QByteArray &value)
{
    replyServer_->setReplyContent(value);
}

bool O2::ignoreSslErrors()
{
    return timedReplies_.ignoreSslErrors();
}

void O2::setIgnoreSslErrors(bool ignoreSslErrors)
{
    timedReplies_.setIgnoreSslErrors(ignoreSslErrors);
}
