/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "user.h"

using namespace OXA;

User::User() = default;

bool User::isValid() const
{
    return mUid != -1;
}

void User::setUid(qlonglong uid)
{
    mUid = uid;
}

qlonglong User::uid() const
{
    return mUid;
}

void User::setEmail(const QString &email)
{
    mEmail = email;
}

QString User::email() const
{
    return mEmail;
}

void User::setName(const QString &name)
{
    mName = name;
}

QString User::name() const
{
    return mName;
}
