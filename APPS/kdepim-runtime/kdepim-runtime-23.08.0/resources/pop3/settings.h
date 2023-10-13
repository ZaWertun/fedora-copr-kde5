/*
   SPDX-FileCopyrightText: 2010 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "settingsbase.h"

#include <qwindowdefs.h>

/**
 * Extended settings class that allows setting the password over dbus, which is used by the
 * wizard.
 */
class Settings : public SettingsBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.POP3.Wallet")
public:
    enum class Option {
        NoOption = 0,
        ExportToDBus = 1,
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit Settings(const KSharedConfigPtr &config, Options options = Option::ExportToDBus);

    void setWindowId(WId id);
    void setResourceId(const QString &resourceIdentifier);

public Q_SLOTS:
    Q_SCRIPTABLE void setPassword(const QString &password);

private:
    WId mWinId;
    QString mResourceId;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Settings::Options)
