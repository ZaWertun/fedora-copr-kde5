/*
    SPDX-FileCopyrightText: 2012 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "keycache.h"
#include <QDirIterator>

KeyCache *KeyCache::mSelf = nullptr;

void KeyCache::addKeys(const QString &dir)
{
    if (!mNewKeys.contains(dir)) {
        mNewKeys.insert(dir, listNew(dir));
        // qCDebug(LIBMAILDIR_LOG) << "Added new keys for: " << dir;
    }

    if (!mCurKeys.contains(dir)) {
        mCurKeys.insert(dir, listCurrent(dir));
        // qCDebug(LIBMAILDIR_LOG) << "Added cur keys for: " << dir;
    }
}

void KeyCache::refreshKeys(const QString &dir)
{
    mNewKeys.remove(dir);
    mCurKeys.remove(dir);
    addKeys(dir);
}

void KeyCache::addNewKey(const QString &dir, const QString &key)
{
    mNewKeys[dir].insert(key);
    // qCDebug(LIBMAILDIR_LOG) << "Added new key for : " << dir << " key: " << key;
}

void KeyCache::addCurKey(const QString &dir, const QString &key)
{
    mCurKeys[dir].insert(key);
    // qCDebug(LIBMAILDIR_LOG) << "Added cur key for : " << dir << " key:" << key;
}

void KeyCache::removeKey(const QString &dir, const QString &key)
{
    // qCDebug(LIBMAILDIR_LOG) << "Removed new and cur key for: " << dir << " key:" << key;
    mNewKeys[dir].remove(key);
    mCurKeys[dir].remove(key);
}

bool KeyCache::isCurKey(const QString &dir, const QString &key) const
{
    return mCurKeys.value(dir).contains(key);
}

bool KeyCache::isNewKey(const QString &dir, const QString &key) const
{
    return mNewKeys.value(dir).contains(key);
}

QSet<QString> KeyCache::listNew(const QString &dir) const
{
    QSet<QString> keys;
    QDirIterator d(dir + QLatin1String("/new"), QDir::Files);
    while (d.hasNext()) {
        d.next();
        keys.insert(d.fileName());
    }
    return keys;
}

QSet<QString> KeyCache::listCurrent(const QString &dir) const
{
    QSet<QString> keys;
    QDirIterator d(dir + QLatin1String("/cur"), QDir::Files);
    while (d.hasNext()) {
        d.next();
        keys.insert(d.fileName());
    }
    return keys;
}
