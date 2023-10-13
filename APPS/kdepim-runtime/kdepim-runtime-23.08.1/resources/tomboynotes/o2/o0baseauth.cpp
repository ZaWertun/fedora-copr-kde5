/*
    SPDX-License-Identifier: BSD-2-Clause
*/

#include "debug.h"
#include <QDataStream>
#include <QIODevice>

#include "o2/o0baseauth.h"
#include "o2/o0globals.h"
#include "o2/o0settingsstore.h"

static const quint16 DefaultLocalPort = 1965;

O0BaseAuth::O0BaseAuth(QObject *parent)
    : QObject(parent)
{
    localPort_ = DefaultLocalPort;
    store_ = new O0SettingsStore(QString::fromLatin1(O2_ENCRYPTION_KEY), this);
}

void O0BaseAuth::setStore(O0AbstractStore *store)
{
    if (store_) {
        store_->deleteLater();
    }
    if (store) {
        store_ = store;
        store_->setParent(this);
    } else {
        store_ = new O0SettingsStore(QString::fromLatin1(O2_ENCRYPTION_KEY), this);
        return;
    }
}

bool O0BaseAuth::linked()
{
    QString key = QString::fromLatin1(O2_KEY_LINKED).arg(clientId_);
    bool result = !store_->value(key).isEmpty();
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O0BaseAuth::linked:" << (result ? "Yes" : "No");
    return result;
}

void O0BaseAuth::setLinked(bool v)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O0BaseAuth::setLinked:" << (v ? "true" : "false");
    bool oldValue = linked();
    QString key = QString::fromLatin1(O2_KEY_LINKED).arg(clientId_);
    store_->setValue(key, v ? QStringLiteral("1") : QString());
    if (oldValue != v) {
        Q_EMIT linkedChanged();
    }
}

QString O0BaseAuth::tokenSecret()
{
    const QString key = QString::fromLatin1(O2_KEY_TOKEN_SECRET).arg(clientId_);
    return store_->value(key);
}

void O0BaseAuth::setTokenSecret(const QString &v)
{
    const QString key = QString::fromLatin1(O2_KEY_TOKEN_SECRET).arg(clientId_);
    store_->setValue(key, v);
    Q_EMIT tokenSecretChanged();
}

QString O0BaseAuth::token() const
{
    const QString key = QString::fromLatin1(O2_KEY_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O0BaseAuth::setToken(const QString &v)
{
    QString key = QString::fromLatin1(O2_KEY_TOKEN).arg(clientId_);
    store_->setValue(key, v);
    Q_EMIT tokenChanged();
}

QString O0BaseAuth::clientId() const
{
    return clientId_;
}

void O0BaseAuth::setClientId(const QString &value)
{
    clientId_ = value;
    Q_EMIT clientIdChanged();
}

QString O0BaseAuth::clientSecret() const
{
    return clientSecret_;
}

void O0BaseAuth::setClientSecret(const QString &value)
{
    clientSecret_ = value;
    Q_EMIT clientSecretChanged();
}

int O0BaseAuth::localPort() const
{
    return localPort_;
}

void O0BaseAuth::setLocalPort(int value)
{
    qCDebug(TOMBOYNOTESRESOURCE_LOG) << "O0BaseAuth::setLocalPort:" << value;
    localPort_ = value;
    Q_EMIT localPortChanged();
}

QVariantMap O0BaseAuth::extraTokens()
{
    QString key = QString::fromLatin1(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    QString value = store_->value(key);
    QByteArray bytes = QByteArray::fromBase64(value.toLatin1());
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    stream >> extraTokens_;
    return extraTokens_;
}

void O0BaseAuth::setExtraTokens(const QVariantMap &extraTokens)
{
    extraTokens_ = extraTokens;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << extraTokens;
    QString key = QString::fromLatin1(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    store_->setValue(key, QString::fromLatin1(bytes.toBase64()));
    Q_EMIT extraTokensChanged();
}

QByteArray O0BaseAuth::createQueryParameters(const QList<O0RequestParameter> &parameters)
{
    QByteArray ret;
    bool first = true;
    for (const O0RequestParameter &h : std::as_const(parameters)) {
        if (first) {
            first = false;
        } else {
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(QString::fromLatin1(h.name)) + "=" + QUrl::toPercentEncoding(QString::fromLatin1(h.value)));
    }
    return ret;
}
