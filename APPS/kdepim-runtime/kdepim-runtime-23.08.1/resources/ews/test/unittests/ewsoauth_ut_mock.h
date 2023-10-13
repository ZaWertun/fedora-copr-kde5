/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QBuffer>
#include <QDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QPointer>
#include <QUrl>
#include <QWidget>

#include <KLocalizedString>
Q_DECLARE_LOGGING_CATEGORY(EWSCLI_LOG)

namespace Mock
{
class QWebEngineUrlRequestJob : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineUrlRequestJob(const QUrl &url, QObject *parent)
        : QObject(parent)
        , mUrl(url)
    {
    }

    ~QWebEngineUrlRequestJob() override = default;

    QUrl requestUrl() const;

    QUrl mUrl;
};

class QWebEngineUrlRequestInfo : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineUrlRequestInfo(const QUrl &url, QObject *parent)
        : QObject(parent)
        , mBlocked(false)
        , mUrl(url)
    {
    }

    ~QWebEngineUrlRequestInfo() override = default;

    QUrl requestUrl() const;
    void block(bool shouldBlock);
    void setHttpHeader(const QByteArray &name, const QByteArray &value);
    void redirect(const QUrl &url);

    bool mBlocked;
    QUrl mUrl;
};

class QWebEngineUrlRequestInterceptor : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineUrlRequestInterceptor(QObject *parent);
    ~QWebEngineUrlRequestInterceptor() override;

    virtual void interceptRequest(QWebEngineUrlRequestInfo &info) = 0;
};

class QWebEngineUrlSchemeHandler : public QObject
{
    Q_OBJECT
public:
    QWebEngineUrlSchemeHandler(QObject *parent);
    ~QWebEngineUrlSchemeHandler() override;

    virtual void requestStarted(QWebEngineUrlRequestJob *request) = 0;
};

class QWebEngineProfile : public QObject
{
    Q_OBJECT
public:
    explicit QWebEngineProfile(QObject *parent = nullptr);
    ~QWebEngineProfile() override;

    void setHttpUserAgent(const QString &ua);
    void setUrlRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);
    void installUrlSchemeHandler(QByteArray const &scheme, QWebEngineUrlSchemeHandler *handler);
Q_SIGNALS:
    void logEvent(const QString &event);

public:
    QString mUserAgent;
    QWebEngineUrlRequestInterceptor *mInterceptor;
    QString mScheme;
    QWebEngineUrlSchemeHandler *mHandler;
};

class QWebEnginePage : public QObject
{
    Q_OBJECT
public:
    explicit QWebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);
    ~QWebEnginePage() override;
Q_SIGNALS:
    void logEvent(const QString &event);

public:
    QWebEngineProfile *mProfile;
};

class QWebEngineView : public QWidget
{
    Q_OBJECT
public:
    using AuthFunc = std::function<void(const QUrl &, QVariantMap &)>;

    explicit QWebEngineView(QWidget *parent);
    ~QWebEngineView() override;

    void load(const QUrl &url);
    void setPage(QWebEnginePage *page);
    void stop();

    void setAuthFunction(const AuthFunc &func);
    void setRedirectUri(const QString &uri);

    static QPointer<QWebEngineView> instance;
Q_SIGNALS:
    void logEvent(const QString &event);

protected:
    void simulatePageLoad(const QUrl &url);

    QWebEnginePage *mPage;
    QString mRedirectUri;
    AuthFunc mAuthFunction;
};

class QNetworkRequest
{
public:
    enum KnownHeaders {
        ContentTypeHeader,
    };
};

class QNetworkReply : public QBuffer
{
    Q_OBJECT
public:
    enum NetworkError {
        NoError = 0,
    };
    Q_ENUM(NetworkError)

    explicit QNetworkReply(QObject *parent)
        : QBuffer(parent)
    {
    }

    ~QNetworkReply() override = default;

    NetworkError error() const;
    QVariant header(QNetworkRequest::KnownHeaders header) const;

    QMap<QNetworkRequest::KnownHeaders, QVariant> mHeaders;
    NetworkError mError;
};

class QAbstractOAuthReplyHandler : public QObject
{
    Q_OBJECT
public:
    QAbstractOAuthReplyHandler(QObject *parent);
    ~QAbstractOAuthReplyHandler() override;

    virtual QString callback() const = 0;
    virtual void networkReplyFinished(QNetworkReply *reply) = 0;
Q_SIGNALS:
    void replyDataReceived(const QByteArray &data);
    void tokensReceived(const QVariantMap &tokens);
};

