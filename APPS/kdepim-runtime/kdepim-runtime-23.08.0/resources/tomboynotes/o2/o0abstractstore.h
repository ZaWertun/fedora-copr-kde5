/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

#include <QObject>
#include <QString>

/// Storage for strings.
class O0AbstractStore : public QObject
{
    Q_OBJECT

public:
    explicit O0AbstractStore(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    /// Retrieve a string value by key.
    virtual QString value(const QString &key, const QString &defaultValue = QString()) = 0;

    /// Set a string value for a key.
    virtual void setValue(const QString &key, const QString &value) = 0;
};
