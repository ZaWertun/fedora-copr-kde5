/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <QDebug>
#include <QIODevice>
#include <QList>
#include <QString>

namespace Kolab
{
class DebugStream;
/**
 * Kolab Error Handler
 *
 * Errors are reported during an operation, but the operation might still succeed.
 * The error handler therefore contains all errors which occurred during a single operation,
 * and must be cleared at the start of a new operation.
 *
 * A user of the kolabobject classes should check ErrorHandler::error() after every operation.
 *
 * all non-const functions are not for the user of this class and only exist for internal usage.
 *
 * TODO: Hide everything which is not meant for the user from the interface.
 * FIXME: Use Threadlocal storage to make this threadsafe.
 */
class KOLAB_EXPORT ErrorHandler
{
public:
    enum Severity {
        Debug,
        Warning, // Warning, error could be corrected, object can be used without dataloss. This warning is also used if dataloss is acceptable because a
                 // feature is explicitly not supported.
        Error, // Potentially corrupt object, writing the object back could result in dataloss. (Object could still be used to display the data readonly).
        Critical // Critical error, produced object cannot be used and should be thrown away (writing back will result in dataloss).
    };

    struct Err {
        Err(Severity s, const QString &m, const QString &l)
            : severity(s)
            , message(m)
            , location(l)
        {
        }

        Severity severity;
        QString message;
        QString location;
    };

    static ErrorHandler &instance()
    {
        static ErrorHandler inst;
        return inst;
    }

    void addError(Severity s, const QString &message, const QString &location);
    const QList<Err> &getErrors() const;
    Severity error() const;
    QString errorMessage() const;
    void clear();

    /**
     * Check for errors during the libkolabxml reading/writing process and copy them into this error handler.
     */
    static void handleLibkolabxmlErrors();

    static void clearErrors()
    {
        ErrorHandler::instance().clear();
    }

    static bool errorOccured()
    {
        if (ErrorHandler::instance().error() >= Error) {
            return true;
        }
        return false;
    }

    /**
     * Returns a debug stream to which logs errors
     */
    static QDebug debugStream(Severity, int line, const char *file);

private:
    ErrorHandler();
    ErrorHandler(const ErrorHandler &) = delete;
    ErrorHandler &operator=(const ErrorHandler &) = delete;

    Severity m_worstError;
    QString m_worstErrorMessage;
    QList<Err> m_errorQueue;
    QScopedPointer<DebugStream> m_debugStream;
};

void logMessage(const QString &, const QString &, int, ErrorHandler::Severity s);

#define LOG(message) logMessage(message, __FILE__, __LINE__, ErrorHandler::Debug);
#define WARNING(message) logMessage(message, __FILE__, __LINE__, ErrorHandler::Warning);
#define ERROR(message) logMessage(message, __FILE__, __LINE__, ErrorHandler::Error);
#define CRITICAL(message) logMessage(message, QStringLiteral(__FILE__), __LINE__, ErrorHandler::Critical);

class DebugStream : public QIODevice
{
    Q_OBJECT
public:
    QString m_location;
    ErrorHandler::Severity m_severity;
    DebugStream();
    ~DebugStream() override;
    bool isSequential() const override
    {
        return true;
    }

    qint64 readData(char *, qint64) override
    {
        return 0; /* eof */
    }

    qint64 readLineData(char *, qint64) override
    {
        return 0; /* eof */
    }

    qint64 writeData(const char *data, qint64 len) override;

private:
    Q_DISABLE_COPY(DebugStream)
};

#define Debug() Kolab::ErrorHandler::debugStream(Kolab::ErrorHandler::Debug, __LINE__, __FILE__)
#define Warning() Kolab::ErrorHandler::debugStream(Kolab::ErrorHandler::Warning, __LINE__, __FILE__)
#define Error() Kolab::ErrorHandler::debugStream(Kolab::ErrorHandler::Error, __LINE__, __FILE__)
#define Critical() Kolab::ErrorHandler::debugStream(Kolab::ErrorHandler::Critical, __LINE__, __FILE__)
}

QDebug operator<<(QDebug dbg, const std::string &s);
