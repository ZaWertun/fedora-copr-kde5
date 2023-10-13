/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#pragma once

#include <QSettings>
#include <QString>

#include "o2/o0abstractstore.h"
#include "o2/o0simplecrypt.h"

/// Persistent storage for authentication tokens, using QSettings.
class O0SettingsStore : public O0AbstractStore
{
    Q_OBJECT

public:
    /// Constructor
    explicit O0SettingsStore(const QString &encryptionKey, QObject *parent = nullptr);

    /// Construct with an explicit QSettings instance
    explicit O0SettingsStore(QSettings *settings, const QString &encryptionKey, QObject *parent = nullptr);

    /// Group key prefix
    Q_PROPERTY(QString groupKey READ groupKey WRITE setGroupKey NOTIFY groupKeyChanged)
    QString groupKey() const;
    void setGroupKey(const QString &groupKey);

    /// Get a string value for a key
    QString value(const QString &key, const QString &defaultValue = QString()) override;

    /// Set a string value for a key
    void setValue(const QString &key, const QString &value) override;

Q_SIGNALS:
    // Property change signals
    void groupKeyChanged();

protected:
    QSettings *settings_ = nullptr;
    QString groupKey_;
    O0SimpleCrypt crypt_;
};
