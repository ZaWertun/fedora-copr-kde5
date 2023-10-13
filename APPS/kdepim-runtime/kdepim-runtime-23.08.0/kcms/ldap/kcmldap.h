/*
  This file is part of libkldap.

  SPDX-FileCopyrightText: 2003-2009 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kcmutils_version.h"
#include <KCModule>
namespace KLDAP
{
class LdapConfigureWidget;
}

class KCMLdap : public KCModule
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMLdap(QWidget *parent, const QVariantList &args);
#else
    explicit KCMLdap(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif
    ~KCMLdap() override;

    void load() override;
    void save() override;

private:
    KLDAP::LdapConfigureWidget *const mLdapConfigureWidget;
};
