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
class UsersRequestJob : public KJob
{
    Q_OBJECT

public:
    explicit UsersRequestJob(QObject *parent = nullptr);

    void start() override;

    Q_REQUIRED_RESULT User::List users() const;

private:
    void davJobFinished(KJob *);
    User::List mUsers;
};
}
