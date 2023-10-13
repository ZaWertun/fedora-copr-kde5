/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionpool.h"

#include <QSslSocket>
#include <QTimer>

#include "imapresource_debug.h"
#include <KLocalizedString>

#include <KIMAP/CapabilitiesJob>
#include <KIMAP/IdJob>
#include <KIMAP/LogoutJob>
#include <KIMAP/NamespaceJob>

#include "imapaccount.h"
#include "passwordrequesterinterface.h"

qint64 SessionPool::m_requestCounter = 0;

SessionPool::SessionPool(int maxPoolSize, QObject *parent)
    : QObject(parent)
    , m_maxPoolSize(maxPoolSize)
{
}

SessionPool::~SessionPool()
{
    disconnect(CloseSession);
}

PasswordRequesterInterface *SessionPool::passwordRequester() const
{
    return m_passwordRequester;
}

void SessionPool::setPasswordRequester(PasswordRequesterInterface *requester)
{
    delete m_passwordRequester;

    m_passwordRequester = requester;
    m_passwordRequester->setParent(this);
    QObject::connect(m_passwordRequester, &PasswordRequesterInterface::done, this, &SessionPool::onPasswordRequestDone);
}

void SessionPool::cancelPasswordRequests()
{
    m_passwordRequester->cancelPasswordRequests();
}

KIMAP::SessionUiProxy::Ptr SessionPool::sessionUiProxy() const
{
    return m_sessionUiProxy;
}

void SessionPool::setSessionUiProxy(KIMAP::SessionUiProxy::Ptr proxy)
{
    m_sessionUiProxy = proxy;
}

bool SessionPool::isConnected() const
{
    return m_initialConnectDone;
}

void SessionPool::requestPassword()
{
    if (m_account->authenticationMode() == KIMAP::LoginJob::GSSAPI) {
        // for GSSAPI we don't have to ask for username/password, because it uses session wide tickets
        QMetaObject::invokeMethod(this,
                                  "onPasswordRequestDone",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, PasswordRequesterInterface::PasswordRetrieved),
                                  Q_ARG(QString, QString()));
    } else {
        m_passwordRequester->requestPassword();
    }
}

bool SessionPool::connect(ImapAccount *account)
{
    if (!account) {
        return false;
    }

    m_account = account;
    requestPassword();

    return true;
}

void SessionPool::disconnect(SessionTermination termination)
{
    if (!m_account) {
        return;
    }

    const auto session{m_unusedPool + m_reservedPool + m_connectingPool};
    for (KIMAP::Session *s : session) {
        killSession(s, termination);
    }
    m_unusedPool.clear();
    m_reservedPool.clear();
    m_connectingPool.clear();
    m_pendingInitialSession = nullptr;
    m_passwordRequester->cancelPasswordRequests();

    delete m_account;
    m_account = nullptr;
    m_namespaces.clear();
    m_capabilities.clear();

    m_initialConnectDone = false;
    Q_EMIT disconnectDone();
}

qint64 SessionPool::requestSession()
{
    if (!m_initialConnectDone) {
        return -1;
    }

    qint64 requestNumber = ++m_requestCounter;

    // The queue was empty, so trigger the processing
    if (m_pendingRequests.isEmpty()) {
        QTimer::singleShot(0, this, &SessionPool::processPendingRequests);
    }

    m_pendingRequests << requestNumber;

    return requestNumber;
}

void SessionPool::cancelSessionRequest(qint64 id)
{
    Q_ASSERT(id > 0);
    m_pendingRequests.removeAll(id);
}

void SessionPool::releaseSession(KIMAP::Session *session)
{
    const int removeSession = m_reservedPool.removeAll(session);
    if (removeSession > 0) {
        m_unusedPool << session;
    }
}

ImapAccount *SessionPool::account() const
{
    return m_account;
}

QStringList SessionPool::serverCapabilities() const
{
    return m_capabilities;
}

