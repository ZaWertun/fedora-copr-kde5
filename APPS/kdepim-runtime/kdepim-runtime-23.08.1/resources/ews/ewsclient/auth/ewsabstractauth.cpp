/*
    SPDX-FileCopyrightText: 2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsabstractauth.h"

#include <QFile>

EwsAbstractAuth::EwsAbstractAuth(QObject *parent)
    : QObject(parent)
{
}

void EwsAbstractAuth::setAuthParentWidget(QWidget *widget)
{
    mAuthParentWidget = widget;
}

void EwsAbstractAuth::notifyRequestAuthFailed()
{
    Q_EMIT requestAuthFailed();
}

void EwsAbstractAuth::setPKeyAuthCertificateFiles(const QString &certFile, const QString &pkeyFile)
{
    if (QFile::exists(certFile) && QFile::exists(pkeyFile)) {
        mPKeyCertFile = certFile;
        mPKeyKeyFile = pkeyFile;
    }
}
