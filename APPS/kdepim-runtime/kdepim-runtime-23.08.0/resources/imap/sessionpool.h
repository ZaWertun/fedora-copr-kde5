/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QStringList>

#include <KIMAP/ListJob>
#include <KIMAP/Session>
#include <kimap/sessionuiproxy.h>

namespace KIMAP
{
struct MailBoxDescriptor;
}

class ImapAccount;
class PasswordRequesterInterface;

class SessionPool : public QObject
{
    Q_OBJECT
    Q_ENUMS(ConnectError)

public:
    enum ErrorCodes {
        NoError,
        PasswordRequestError,
        ReconnectNeededError,
        EncryptionError,
        LoginFailError,
        CapabilitiesTestError,
        IncompatibleServerError,
        NoAvailableSessionError,
        CouldNotConnectError,
        CancelledError
    };

    enum SessionTermination {
        LogoutSession,
        CloseSession,
    };

    explicit SessionPool(int maxPoolSize, QObject *parent = nullptr);
    ~SessionPool() override;

    PasswordRequesterInterface *passwordRequester() const;
    void setPasswordRequester(PasswordRequesterInterface *requester);
    void cancelPasswordRequests();

    KIMAP::SessionUiProxy::Ptr sessionUiProxy() const;
    void setSessionUiProxy(KIMAP::SessionUiProxy::Ptr proxy);

    void setClientId(const QByteArray &clientId);

    Q_REQUIRED_RESULT bool isConnected() const;
    bool connect(ImapAccount *account);
    void disconnect(SessionTermination termination = LogoutSession);

    Q_REQUIRED_RESULT qint64 requestSession();
    void cancelSessionRequest(qint64 id);
    void releaseSession(KIMAP::Session *session);

    ImapAccount *account() const;
    Q_REQUIRED_RESULT QStringList serverCapabilities() const;
    QList<KIMAP::MailBoxDescriptor> serverNamespaces() const;
    enum Namespace {
        Personal,
        User,
        Shared,
    };
    QList<KIMAP::MailBoxDescriptor> serverNamespaces(Namespace) const;

Q_SIGNALS:
    void connectionLost(KIMAP::Session *session);

    void sessionRequestDone(qint64 requestNumber, KIMAP::Session *session, int errorCode = NoError, const QString &errorString = QString());
    void connectDone(int errorCode = NoError, const QString &errorString = QString());
    void disconnectDone();

private Q_SLOTS:
    void processPendingRequests();

    void onPasswordRequestDone(int resultType, const QString &password);
    void onLoginDone(KJob *job);
    void onCapabilitiesTestDone(KJob *job);
    void onNamespacesTestDone(KJob *job);
    void onIdDone(KJob *job);

    void onSessionDestroyed(QObject *);

private:
    void onConnectionLost();
    void killSession(KIMAP::Session *session, SessionTermination termination);
    void declareSessionReady(KIMAP::Session *session);
    void cancelSessionCreation(KIMAP::Session *session, int errorCode, const QString &errorString);
    void requestPassword();

    static qint64 m_requestCounter;

    const int m_maxPoolSize;
    ImapAccount *m_account = nullptr;
    PasswordRequesterInterface *m_passwordRequester = nullptr;
    KIMAP::SessionUiProxy::Ptr m_sessionUiProxy;

    bool m_initialConnectDone = false;
    KIMAP::Session *m_pendingInitialSession = nullptr;

    QList<qint64> m_pendingRequests;
    QList<KIMAP::Session *> m_connectingPool; // in preparation
    QList<KIMAP::Session *> m_unusedPool; // ready to be used
    QList<KIMAP::Session *> m_reservedPool; // currently used

    QStringList m_capabilities;
    QList<KIMAP::MailBoxDescriptor> m_namespaces;
    QList<KIMAP::MailBoxDescriptor> m_personalNamespaces;
    QList<KIMAP::MailBoxDescriptor> m_userNamespaces;
    QList<KIMAP::MailBoxDescriptor> m_sharedNamespaces;
    QByteArray m_clientId;
};
