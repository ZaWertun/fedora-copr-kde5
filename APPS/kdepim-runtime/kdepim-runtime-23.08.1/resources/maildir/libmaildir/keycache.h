/*
    SPDX-FileCopyrightText: 2012 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

/** @brief a cache for the maildir keys (file names in cur/new folders).
 *  It is used to find if a file is in cur or new
 */

#include <QHash>
#include <QSet>

class KeyCache
{
public:
    static KeyCache *self()
    {
        if (!mSelf) {
            mSelf = new KeyCache();
        }
        return mSelf;
    }

    /** Find the new and cur keys on the disk for @param dir and add them to the cache */
    void addKeys(const QString &dir);

    /** Refresh the new and cur keys for @param dir */
    void refreshKeys(const QString &dir);

    /** Add a "new" key for @param dir. */
    void addNewKey(const QString &dir, const QString &key);

    /** Add a "cur" key for @param dir. */
    void addCurKey(const QString &dir, const QString &key);

    /** Remove all keys associated with @param dir. */
    void removeKey(const QString &dir, const QString &key);

    /** Check if the @param key is a "cur" key in @param dir */
    bool isCurKey(const QString &dir, const QString &key) const;

    /** Check if the @param key is a "new" key in @param dir */
    bool isNewKey(const QString &dir, const QString &key) const;

private:
    KeyCache() = default;

    QSet<QString> listNew(const QString &dir) const;

    QSet<QString> listCurrent(const QString &dir) const;

    QHash<QString, QSet<QString>> mNewKeys;
    QHash<QString, QSet<QString>> mCurKeys;

    static KeyCache *mSelf;
};