QList<KIMAP::MailBoxDescriptor> SessionPool::serverNamespaces() const
{
    return m_namespaces;
}

QList<KIMAP::MailBoxDescriptor> SessionPool::serverNamespaces(Namespace ns) const
{
    switch (ns) {
    case Personal:
        return m_personalNamespaces;
    case User:
        return m_userNamespaces;
    case Shared:
        return m_sharedNamespaces;
    default:
        break;
    }
    Q_ASSERT(false);
    return {};
}

void SessionPool::killSession(KIMAP::Session *session, SessionTermination termination)
{
    Q_ASSERT(session);

    if (!m_unusedPool.contains(session) && !m_reservedPool.contains(session) && !m_connectingPool.contains(session)) {
        qCWarning(IMAPRESOURCE_LOG) << "Unmanaged session" << session;
        Q_ASSERT(false);
        return;
    }
    QObject::disconnect(session, &KIMAP::Session::connectionLost, this, &SessionPool::onConnectionLost);
    m_unusedPool.removeAll(session);
    m_reservedPool.removeAll(session);
    m_connectingPool.removeAll(session);

    if (session->state() != KIMAP::Session::Disconnected && termination == LogoutSession) {
        auto logout = new KIMAP::LogoutJob(session);
        QObject::connect(logout, &KJob::result, session, &QObject::deleteLater);
        logout->start();
    } else {
        session->close();
        session->deleteLater();
    }
}

void SessionPool::declareSessionReady(KIMAP::Session *session)
{
    // This can happen if we happen to disconnect while capabilities and namespace are being retrieved,
    // resulting in us keeping a dangling pointer to a deleted session
    if (!m_connectingPool.contains(session)) {
        qCWarning(IMAPRESOURCE_LOG) << "Tried to declare a removed session ready";
        return;
    }

    m_pendingInitialSession = nullptr;

    if (!m_initialConnectDone) {
        m_initialConnectDone = true;
        Q_EMIT connectDone();
        // If the slot connected to connectDone() decided to disconnect the SessionPool
        // then we must end here, because we expect the pools to be empty now!
        if (!m_initialConnectDone) {
            return;
        }
    }

    m_connectingPool.removeAll(session);

    if (m_pendingRequests.isEmpty()) {
        m_unusedPool << session;
    } else {
        m_reservedPool << session;
        Q_EMIT sessionRequestDone(m_pendingRequests.takeFirst(), session);

        if (!m_pendingRequests.isEmpty()) {
            QTimer::singleShot(0, this, &SessionPool::processPendingRequests);
        }
    }
}

void SessionPool::cancelSessionCreation(KIMAP::Session *session, int errorCode, const QString &errorMessage)
{
    m_pendingInitialSession = nullptr;

    QString msg;
    if (m_account) {
        msg = i18n("Could not connect to the IMAP-server %1.\n%2", m_account->server(), errorMessage);
    } else {
        // Can happen when we lose all ready connections while trying to establish
        // a new connection, for example.
        msg = i18n("Could not connect to the IMAP server.\n%1", errorMessage);
    }

    if (!m_initialConnectDone) {
        disconnect(); // kills all sessions, including \a session
    } else {
        if (session) {
            killSession(session, LogoutSession);
        }
        if (!m_pendingRequests.isEmpty()) {
            Q_EMIT sessionRequestDone(m_pendingRequests.takeFirst(), nullptr, errorCode, errorMessage);
            if (!m_pendingRequests.isEmpty()) {
                QTimer::singleShot(0, this, &SessionPool::processPendingRequests);
            }
        }
    }
    // Always emit this at the end. This can call SessionPool::disconnect via ImapResource.
    Q_EMIT connectDone(errorCode, msg);
}

