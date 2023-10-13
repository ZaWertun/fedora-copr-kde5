/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "users.h"

#include <QDataStream>
#include <QFile>
#include <QStandardPaths>

using namespace OXA;

Users *Users::mSelf = nullptr;

Users::Users()
    : mCurrentUserId(-1)
{
}

Users::~Users() = default;

Users *Users::self()
{
    if (!mSelf) {
        mSelf = new Users();
    }

    return mSelf;
}

void Users::init(const QString &identifier)
{
    mIdentifier = identifier;

    loadFromCache();
}

qlonglong Users::currentUserId() const
{
    return mCurrentUserId;
}

User Users::lookupUid(qlonglong uid) const
{
    return mUsers.value(uid);
}

User Users::lookupEmail(const QString &email) const
{
    QMapIterator<qlonglong, User> it(mUsers);
    while (it.hasNext()) {
        it.next();

        if (it.value().email() == email) {
            return it.value();
        }
    }

    return {};
}

QString Users::cacheFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("openxchangeresource_") + mIdentifier;
}

void Users::setCurrentUserId(qlonglong id)
{
    mCurrentUserId = id;

    saveToCache();
}

void Users::setUsers(const User::List &users)
{
    mUsers.clear();
    for (const User &user : users) {
        mUsers.insert(user.uid(), user);
    }

    saveToCache();
}

void Users::loadFromCache()
{
    QFile cacheFile(cacheFilePath());
    if (!cacheFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream stream(&cacheFile);
    stream.setVersion(QDataStream::Qt_4_6);

    mUsers.clear();

    stream >> mCurrentUserId;

    qulonglong count;
    stream >> count;

    qlonglong uid;
    QString name;
    QString email;
    for (qulonglong i = 0; i < count; ++i) {
        stream >> uid >> name >> email;

        User user;
        user.setUid(uid);
        user.setName(name);
        user.setEmail(email);
        mUsers.insert(user.uid(), user);
    }
}

void Users::saveToCache()
{
    QFile cacheFile(cacheFilePath());
    if (!cacheFile.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream stream(&cacheFile);
    stream.setVersion(QDataStream::Qt_4_6);

    // write current user id
    stream << mCurrentUserId;

    // write number of users
    stream << (qulonglong)mUsers.count();

    // write uid, name and email address for each user
    QMapIterator<qlonglong, User> it(mUsers);
    while (it.hasNext()) {
        it.next();

        stream << it.value().uid() << it.value().name() << it.value().email();
    }
}
