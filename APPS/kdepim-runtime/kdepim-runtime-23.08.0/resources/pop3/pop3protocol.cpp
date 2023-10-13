/*
 * SPDX-FileCopyrightText: 1999-2001 Alex Zepeda <zipzippy@sonic.net>
 * SPDX-FileCopyrightText: 2001-2002 Michael Haeckel <haeckel@kde.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */
#include "pop3protocol.h"
#include "settings.h"

extern "C" {
#include <sasl/sasl.h>
}

#include "pop3protocol_debug.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QNetworkProxy>
#include <QRegularExpression>

#include <KLocalizedString>

#include <MailTransport/Transport>

#include <QSslCipher>
#include <QSslSocket>
#include <QThread>
#include <cstring>

#include <KSslErrorUiData>

#define GREETING_BUF_LEN 1024
#define MAX_RESPONSE_LEN 512
#define MAX_COMMANDS 10

static const int s_connectTimeout = 20 * 1000; // 20 sec

static const sasl_callback_t callbacks[] = {
    {SASL_CB_ECHOPROMPT, nullptr, nullptr},
    {SASL_CB_NOECHOPROMPT, nullptr, nullptr},
    {SASL_CB_GETREALM, nullptr, nullptr},
    {SASL_CB_USER, nullptr, nullptr},
    {SASL_CB_AUTHNAME, nullptr, nullptr},
    {SASL_CB_PASS, nullptr, nullptr},
    {SASL_CB_CANON_USER, nullptr, nullptr},
    {SASL_CB_LIST_END, nullptr, nullptr},
};

bool POP3Protocol::initSASL() // static
{
#ifdef Q_OS_WIN32 // krazy:exclude=cpp
#if 0
    QByteArray libInstallPath(QFile::encodeName(QDir::toNativeSeparators(KGlobal::dirs()->installPath("lib") + QLatin1String("sasl2"))));
    QByteArray configPath(QFile::encodeName(QDir::toNativeSeparators(KGlobal::dirs()->installPath("config") + QLatin1String("sasl2"))));
    if (sasl_set_path(SASL_PATH_TYPE_PLUGIN, libInstallPath.data()) != SASL_OK
        || sasl_set_path(SASL_PATH_TYPE_CONFIG, configPath.data()) != SASL_OK) {
        fprintf(stderr, "SASL path initialization failed!\n");
        return false;
    }
#endif
#endif

    if (sasl_client_init(nullptr) != SASL_OK) {
        fprintf(stderr, "SASL library initialization failed!\n");
        return false;
    }
    return true;
}

POP3Protocol::POP3Protocol(const Settings &settings, const QString &password)
    : mSettings(settings)
    , mSocket(new QSslSocket(this))
{
    qCDebug(POP3_LOG);
    // m_cmd = CMD_NONE;
    supports_apop = false;
    m_try_apop = true;
    m_try_sasl = true;
    readBufferLen = 0;

    m_sServer = mSettings.host();
    m_iPort = mSettings.port();
    m_sUser = mSettings.login();
    m_sPass = password;
}

POP3Protocol::~POP3Protocol()
{
    qCDebug(POP3_LOG);
    closeConnection();

    sasl_done();
}

qint64 POP3Protocol::myRead(void *data, qint64 len)
{
    if (readBufferLen) {
        qint64 copyLen = (len < readBufferLen) ? len : readBufferLen;
        memcpy(data, readBuffer, copyLen);
        readBufferLen -= copyLen;
        if (readBufferLen) {
            memmove(readBuffer, &readBuffer[copyLen], readBufferLen);
        }
        return copyLen;
    }
    if (!mSocket->bytesAvailable()) {
        mSocket->waitForReadyRead(600 * 1000);
    }
    return mSocket->read((char *)data, len);
}

qint64 POP3Protocol::myReadLine(char *data, qint64 len)
{
    qint64 copyLen = 0, readLen = 0;
    while (true) {
        while (copyLen < readBufferLen && readBuffer[copyLen] != '\n') {
            copyLen++;
        }
        if (copyLen < readBufferLen || copyLen == len) {
            copyLen++;
            memcpy(data, readBuffer, copyLen);
            data[copyLen] = '\0';
            readBufferLen -= copyLen;
            if (readBufferLen) {
                memmove(readBuffer, &readBuffer[copyLen], readBufferLen);
            }
            return copyLen;
        }
        if (!mSocket->bytesAvailable()) {
            mSocket->waitForReadyRead(600 * 1000);
        }
        readLen = mSocket->read(&readBuffer[readBufferLen], len - readBufferLen);
        readBufferLen += readLen;
        if (readLen <= 0) {
            data[0] = '\0';
            return 0;
        }
    }
}

