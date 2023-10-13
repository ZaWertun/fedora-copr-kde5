/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "object.h"

#include <KJob>

namespace OXA
{
class ObjectDeleteJob : public KJob
{
    Q_OBJECT

public:
    explicit ObjectDeleteJob(const Object &object, QObject *parent = nullptr);

    void start() override;

private:
    void davJobFinished(KJob *);
    Object mObject;
};
}
