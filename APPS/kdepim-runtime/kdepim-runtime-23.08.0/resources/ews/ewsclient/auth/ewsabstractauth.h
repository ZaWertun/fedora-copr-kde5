/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QObject>

/**
 *  @brief  Abstract base class for authentication providers
 *
 *  This class forms an interface for authentication modules. It abstracts
 *  all actions needed to authenticate HTTP requests.
 *
 *  The goal is for the EwsResource class to instantiate a concrete class
 *  derived from this one depending on the configured authentication setting.
 *  Once the class is instantiated the resource class must connect the
 *  requestWalletPassword(), requestWalletMap(), walletPasswordReqiestFinished()
 *  and walletMapRequestFinished() methods to the appropriate signals/slots of
 *  the settings object. Once that is done the resource shall call the init()
 *  method which triggers retrieval of any stored credential state.
 *
 *  The main user of this class is the @e EwsRequest class, which uses the
 *  getAuthData() method on every HTTP request it issues. This method shall
 *  return all the necessary information needed to authenticate, which could
 *  be a username/password set and/or custom HTTP headers. At this stage the
 *  getAuthData() method must return immediately - it must not perform any
 *  long operations such as external requests. In case no authentication
 *  information is available the method returns false, which causes the request
 *  to be aborted.
 *
 *  In case the request fails due to a 401 Unauthorized error or is aborted due
 *  to abobe conditions the request calls the requestAuthFailed() method, signals
 *  the main resource class that authentication has failed and aborts the request.
 *
 *  The main resource class in response to an authentication failure sets the
 *  resource offline and attempts to reauthenticate (if the resource supports it)
 *  by calling the authenticate() method - first with the @e interactive argument
 *  set to @e false. If that fails (the authFailed() signal is received) the
 *  resource displays a notification message that interactive authentication is
 *  needed. The message to display is retrieved using the reauthPrompt() method.
 *  If the user chooses to authenticate the authenticate() method is called with
 *  @e interactive set to @e true. If that also fails, the failed authentication
 *  prompt is retrieved using the authFailedPrompt() method and the resource stays
 *  offline until the user updates the configuration. In case authentication
 *  succeeds at any stage the resource is set back to online. At this stage the
 *  authentication class also uses the setWalletPassword() and setWalletMap()
 *  signals to write the updated credentials to the wallet.
 */
class EwsAbstractAuth : public QObject
{
    Q_OBJECT
public:
    explicit EwsAbstractAuth(QObject *parent = nullptr);
    ~EwsAbstractAuth() override = default;
    virtual void init() = 0;
    virtual bool getAuthData(QString &username, QString &password, QStringList &customHeaders) = 0;
    virtual void notifyRequestAuthFailed();
    virtual bool authenticate(bool interactive) = 0;
    virtual const QString &reauthPrompt() const = 0;
    virtual const QString &authFailedPrompt() const = 0;

    virtual void walletPasswordRequestFinished(const QString &password) = 0;
    virtual void walletMapRequestFinished(const QMap<QString, QString> &map) = 0;

    void setAuthParentWidget(QWidget *widget);

    void setPKeyAuthCertificateFiles(const QString &certFile, const QString &pkeyFile);
Q_SIGNALS:
    void authSucceeded();
    void authFailed(const QString &error);
    void requestAuthFailed();
    void requestWalletPassword(bool ask);
    void requestWalletMap();
    void setWalletPassword(const QString &password);
    void setWalletMap(const QMap<QString, QString> &map);

protected:
    QWidget *mAuthParentWidget = nullptr;
    QString mPKeyCertFile;
    QString mPKeyKeyFile;
};