POP3Protocol::Resp POP3Protocol::getResponse(char *r_buf, unsigned int r_len)
{
    char *buf = nullptr;
    unsigned int recv_len = 0;
    // fd_set FDs;

    // Give the buffer the appropriate size
    r_len = r_len ? r_len : MAX_RESPONSE_LEN;

    buf = new char[r_len];

    // Clear out the buffer
    memset(buf, 0, r_len);
    myReadLine(buf, r_len - 1);
    // qCDebug(POP3_LOG) << "S:" << buf;

    // This is really a funky crash waiting to happen if something isn't
    // null terminated.
    recv_len = strlen(buf);

    /*
     *   From rfc1939:
     *
     *   Responses in the POP3 consist of a status indicator and a keyword
     *   possibly followed by additional information.  All responses are
     *   terminated by a CRLF pair.  Responses may be up to 512 characters
     *   long, including the terminating CRLF.  There are currently two status
     *   indicators: positive ("+OK") and negative ("-ERR").  Servers MUST
     *   send the "+OK" and "-ERR" in upper case.
     */

    if (strncmp(buf, "+OK", 3) == 0) {
        if (r_buf && r_len) {
            memcpy(r_buf, (buf[3] == ' ' ? buf + 4 : buf + 3), qMin(r_len, (buf[3] == ' ' ? recv_len - 4 : recv_len - 3)));
        }

        delete[] buf;

        return Ok;
    } else if (strncmp(buf, "-ERR", 4) == 0) {
        if (r_buf && r_len) {
            memcpy(r_buf, (buf[4] == ' ' ? buf + 5 : buf + 4), qMin(r_len, (buf[4] == ' ' ? recv_len - 5 : recv_len - 4)));
        }

        QString serverMsg = QString::fromLatin1(buf).mid(5).trimmed();

        m_sError = i18n("The server said: \"%1\"", serverMsg);

        delete[] buf;

        return Err;
    } else if (strncmp(buf, "+ ", 2) == 0) {
        if (r_buf && r_len) {
            memcpy(r_buf, buf + 2, qMin(r_len, recv_len - 4));
            r_buf[qMin(r_len - 1, recv_len - 4)] = '\0';
        }

        delete[] buf;

        return Cont;
    } else {
        qCDebug(POP3_LOG) << "Invalid POP3 response received!";

        if (r_buf && r_len) {
            memcpy(r_buf, buf, qMin(r_len, recv_len));
        }

        if (!*buf) {
            m_sError = i18n("The server terminated the connection.");
        } else {
            m_sError = i18n("Invalid response from server:\n\"%1\"", QLatin1String(buf));
        }

        delete[] buf;

        return Invalid;
    }
}

bool POP3Protocol::sendCommand(const QByteArray &cmd)
{
    /*
     *   From rfc1939:
     *
     *   Commands in the POP3 consist of a case-insensitive keyword, possibly
     *   followed by one or more arguments.  All commands are terminated by a
     *   CRLF pair.  Keywords and arguments consist of printable ASCII
     *   characters.  Keywords and arguments are each separated by a single
     *   SPACE character.  Keywords are three or four characters long. Each
     *   argument may be up to 40 characters long.
     */

    if (mSocket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }

    QByteArray cmdrn = cmd + "\r\n";

    // Show the command line the client sends, but make sure the password
    // doesn't show up in the debug output
    QByteArray debugCommand = cmd;
    if (!m_sPass.isEmpty()) {
        debugCommand.replace(m_sPass.toLatin1(), "<password>");
    }
    // qCDebug(POP3_LOG) << "C:" << debugCommand;

    // Now actually write the command to the socket
    if (mSocket->write(cmdrn.data(), cmdrn.size()) != static_cast<qint64>(cmdrn.size())) {
        m_sError = i18n("Could not send to server.\n");
        return false;
    }
    if (mSocket->waitForBytesWritten(-1)) {
        m_sError = i18n("Could not send to server.\n");
        return false;
    }
    mSocket->flush(); // this is supposed to get the data on the wire faster

    return mSocket->state() == QAbstractSocket::ConnectedState;
}

