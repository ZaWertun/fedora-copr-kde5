/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

#include "user.h"

namespace OXA
{
class UpdateUsersJob : public KJob
{
    Q_OBJECT

public:
    explicit UpdateUsersJob(QObject *parent = nullptr);

    void start() override;

private:
    void userIdRequestJobFinished(KJob *);
    void usersRequestJobFinished(KJob *);
    void finish();

    bool mUserIdRequestFinished = false;
    bool mUsersRequestFinished = false;
    User::List mUsers;
    qlonglong mUserId = -1;
};
}