class QAbstractOAuth : public QObject
{
    Q_OBJECT
public:
    Q_ENUMS(Stage)
    enum class Stage {
        RequestingTemporaryCredentials,
        RequestingAuthorization,
        RequestingAccessToken,
        RefreshingAccessToken,
    };

    Q_ENUMS(Status)
    enum class Status {
        NotAuthenticated,
        TemporaryCredentialsReceived,
        Granted,
        RefreshingToken,
    };

    explicit QAbstractOAuth(QObject *parent);
    ~QAbstractOAuth() override = default;

    void setReplyHandler(QAbstractOAuthReplyHandler *handler);
    void setAuthorizationUrl(const QUrl &url);
    void setClientIdentifier(const QString &identifier);
    void setModifyParametersFunction(const std::function<void(QAbstractOAuth::Stage, QMap<QString, QVariant> *)> &func);
    QString token() const;
    void setToken(const QString &token);
    Status status() const;
Q_SIGNALS:
    void authorizeWithBrowser(const QUrl &url);
    void granted();
    void logEvent(const QString &event);

protected:
    QAbstractOAuthReplyHandler *mReplyHandler;
    QUrl mAuthUrl;
    QString mClientId;
    std::function<void(QAbstractOAuth::Stage, QMap<QString, QVariant> *)> mModifyParamsFunc;
    QString mToken;
    QString mRefreshToken;
    QUrl mTokenUrl;
    QString mResource;
    Status mStatus;
};

class QAbstractOAuth2 : public QAbstractOAuth
{
    Q_OBJECT
public:
    explicit QAbstractOAuth2(QObject *parent);
    ~QAbstractOAuth2() override = default;

    QString refreshToken() const;
    void setRefreshToken(const QString &token);

Q_SIGNALS:
    void authorizationCallbackReceived(QMap<QString, QVariant> const &params);
    void error(const QString &error, const QString &errorDescription, const QUrl &uri);
};

class QOAuth2AuthorizationCodeFlow : public QAbstractOAuth2
{
    Q_OBJECT
public:
    using TokenFunc = std::function<QNetworkReply::NetworkError(QString &, QMap<QNetworkRequest::KnownHeaders, QVariant> &)>;

    explicit QOAuth2AuthorizationCodeFlow(QObject *parent = nullptr);
    ~QOAuth2AuthorizationCodeFlow() override;

    void setAccessTokenUrl(const QUrl &url);
    void grant();
    void refreshAccessToken();

    QString redirectUri() const;
    void setTokenFunction(const TokenFunc &func);
    void setState(const QString &state);

    static QUrlQuery mapToSortedQuery(QMap<QString, QVariant> const &map);

    static QPointer<QOAuth2AuthorizationCodeFlow> instance;

protected:
    void authCallbackReceived(QMap<QString, QVariant> const &params);
    void replyDataCallbackReceived(const QByteArray &data);
    void tokenCallbackReceived(const QVariantMap &tokens);
    void doRefreshAccessToken();

    TokenFunc mTokenFunc;
    QString mState;
};

class KJob : public QObject
{
    Q_OBJECT
public:
    explicit KJob(QObject *)
    {
    }

    ~KJob() override = default;

    int error() const
    {
        return 0;
    }

    const QString &errorString() const;
Q_SIGNALS:
    void result(KJob *job);
};

class EwsPKeyAuthJob : public KJob
{
    Q_OBJECT
public:
    explicit EwsPKeyAuthJob(const QUrl &pkeyUri, const QString &certFile, const QString &keyFile, const QString &keyPassword, QObject *parent);
    ~EwsPKeyAuthJob() override = default;
    void start()
    {
    }

    const QUrl &resultUri() const;
    QString getAuthHeader();
};

QString browserDisplayRequestString();
QString modifyParamsAuthString(const QString &clientId, const QString &returnUri, const QString &state);
QString
authUrlString(const QString &authUrl, const QString &clientId, const QString &returnUri, const QString &email, const QString &resource, const QString &state);
QString authorizeWithBrowserString(const QString &url);
QString loadWebPageString(const QString &url);
QString interceptRequestString(const QString &url);
QString interceptRequestBlockedString(bool blocked);
QString authorizationCallbackReceivedString(const QString &code);
QString modifyParamsTokenString(const QString &clientId, const QString &returnUri, const QString &code);
QString networkReplyFinishedString(const QString &data);
QString replyDataCallbackString(const QString &data);
QString tokenCallbackString(const QString &accessToken,
                            const QString &refreshToken,
                            const QString &idToken,
                            quint64 time,
                            unsigned int tokenLifetime,
                            unsigned int extTokenLifetime,
                            const QString &resource);
QString requestWalletMapString();
}