POP3Protocol::Resp POP3Protocol::command(const QByteArray &cmd, char *recv_buf, unsigned int len)
{
    sendCommand(cmd);
    return getResponse(recv_buf, len);
}

static bool useSASL(const Settings &settings)
{
    const int type = settings.authenticationMethod();
    switch (type) {
    case MailTransport::Transport::EnumAuthenticationType::PLAIN:
    case MailTransport::Transport::EnumAuthenticationType::LOGIN:
    case MailTransport::Transport::EnumAuthenticationType::CRAM_MD5:
    case MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5:
    case MailTransport::Transport::EnumAuthenticationType::NTLM:
    case MailTransport::Transport::EnumAuthenticationType::GSSAPI:
        return true;
    default:
        return false;
    }
}

static QString saslAuthTypeString(const Settings &settings)
{
    switch (settings.authenticationMethod()) {
    case MailTransport::Transport::EnumAuthenticationType::LOGIN:
        return QStringLiteral("LOGIN");
    case MailTransport::Transport::EnumAuthenticationType::PLAIN:
        return QStringLiteral("PLAIN");
    case MailTransport::Transport::EnumAuthenticationType::CRAM_MD5:
        return QStringLiteral("CRAM-MD5");
    case MailTransport::Transport::EnumAuthenticationType::DIGEST_MD5:
        return QStringLiteral("DIGEST-MD5");
    case MailTransport::Transport::EnumAuthenticationType::GSSAPI:
        return QStringLiteral("GSSAPI");
    case MailTransport::Transport::EnumAuthenticationType::NTLM:
        return QStringLiteral("NTLM");
    case MailTransport::Transport::EnumAuthenticationType::CLEAR:
        return QStringLiteral("USER");
    case MailTransport::Transport::EnumAuthenticationType::APOP:
        return QStringLiteral("APOP");
    default:
        break;
    }
    return {};
}

void POP3Protocol::closeConnection()
{
    if (!mConnected) {
        return;
    }

    // Send a QUIT and wait for a response.
    // We don't care if it's positive or negative.
    command("QUIT");

    mSocket->disconnectFromHost();
    if (mSocket->state() != QAbstractSocket::UnconnectedState) {
        mSocket->waitForDisconnected(-1); // wait for unsent data to be sent
    }
    mSocket->close();

    readBufferLen = 0;
    mConnected = false;
}

Result POP3Protocol::loginAPOP(const char *challenge)
{
    char buf[512];
    memset(buf, 0, sizeof(buf));

    // Generate digest
    QCryptographicHash ctx(QCryptographicHash::Md5);
    // qCDebug(POP3_LOG) << "APOP challenge: " << challenge;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ctx.addData(challenge, strlen(challenge));
#else
    ctx.addData(QByteArray(challenge, strlen(challenge)));
#endif
    ctx.addData(m_sPass.toLatin1());

    // Genenerate APOP command
    const QByteArray apop_string = "APOP " + m_sUser.toLocal8Bit() + ' ' + ctx.result().toHex();
    if (command(apop_string, buf, sizeof(buf)) == Ok) {
        return Result::pass();
    }

    qCDebug(POP3_LOG) << "Could not login via APOP. Falling back to USER/PASS";
    closeConnection();
    if (mSettings.authenticationMethod() == MailTransport::Transport::EnumAuthenticationType::APOP) {
        return Result::fail(
            ERR_CANNOT_LOGIN,
            i18n("Login via APOP failed. The server %1 may not support APOP, although it claims to support it, or the password may be wrong.\n\n%2",
                 m_sServer,
                 m_sError));
    }
    return Result::fail(ERR_LOGIN_FAILED_TRY_FALLBACKS, {});
}