void SessionPool::processPendingRequests()
{
    if (!m_account) {
        // The connection to the server is lost; no point processing pending requests
        for (int request : std::as_const(m_pendingRequests)) {
            Q_EMIT sessionRequestDone(request, nullptr, LoginFailError, i18n("Disconnected from server during login."));
        }
        return;
    }

    if (!m_unusedPool.isEmpty()) {
        // We have a session ready to give out
        KIMAP::Session *session = m_unusedPool.takeFirst();
        m_reservedPool << session;
        if (!m_pendingRequests.isEmpty()) {
            Q_EMIT sessionRequestDone(m_pendingRequests.takeFirst(), session);
            if (!m_pendingRequests.isEmpty()) {
                QTimer::singleShot(0, this, &SessionPool::processPendingRequests);
            }
        }
    } else if (m_unusedPool.size() + m_reservedPool.size() < m_maxPoolSize) {
        // We didn't reach the max pool size yet so create a new one
        requestPassword();
    } else {
        // No session available, and max pool size reached
        if (!m_pendingRequests.isEmpty()) {
            Q_EMIT sessionRequestDone(m_pendingRequests.takeFirst(),
                                      nullptr,
                                      NoAvailableSessionError,
                                      i18n("Could not create another extra connection to the IMAP-server %1.", m_account->server()));
            if (!m_pendingRequests.isEmpty()) {
                QTimer::singleShot(0, this, &SessionPool::processPendingRequests);
            }
        }
    }
}

void SessionPool::onPasswordRequestDone(int resultType, const QString &password)
{
    QString errorMessage;

    if (!m_account) {
        // it looks like the connection was lost while we were waiting
        // for the password, we should fail all the pending requests and stop there
        for (int request : std::as_const(m_pendingRequests)) {
            Q_EMIT sessionRequestDone(request, nullptr, LoginFailError, i18n("Disconnected from server during login."));
        }
        return;
    }

    switch (resultType) {
    case PasswordRequesterInterface::PasswordRetrieved:
        // All is fine
        break;
    case PasswordRequesterInterface::ReconnectNeeded:
        cancelSessionCreation(m_pendingInitialSession, ReconnectNeededError, errorMessage);
        return;
    case PasswordRequesterInterface::UserRejected:
        errorMessage = i18n("Could not read the password: user rejected wallet access");
        if (m_pendingInitialSession) {
            cancelSessionCreation(m_pendingInitialSession, LoginFailError, errorMessage);
        } else {
            Q_EMIT connectDone(PasswordRequestError, errorMessage);
        }
        return;
    case PasswordRequesterInterface::EmptyPasswordEntered:
        errorMessage = i18n("Empty password");
        if (m_pendingInitialSession) {
            cancelSessionCreation(m_pendingInitialSession, LoginFailError, errorMessage);
        } else {
            Q_EMIT connectDone(PasswordRequestError, errorMessage);
        }
        return;
    }

    if (m_account->encryptionMode() != KIMAP::LoginJob::Unencrypted && !QSslSocket::supportsSsl()) {
        qCWarning(IMAPRESOURCE_LOG) << "Crypto not supported!";
        Q_EMIT connectDone(EncryptionError,
                           i18n("You requested TLS/SSL to connect to %1, but your "
                                "system does not seem to be set up for that.",
                                m_account->server()));
        disconnect();
        return;
    }

    KIMAP::Session *session = nullptr;
    if (m_pendingInitialSession) {
        session = m_pendingInitialSession;
    } else {
        session = new KIMAP::Session(m_account->server(), m_account->port(), this);
        QObject::connect(session, &QObject::destroyed, this, &SessionPool::onSessionDestroyed);
        session->setUiProxy(m_sessionUiProxy);
        session->setTimeout(m_account->timeout());
        session->setUseNetworkProxy(m_account->useNetworkProxy());
        m_connectingPool << session;
    }

    QObject::connect(session, &KIMAP::Session::connectionLost, this, &SessionPool::onConnectionLost);

    auto loginJob = new KIMAP::LoginJob(session);
    loginJob->setUserName(m_account->userName());
    loginJob->setPassword(password);
    loginJob->setEncryptionMode(m_account->encryptionMode());
    loginJob->setAuthenticationMode(m_account->authenticationMode());

    QObject::connect(loginJob, &KJob::result, this, &SessionPool::onLoginDone);
    loginJob->start();
}

