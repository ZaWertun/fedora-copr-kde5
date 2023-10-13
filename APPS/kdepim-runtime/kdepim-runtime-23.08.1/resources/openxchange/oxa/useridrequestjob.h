/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

namespace OXA
{
class UserIdRequestJob : public KJob
{
    Q_OBJECT

public:
    explicit UserIdRequestJob(QObject *parent = nullptr);

    void start() override;

    Q_REQUIRED_RESULT qlonglong userId() const;

private:
    void davJobFinished(KJob *);
    qlonglong mUserId = -1;
};
}