bool POP3Protocol::saslInteract(void *in)
{
    qCDebug(POP3_LOG);
    auto interact = (sasl_interact_t *)in;
    while (interact->id != SASL_CB_LIST_END) {
        qCDebug(POP3_LOG) << "SASL_INTERACT id: " << interact->id;
        switch (interact->id) {
        case SASL_CB_USER:
        case SASL_CB_AUTHNAME:
            qCDebug(POP3_LOG) << "SASL_CB_[USER|AUTHNAME]: " << m_sUser;
            interact->result = strdup(m_sUser.toUtf8().constData());
            interact->len = strlen((const char *)interact->result);
            break;
        case SASL_CB_PASS:
            qCDebug(POP3_LOG) << "SASL_CB_PASS: [hidden] ";
            interact->result = strdup(m_sPass.toUtf8().constData());
            interact->len = strlen((const char *)interact->result);
            break;
        default:
            interact->result = nullptr;
            interact->len = 0;
            break;
        }
        interact++;
    }
    return true;
}

Result POP3Protocol::loginSASL()
{
    char buf[512];

    sasl_conn_t *conn = nullptr;
    sasl_interact_t *client_interact = nullptr;
    const char *out = nullptr;
    uint outlen;
    const char *mechusing = nullptr;
    Resp resp;

    int result = sasl_client_new("pop", m_sServer.toLatin1().constData(), nullptr, nullptr, callbacks, 0, &conn);

    auto saslError = [&]() {
        closeConnection();
        return Result::fail(ERR_SASL_FAILURE, i18n("An error occurred during authentication: %1", QString::fromUtf8(sasl_errdetail(conn))));
    };

    if (result != SASL_OK) {
        qCDebug(POP3_LOG) << "sasl_client_new failed with: " << result;
        return saslError();
    }

    // We need to check what methods the server supports...
    // This is based on RFC 1734's wisdom
    const bool userRequestedSASL = useSASL(mSettings);
    if (userRequestedSASL || command("AUTH") == Ok) {
        QStringList sasl_list;
        if (userRequestedSASL) {
            sasl_list.append(saslAuthTypeString(mSettings));
        } else {
            while (true /* !AtEOF() */) {
                memset(buf, 0, sizeof(buf));
                myReadLine(buf, sizeof(buf) - 1);

                // HACK: This assumes fread stops at the first \n and not \r
                if ((buf[0] == 0) || (strcmp(buf, ".\r\n") == 0)) {
                    break; // End of data
                }
                // sanders, changed -2 to -1 below
                buf[strlen(buf) - 2] = '\0';

                sasl_list.append(QLatin1String(buf));
            }
        }

        do {
            result = sasl_client_start(conn, sasl_list.join(QLatin1Char(' ')).toLatin1().constData(), &client_interact, &out, &outlen, &mechusing);

            if (result == SASL_INTERACT) {
                if (!saslInteract(client_interact)) {
                    closeConnection();
                    sasl_dispose(&conn);
                    return Result::fail(ERR_USER_CANCELED, QString());
                }
            }
        } while (result == SASL_INTERACT);
        if (result != SASL_CONTINUE && result != SASL_OK) {
            qCDebug(POP3_LOG) << "sasl_client_start failed with: " << result;
            sasl_dispose(&conn);
            return saslError();
        }

        qCDebug(POP3_LOG) << "Preferred authentication method is " << mechusing << ".";

        QByteArray msg, tmp;

        QString firstCommand = QLatin1String("AUTH ") + QString::fromLatin1(mechusing);
        msg = QByteArray::fromRawData(out, outlen).toBase64();
        if (!msg.isEmpty()) {
            firstCommand += QLatin1Char(' ');
            firstCommand += QString::fromLatin1(msg.data(), msg.size());
        }

        tmp.resize(2049);
        resp = command(firstCommand.toLatin1(), tmp.data(), 2049);
        while (resp == Cont) {
            tmp.resize(tmp.indexOf((char)0));
            msg = QByteArray::fromBase64(tmp);
            do {
                result = sasl_client_step(conn, msg.isEmpty() ? nullptr : msg.data(), msg.size(), &client_interact, &out, &outlen);

                if (result == SASL_INTERACT) {
                    if (!saslInteract(client_interact)) {
                        closeConnection();
                        sasl_dispose(&conn);
                        return Result::fail(ERR_USER_CANCELED, QString());
                    }
                }
            } while (result == SASL_INTERACT);
            if (result != SASL_CONTINUE && result != SASL_OK) {
                qCDebug(POP3_LOG) << "sasl_client_step failed with: " << result;
                sasl_dispose(&conn);
                return saslError();
            }

            msg = QByteArray::fromRawData(out, outlen).toBase64();
            tmp.resize(2049);
            resp = command(msg, tmp.data(), 2049);
        }

        sasl_dispose(&conn);
        if (resp == Ok) {
            qCDebug(POP3_LOG) << "SASL authenticated";
            return Result::pass();
        }

        if (useSASL(mSettings)) {
            closeConnection();
            const QString errorString = i18n("Login via SASL (%1) failed. The server may not support %2, or the password may be wrong.\n\n%3",
                                             QLatin1String(mechusing),
                                             QLatin1String(mechusing),
                                             m_sError);
            return Result::fail(ERR_CANNOT_LOGIN, errorString);
        }
    }

    if (userRequestedSASL) {
        closeConnection();
        const QString errorString = i18n(
            "Your POP3 server (%1) does not support SASL.\n"
            "Choose a different authentication method.",
            m_sServer);
        return Result::fail(ERR_CANNOT_LOGIN, errorString);
    }
    return Result::fail(ERR_LOGIN_FAILED_TRY_FALLBACKS, {});
}

