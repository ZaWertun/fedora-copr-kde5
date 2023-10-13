/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "session_p.h"

#include <memory>

namespace Akonadi
{
namespace FileStore
{
class FiFoQueueJobSessionPrivate;

/**
 */
class FiFoQueueJobSession : public AbstractJobSession
{
    Q_OBJECT

public:
    explicit FiFoQueueJobSession(QObject *parent = nullptr);

    ~FiFoQueueJobSession() override;

    void addJob(Job *job) override;

    void cancelAllJobs() override;

protected:
    void removeJob(Job *job) override;

private:
    std::unique_ptr<FiFoQueueJobSessionPrivate> const d;
};
}
}
