/*
 * SPDX-FileCopyrightText: 1999, 2000 Alex Zepeda <zipzippy@sonic.net>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */

#pragma once

#include "result.h"

#include <KJob>

#include <QObject>

#include <sys/types.h>

class KSslErrorUiData;
class QSslSocket;
#define MAX_PACKET_LEN 4096

class Settings;

class POP3Protocol : public QObject
{
    Q_OBJECT
public:
    POP3Protocol(const Settings &settings, const QString &password);
    ~POP3Protocol() override;

    // clang-format off
    enum {
        ERR_LOGIN_FAILED_TRY_FALLBACKS = KJob::UserDefinedError + 1,
        ERR_SASL_FAILURE,
        ERR_SSL_FAILURE,
        ERR_CANNOT_LOGIN,
        ERR_USER_CANCELED,
        ERR_INTERNAL,
        ERR_PROTOCOL,
        ERR_DISCONNECTED,
    };
    // clang-format on

    static bool initSASL();

    /**
     * Attempt to initiate a POP3 connection via a TCP socket.
     */
    Q_REQUIRED_RESULT Result openConnection();

    /**
     *  Attempt to properly shut down the POP3 connection by sending
     *  "QUIT\r\n" before closing the socket.
     */
    void closeConnection();

    /**
     * Entry point for all features
     * TODO: this could be split up!
     */
    Q_REQUIRED_RESULT Result get(const QString &command);

    /**
     * Sets whether to continue or abort after a SSL error
     */
    void setContinueAfterSslError(bool b);

Q_SIGNALS:
    void sslError(const KSslErrorUiData &);
    void data(const QByteArray &data);
    void messageComplete();

private:
    qint64 myRead(void *data, qint64 len);
    qint64 myReadLine(char *data, qint64 len);

    /**
     * This returns the size of a message as a long integer.
     * This is useful as an internal member, because the "other"
     * getSize command will emit a signal, which would be harder
     * to trap when doing something like listing a directory.
     */
    size_t realGetSize(unsigned int msg_num);

    /**
     *  Send a command to the server. Using this function, getResponse
     *  has to be called separately.
     */
    bool sendCommand(const QByteArray &cmd);

    enum Resp {
        Err,
        Ok,
        Cont,
        Invalid,
    };

    /**
     *  Send a command to the server, and wait for the  one-line-status
     *  reply via getResponse.  Similar rules apply.  If no buffer is
     *  specified, no data is passed back.
     */
    Resp command(const QByteArray &buf, char *r_buf = nullptr, unsigned int r_len = 0);

    /**
     *  All POP3 commands will generate a response.  Each response will
     *  either be prefixed with a "+OK " or a "-ERR ".  The getResponse
     *  function will wait until there's data to be read, and then read in
     *  the first line (the response), and copy the response sans +OK/-ERR
     *  into a buffer (up to len bytes) if one was passed to it.
     */
    Resp getResponse(char *buf, unsigned int len);

    /**
     * Authenticate via APOP
     */
    Q_REQUIRED_RESULT Result loginAPOP(const char *challenge);

    bool saslInteract(void *in);
    /**
     * Authenticate via SASL
     */
    Q_REQUIRED_RESULT Result loginSASL();
    /**
     * Authenticate via traditional USER/PASS
     */
    Q_REQUIRED_RESULT Result loginPASS();

    Q_REQUIRED_RESULT Result startSsl();

    const Settings &mSettings;
    QSslSocket *const mSocket;
    unsigned short int m_iPort;
    QString m_sServer, m_sPass, m_sUser;
    bool m_try_apop, m_try_sasl, supports_apop;
    bool mConnected = false;
    bool mContinueAfterSslError = false;
    QString m_sError;
    char readBuffer[MAX_PACKET_LEN];
    qint64 readBufferLen;
};