Result POP3Protocol::loginPASS()
{
    char buf[512];

    const QString userCommand = QStringLiteral("USER ") + m_sUser;

    if (command(userCommand.toLocal8Bit(), buf, sizeof(buf)) != Ok) {
        qCDebug(POP3_LOG) << "Could not login. Bad username Sorry";
        closeConnection();
        const QString errorString = i18n("Could not login to %1.\n\n", m_sServer) + m_sError;
        return Result::fail(ERR_CANNOT_LOGIN, errorString);
    }

    const QString passwordCommand = QStringLiteral("PASS ") + m_sPass;
    if (command(passwordCommand.toLocal8Bit(), buf, sizeof(buf)) != Ok) {
        qCDebug(POP3_LOG) << "Could not login. Bad password Sorry.";
        closeConnection();
        const QString errorString = i18n("Could not login to %1. The password may be wrong.\n\n%2", m_sServer, m_sError);
        return Result::fail(ERR_CANNOT_LOGIN, errorString);
    }
    qCDebug(POP3_LOG) << "USER/PASS login succeeded";
    return Result::pass();
}

Result POP3Protocol::startSsl()
{
    mSocket->ignoreSslErrors(); // Don't worry, errors are handled manually below
    mSocket->startClientEncryption();
    const bool encryptionStarted = mSocket->waitForEncrypted(s_connectTimeout);

    const QSslCipher cipher = mSocket->sessionCipher();
    const QList<QSslError> errors = mSocket->sslHandshakeErrors();
    if (!encryptionStarted || !errors.isEmpty() || !mSocket->isEncrypted() || cipher.isNull() || cipher.usedBits() == 0) {
        QString errorString = std::accumulate(errors.begin(), errors.end(), QString(), [](QString cur, const QSslError &error) {
            if (!cur.isEmpty())
                cur += QLatin1Char('\n');
            cur += error.errorString();
            return cur;
        });

        qCDebug(POP3_LOG) << "Initial SSL handshake failed. cipher.isNull() is" << cipher.isNull() << ", cipher.usedBits() is" << cipher.usedBits()
                          << ", the socket says:" << mSocket->errorString() << "and the SSL errors are:" << errorString;
        mContinueAfterSslError = false;
        Q_EMIT sslError(KSslErrorUiData(mSocket));
        if (!mContinueAfterSslError) {
            if (errorString.isEmpty())
                errorString = mSocket->errorString();
            qCDebug(POP3_LOG) << "TLS setup has failed. Aborting." << errorString;
            closeConnection();
            return Result::fail(ERR_SSL_FAILURE, i18n("SSL/TLS error: %1", errorString));
        }
    } else {
        qCDebug(POP3_LOG) << "TLS has been enabled.";
    }
    return Result::pass();
}