void SessionPool::onLoginDone(KJob *job)
{
    auto login = static_cast<KIMAP::LoginJob *>(job);
    // Can happen if we disconnected meanwhile
    if (!m_connectingPool.contains(login->session())) {
        Q_EMIT connectDone(CancelledError, i18n("Disconnected from server during login."));
        return;
    }

    if (job->error() == 0) {
        if (m_initialConnectDone) {
            declareSessionReady(login->session());
        } else {
            // On initial connection we ask for capabilities
            auto capJob = new KIMAP::CapabilitiesJob(login->session());
            QObject::connect(capJob, &KIMAP::CapabilitiesJob::result, this, &SessionPool::onCapabilitiesTestDone);
            capJob->start();
        }
    } else {
        if (job->error() == KIMAP::LoginJob::ERR_COULD_NOT_CONNECT) {
            if (m_account) {
                cancelSessionCreation(login->session(),
                                      CouldNotConnectError,
                                      i18n("Could not connect to the IMAP-server %1.\n%2", m_account->server(), job->errorString()));
            } else {
                // Can happen when we lose all ready connections while trying to login.
                cancelSessionCreation(login->session(), CouldNotConnectError, i18n("Could not connect to the IMAP-server.\n%1", job->errorString()));
            }
        } else {
            // Connection worked, but login failed -> ask for a different password or ssl settings.
            m_pendingInitialSession = login->session();
            m_passwordRequester->requestPassword(PasswordRequesterInterface::WrongPasswordRequest, job->errorString());
        }
    }
}

void SessionPool::onCapabilitiesTestDone(KJob *job)
{
    auto capJob = qobject_cast<KIMAP::CapabilitiesJob *>(job);
    // Can happen if we disconnected meanwhile
    if (!m_connectingPool.contains(capJob->session())) {
        Q_EMIT connectDone(CancelledError, i18n("Disconnected from server during login."));
        return;
    }

    if (job->error()) {
        if (m_account) {
            cancelSessionCreation(capJob->session(),
                                  CapabilitiesTestError,
                                  i18n("Could not test the capabilities supported by the "
                                       "IMAP server %1.\n%2",
                                       m_account->server(),
                                       job->errorString()));
        } else {
            // Can happen when we lose all ready connections while trying to check capabilities.
            cancelSessionCreation(capJob->session(),
                                  CapabilitiesTestError,
                                  i18n("Could not test the capabilities supported by the "
                                       "IMAP server.\n%1",
                                       job->errorString()));
        }
        return;
    }

    m_capabilities = capJob->capabilities();

    QStringList missing;
    const QStringList expected = {QStringLiteral("IMAP4REV1")};
    for (const QString &capability : expected) {
        if (!m_capabilities.contains(capability)) {
            missing << capability;
        }
    }

    if (!missing.isEmpty()) {
        cancelSessionCreation(capJob->session(),
                              IncompatibleServerError,
                              i18n("Cannot use the IMAP server %1, "
                                   "some mandatory capabilities are missing: %2. "
                                   "Please ask your sysadmin to upgrade the server.",
                                   m_account->server(),
                                   missing.join(QLatin1String(", "))));
        return;
    }

    // If the extension is supported, grab the namespaces from the server
    if (m_capabilities.contains(QLatin1String("NAMESPACE"))) {
        auto nsJob = new KIMAP::NamespaceJob(capJob->session());
        QObject::connect(nsJob, &KIMAP::NamespaceJob::result, this, &SessionPool::onNamespacesTestDone);
        nsJob->start();
        return;
    } else if (m_capabilities.contains(QLatin1String("ID"))) {
        auto idJob = new KIMAP::IdJob(capJob->session());
        idJob->setField("name", m_clientId);
        QObject::connect(idJob, &KIMAP::IdJob::result, this, &SessionPool::onIdDone);
        idJob->start();
        return;
    } else {
        declareSessionReady(capJob->session());
    }
}

