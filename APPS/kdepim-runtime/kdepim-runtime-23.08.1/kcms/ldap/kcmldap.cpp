/*
  This file is part of libkldap.

  SPDX-FileCopyrightText: 2002-2009 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmldap.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KLDAP/LdapConfigureWidget>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(KCMLdap, "kcmldap.json")
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KCMLdap::KCMLdap(QWidget *parent, const QVariantList &args)
    : KCModule(parent)
    , mLdapConfigureWidget(new KLDAP::LdapConfigureWidget(this))
#else
KCMLdap::KCMLdap(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , mLdapConfigureWidget(new KLDAP::LdapConfigureWidget(widget()))
#endif
{
    setButtons(KCModule::Apply);
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto about = new KAboutData(QStringLiteral("kcmldap"),
                                i18n("kcmldap"),
                                QString(),
                                i18n("LDAP Server Settings"),
                                KAboutLicense::LGPL,
                                i18n("(c) 2009 - 2010 Tobias Koenig"));
    about->addAuthor(i18n("Tobias Koenig"), QString(), QStringLiteral("tokoe@kde.org"));
    setAboutData(about);
#endif
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto layout = new QVBoxLayout(this);
#else
    auto layout = new QVBoxLayout(widget());
#endif
    layout->setContentsMargins({});

    layout->addWidget(mLdapConfigureWidget);
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    connect(mLdapConfigureWidget, &KLDAP::LdapConfigureWidget::changed, this, &KCMLdap::changed);
#else
    connect(mLdapConfigureWidget, &KLDAP::LdapConfigureWidget::changed, this, &KCMLdap::markAsChanged);
#endif
}

KCMLdap::~KCMLdap() = default;

void KCMLdap::load()
{
    mLdapConfigureWidget->load();
}

void KCMLdap::save()
{
    mLdapConfigureWidget->save();
}

#include "kcmldap.moc"