Result POP3Protocol::openConnection()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    m_try_apop = mSettings.authenticationMethod() == MailTransport::Transport::EnumAuthenticationType::APOP;
    m_try_sasl = useSASL(mSettings);

    qCDebug(POP3_LOG);

    if (!mSettings.useProxy()) {
        qCDebug(POP3_LOG) << "requested to use no proxy";
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        mSocket->setProxy(proxy);
    } // ## and what if useProxy() is true?

    char *greeting_buf;
    do {
        closeConnection();

        mSocket->setPeerVerifyName(m_sServer); // Used for ssl certificate verification (SNI)
        mSocket->connectToHost(m_sServer, m_iPort);
        if (!mSocket->waitForConnected(s_connectTimeout)) {
            const QString errorString = i18n("%1: %2", m_sServer, mSocket->errorString());
            return Result::fail(mSocket->error(), errorString);
        }

        if (mSettings.useSSL()) {
            const Result res = startSsl();
            if (!res.success) {
                return res;
            }
        }

        mConnected = true;

        greeting_buf = new char[GREETING_BUF_LEN];
        memset(greeting_buf, 0, GREETING_BUF_LEN);

        // If the server doesn't respond with a greeting
        if (getResponse(greeting_buf, GREETING_BUF_LEN) != Ok) {
            const QString errorString = i18n("Could not login to %1.\n\n", m_sServer)
                + ((!greeting_buf || !*greeting_buf) ? i18n("The server terminated the connection immediately.")
                                                     : i18n("Server does not respond properly:\n%1\n", QLatin1String(greeting_buf)));
            delete[] greeting_buf;
            closeConnection();
            // we've got major problems, and possibly the wrong port
            return Result::fail(ERR_CANNOT_LOGIN, errorString);
        }
        QString greeting = QLatin1String(greeting_buf);
        delete[] greeting_buf;

        if (!greeting.isEmpty()) {
            greeting.chop(2);
        }

        // Does the server support APOP?
        // QString apop_cmd;
        const QRegularExpression re(QStringLiteral("<[A-Za-z0-9\\.\\-_]+@[A-Za-z0-9\\.\\-_]+>$"), QRegularExpression::CaseInsensitiveOption);

        qCDebug(POP3_LOG) << "greeting: " << greeting;
        const int apop_pos = greeting.indexOf(re);
        supports_apop = (apop_pos != -1);

        if (m_try_apop && !supports_apop) {
            closeConnection();
            return Result::fail(ERR_CANNOT_LOGIN,
                                i18n("Your POP3 server (%1) does not support APOP.\n"
                                     "Choose a different authentication method.",
                                     m_sServer));
        }

        // Try to go into TLS mode
        if (mSettings.useTLS()) {
            if (command("STLS") != Ok) {
                closeConnection();
                return Result::fail(ERR_SSL_FAILURE,
                                    i18n("Your POP3 server claims to "
                                         "support TLS but negotiation "
                                         "was unsuccessful.\nYou can "
                                         "disable TLS in the POP account settings dialog."));
            }
            const Result res = startSsl();
            if (!res.success) {
                return res;
            }
        }

        if (supports_apop && m_try_apop) {
            qCDebug(POP3_LOG) << "Trying APOP";
            const Result retval = loginAPOP(greeting.toLatin1().data() + apop_pos);
            if (retval.success || retval.error != ERR_LOGIN_FAILED_TRY_FALLBACKS) {
                return retval;
            }
            m_try_apop = false;
        } else if (m_try_sasl) {
            qCDebug(POP3_LOG) << "Trying SASL";
            const Result retval = loginSASL();
            if (retval.success || retval.error != ERR_LOGIN_FAILED_TRY_FALLBACKS) {
                return retval;
            }
            m_try_sasl = false;
        } else {
            // Fall back to conventional USER/PASS scheme
            qCDebug(POP3_LOG) << "Trying USER/PASS";
            return loginPASS();
        }
    } while (true);
}

size_t POP3Protocol::realGetSize(unsigned int msg_num)
{
    char *buf;
    QByteArray cmd;
    size_t ret = 0;

    buf = new char[MAX_RESPONSE_LEN];
    memset(buf, 0, MAX_RESPONSE_LEN);
    cmd = "LIST " + QByteArray::number(msg_num);
    if (command(cmd, buf, MAX_RESPONSE_LEN) != Ok) {
        delete[] buf;
        return 0;
    } else {
        cmd = buf;
        cmd.remove(0, cmd.indexOf(" "));
        ret = cmd.toLong();
    }
    delete[] buf;
    return ret;
}

