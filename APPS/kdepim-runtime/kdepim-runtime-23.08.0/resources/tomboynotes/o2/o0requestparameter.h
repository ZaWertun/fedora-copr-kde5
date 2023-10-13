/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

/// Request parameter (name-value pair) participating in authentication.
struct O0RequestParameter {
    O0RequestParameter(const QByteArray &n, const QByteArray &v)
        : name(n)
        , value(v)
    {
    }

    bool operator<(const O0RequestParameter &other) const
    {
        return (name == other.name) ? (value < other.value) : (name < other.name);
    }

    QByteArray name;
    QByteArray value;
};