void SessionPool::setClientId(const QByteArray &clientId)
{
    m_clientId = clientId;
}

void SessionPool::onNamespacesTestDone(KJob *job)
{
    auto nsJob = qobject_cast<KIMAP::NamespaceJob *>(job);
    // Can happen if we disconnect meanwhile
    if (!m_connectingPool.contains(nsJob->session())) {
        Q_EMIT connectDone(CancelledError, i18n("Disconnected from server during login."));
        return;
    }

    m_personalNamespaces = nsJob->personalNamespaces();
    m_userNamespaces = nsJob->userNamespaces();
    m_sharedNamespaces = nsJob->sharedNamespaces();

    if (nsJob->containsEmptyNamespace()) {
        // When we got the empty namespace here, we assume that the other
        // ones can be freely ignored and that the server will give us all
        // the mailboxes if we list from the empty namespace itself...

        m_namespaces.clear();
    } else {
        // ... otherwise we assume that we have to list explicitly each
        // namespace

        m_namespaces = nsJob->personalNamespaces() + nsJob->userNamespaces() + nsJob->sharedNamespaces();
    }

    if (m_capabilities.contains(QLatin1String("ID"))) {
        auto idJob = new KIMAP::IdJob(nsJob->session());
        idJob->setField("name", m_clientId);
        QObject::connect(idJob, &KIMAP::IdJob::result, this, &SessionPool::onIdDone);
        idJob->start();
        return;
    } else {
        declareSessionReady(nsJob->session());
    }
}

void SessionPool::onIdDone(KJob *job)
{
    auto idJob = qobject_cast<KIMAP::IdJob *>(job);
    // Can happen if we disconnected meanwhile
    if (!m_connectingPool.contains(idJob->session())) {
        Q_EMIT connectDone(CancelledError, i18n("Disconnected during login."));
        return;
    }
    declareSessionReady(idJob->session());
}

void SessionPool::onConnectionLost()
{
    auto session = static_cast<KIMAP::Session *>(sender());

    m_unusedPool.removeAll(session);
    m_reservedPool.removeAll(session);
    m_connectingPool.removeAll(session);

    if (m_unusedPool.isEmpty() && m_reservedPool.isEmpty()) {
        m_passwordRequester->cancelPasswordRequests();
        delete m_account;
        m_account = nullptr;
        m_namespaces.clear();
        m_capabilities.clear();

        m_initialConnectDone = false;
    }

    Q_EMIT connectionLost(session);

    if (!m_pendingRequests.isEmpty()) {
        cancelSessionCreation(nullptr, CouldNotConnectError, QString());
    }

    session->deleteLater();
    if (session == m_pendingInitialSession) {
        m_pendingInitialSession = nullptr;
    }
}

void SessionPool::onSessionDestroyed(QObject *object)
{
    // Safety net for bugs that cause dangling session pointers
    auto session = static_cast<KIMAP::Session *>(object);
    bool sessionInPool = false;
    if (m_unusedPool.contains(session)) {
        qCWarning(IMAPRESOURCE_LOG) << "Session" << object << "destroyed while still in unused pool!";
        m_unusedPool.removeAll(session);
        sessionInPool = true;
    }
    if (m_reservedPool.contains(session)) {
        qCWarning(IMAPRESOURCE_LOG) << "Session" << object << "destroyed while still in reserved pool!";
        m_reservedPool.removeAll(session);
        sessionInPool = true;
    }
    if (m_connectingPool.contains(session)) {
        qCWarning(IMAPRESOURCE_LOG) << "Session" << object << "destroyed while still in connecting pool!";
        m_connectingPool.removeAll(session);
        sessionInPool = true;
    }
    Q_ASSERT(!sessionInPool);
}