Result POP3Protocol::get(const QString &_commandString)
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    qCDebug(POP3_LOG) << _commandString;
    // List of supported commands
    //
    // commandString  Command   Result
    // /index         LIST      List message sizes
    // /uidl          UIDL      List message UIDs
    // /remove/#1     DELE #1   Mark a message for deletion
    // /download/#1   RETR #1   Get message header and body
    // /list/#1       LIST #1   Get size of a message
    // /uid/#1        UIDL #1   Get UID of a message
    // /quit          QUIT      Send QUIT, close connection
    // /headers/#1    TOP #1    Get header of message
    //
    // Notes:
    // Sizes are in bytes.
    // No support for the STAT command has been implemented.

    Q_ASSERT(_commandString.startsWith(QLatin1Char('/')));
    const QString commandString = _commandString.mid(1);
    Q_ASSERT(!commandString.isEmpty());

    char buf[MAX_PACKET_LEN];
    char destbuf[MAX_PACKET_LEN];
    const int maxCommands = mSettings.pipelining() ? MAX_COMMANDS : 1;

    if (((commandString.indexOf(QLatin1Char('/')) == -1) && (commandString != QLatin1String("index")) && (commandString != QLatin1String("uidl"))
         && (commandString != QLatin1String("quit")))) {
        return Result::fail(ERR_INTERNAL, i18n("Internal error: missing argument for command %1", commandString));
    }

    const int slashPos = commandString.indexOf(QLatin1Char('/'));
    const QString cmd = commandString.left(slashPos);
    const QString path = commandString.mid(slashPos + 1);

    if ((cmd == QLatin1String("index")) || (cmd == QLatin1String("uidl"))) {
        bool result;
        if (cmd == QLatin1String("index")) {
            result = (command("LIST") == Ok);
        } else {
            result = (command("UIDL") == Ok);
        }

        /*
           LIST
           +OK Mailbox scan listing follows
           1 2979
           2 1348
           .
         */
        if (result) {
            while (true /* !AtEOF() */) {
                memset(buf, 0, sizeof(buf));
                myReadLine(buf, sizeof(buf) - 1);

                // HACK: This assumes fread stops at the first \n and not \r
                if ((buf[0] == 0) || (strcmp(buf, ".\r\n") == 0)) {
                    break; // End of data
                }
                // sanders, changed -2 to -1 below
                int bufStrLen = strlen(buf);
                buf[bufStrLen - 2] = '\0';
                Q_EMIT data(QByteArray(buf, bufStrLen));
            }
        }
        qCDebug(POP3_LOG) << "Finishing up list";
    } else if (cmd == QLatin1String("remove")) {
        const QStringList waitingCommands = path.split(QLatin1Char(','));
        int activeCommands = 0;
        QStringList::ConstIterator it = waitingCommands.begin();
        while (it != waitingCommands.end() || activeCommands > 0) {
            while (activeCommands < maxCommands && it != waitingCommands.end()) {
                sendCommand((QLatin1String("DELE ") + *it).toLatin1());
                activeCommands++;
                it++;
            }
            getResponse(buf, sizeof(buf) - 1);
            activeCommands--;
        }
    } else if (cmd == QLatin1String("download") || cmd == QLatin1String("headers")) {
        const QStringList waitingCommands = path.split(QLatin1Char(','), Qt::SkipEmptyParts);
        if (waitingCommands.isEmpty()) {
            qCDebug(POP3_LOG) << "tried to request" << cmd << "for" << path << "with no specific item to get";
            closeConnection();
            return Result::fail(ERR_INTERNAL, i18n("Internal error: missing item"));
        }
        memset(buf, 0, sizeof(buf));

        int activeCommands = 0;
        QStringList::ConstIterator it = waitingCommands.begin();
        while (it != waitingCommands.end() || activeCommands > 0) {
            while (activeCommands < maxCommands && it != waitingCommands.end()) {
                sendCommand(QString((cmd == QLatin1String("headers")) ? QString(QLatin1String("TOP ") + *it + QLatin1String(" 0"))
                                                                      : QString(QLatin1String("RETR ") + *it))
                                .toLatin1());
                activeCommands++;
                it++;
            }
            if (getResponse(buf, sizeof(buf) - 1) == Ok) {
                activeCommands--;
                // totalSize(msg_len);
                memset(buf, 0, sizeof(buf));
                char ending = '\n';
                bool endOfMail = false;
                bool eat = false;
                while (true /* !AtEOF() */) {
                    qint64 readlen = myRead(buf, sizeof(buf) - 1);
                    if (readlen <= 0) {
                        const bool wasConnected = (mSocket->state() == QAbstractSocket::ConnectedState);
                        closeConnection();
                        if (wasConnected) {
                            return Result::fail(ERR_DISCONNECTED, i18n("Timeout from POP3 server %1", m_sServer));
                        } else {
                            return Result::fail(ERR_DISCONNECTED, i18n("Connection broken from POP3 server %1", m_sServer));
                        }
                    }
                    if (ending == '.' && readlen > 1 && buf[0] == '\r' && buf[1] == '\n') {
                        readBufferLen = readlen - 2;
                        memcpy(readBuffer, &buf[2], readBufferLen);
                        break;
                    }
                    bool newline = (ending == '\n');

                    if (buf[readlen - 1] == '\n') {
                        ending = '\n';
                    } else if (buf[readlen - 1] == '.' && ((readlen > 1) ? buf[readlen - 2] == '\n' : ending == '\n')) {
                        ending = '.';
                    } else {
                        ending = ' ';
                    }

                    char *buf1 = buf, *buf2 = destbuf;
                    // ".." at start of a line means only "."
                    // "." means end of data
                    for (qint64 i = 0; i < readlen; i++) {
                        if (*buf1 == '\r' && eat) {
                            endOfMail = true;
                            if (i == readlen - 1 /* && !AtEOF() */) {
                                myRead(buf, 1);
                            } else if (i < readlen - 2) {
                                readBufferLen = readlen - i - 2;
                                memcpy(readBuffer, &buf[i + 2], readBufferLen);
                            }
                            break;
                        } else if (*buf1 == '\n') {
                            newline = true;
                            eat = false;
                        } else if (*buf1 == '.' && newline) {
                            newline = false;
                            eat = true;
                        } else {
                            newline = false;
                            eat = false;
                        }
                        if (!eat) {
                            *buf2 = *buf1;
                            buf2++;
                        }
                        buf1++;
                    }

                    if (buf2 > destbuf) {
                        Q_EMIT data(QByteArray(destbuf, buf2 - destbuf));
                    }

                    if (endOfMail) {
                        break;
                    }
                }
                Q_EMIT messageComplete();
            } else {
                qCDebug(POP3_LOG) << "Could not login. Bad RETR Sorry";
                closeConnection();
                return Result::fail(ERR_DISCONNECTED,
                                    i18n("Error during communication with the POP3 server while "
                                         "trying to download mail: %1",
                                         m_sError));
            }
        }
        qCDebug(POP3_LOG) << "Finishing up";
    } else if ((cmd == QLatin1String("uid")) || (cmd == QLatin1String("list"))) {
        bool ok = true;
        (void)path.toInt(&ok);
        if (!ok) {
            return Result::fail(ERR_INTERNAL, i18n("Internal error, number expected: %1", path));
        }

        QString commandStr;
        if (cmd == QLatin1String("uid")) {
            commandStr = QLatin1String("UIDL ") + path;
        } else {
            commandStr = QLatin1String("LIST ") + path;
        }

        memset(buf, 0, sizeof(buf));
        if (command(commandStr.toLatin1(), buf, sizeof(buf) - 1) == Ok) {
            const int len = strlen(buf);
            // totalSize(len);
            Q_EMIT data(QByteArray(buf, len));
            // processedSize(len);
            qCDebug(POP3_LOG) << buf;
            qCDebug(POP3_LOG) << "Finishing up uid";
        } else {
            closeConnection();
            return Result::fail(ERR_PROTOCOL, i18n("Unexpected response from POP3 server."));
        }
    } else if (cmd == QLatin1String("quit")) {
        qCDebug(POP3_LOG) << "Issued QUIT";
        closeConnection();
    }
    return Result::pass();
}

void POP3Protocol::setContinueAfterSslError(bool b)
{
    mContinueAfterSslError = b;
}
