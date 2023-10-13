/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "user.h"

#include <QMap>
#include <QObject>

namespace OXA
{
class Users : public QObject
{
    Q_OBJECT

public:
    ~Users() override;

    static Users *self();

    void init(const QString &identifier);

    Q_REQUIRED_RESULT qlonglong currentUserId() const;

    User lookupUid(qlonglong uid) const;
    User lookupEmail(const QString &email) const;

    QString cacheFilePath() const;

private:
    friend class UpdateUsersJob;

    Users();
    void setCurrentUserId(qlonglong);
    void setUsers(const User::List &);

    void loadFromCache();
    void saveToCache();

    qlonglong mCurrentUserId = -1;
    QMap<qlonglong, User> mUsers;
    QString mIdentifier;

    static Users *mSelf;
};
}
