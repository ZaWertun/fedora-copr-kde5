/*
   SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMetaType>
#include <QString>

/**
 * Result type for returning error context.
 *
 * This is meant to be returned by functions that do not have a simple
 * error conditions that could be represented by returning a bool, or
 * when the contextual error string can only be correctly constructed
 * inside the function. When using the Result type always mark the
 * function Q_REQUIRED_RESULT to enforce handling of the Result.
 */
struct Result {
    bool success;
    int error;
    QString errorString;

    inline static Result fail(int _error, const QString &_errorString)
    {
        return Result{false, _error, _errorString};
    }

    inline static Result pass()
    {
        return Result{true, 0, QString()};
    }
};

Q_DECLARE_METATYPE(Result)
