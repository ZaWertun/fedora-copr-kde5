/*
    SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kolabsettings.h"

KolabSettings::KolabSettings(WId winId)
    : Settings(winId)
{
    changeDefaults();
    load();
}

void KolabSettings::changeDefaults()
{
    setCurrentGroup(QStringLiteral("network"));
    KConfigSkeleton::ItemInt *itemImapPort = (KConfigSkeleton::ItemInt *)findItem(QStringLiteral("ImapPort"));
    itemImapPort->setDefaultValue(143);
    KConfigSkeleton::ItemString *itemSafety = (KConfigSkeleton::ItemString *)findItem(QStringLiteral("Safety"));
    itemSafety->setDefaultValue(QStringLiteral("STARTTLS"));
    KConfigSkeleton::ItemBool *itemSubscriptionEnabled = (KConfigSkeleton::ItemBool *)findItem(QStringLiteral("SubscriptionEnabled"));
    itemSubscriptionEnabled->setDefaultValue(true);

    setCurrentGroup(QStringLiteral("cache"));
    KConfigSkeleton::ItemBool *itemDisconnectedModeEnabled = (KConfigSkeleton::ItemBool *)findItem(QStringLiteral("DisconnectedModeEnabled"));
    itemDisconnectedModeEnabled->setDefaultValue(true);

    setCurrentGroup(QStringLiteral("siever"));
    KConfigSkeleton::ItemBool *itemSieveSupport = (KConfigSkeleton::ItemBool *)findItem(QStringLiteral("SieveSupport"));
    itemSieveSupport->setDefaultValue(true);
    KConfigSkeleton::ItemBool *itemSieveReuseConfig = (KConfigSkeleton::ItemBool *)findItem(QStringLiteral("SieveReuseConfig"));
    itemSieveReuseConfig->setDefaultValue(true);
}
