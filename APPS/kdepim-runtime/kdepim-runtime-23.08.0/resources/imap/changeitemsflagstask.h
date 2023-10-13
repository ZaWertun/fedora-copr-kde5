/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

namespace KIMAP
{
class StoreJob;
}

class ChangeItemsFlagsTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit ChangeItemsFlagsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~ChangeItemsFlagsTask() override;

protected Q_SLOTS:
    void onSelectDone(KJob *job);
    void onAppendFlagsDone(KJob *job);
    void onRemoveFlagsDone(KJob *job);

protected:
    KIMAP::StoreJob *prepareJob(KIMAP::Session *session);

    void doStart(KIMAP::Session *session) override;

    virtual void triggerAppendFlagsJob(KIMAP::Session *session);
    virtual void triggerRemoveFlagsJob(KIMAP::Session *session);

protected:
    int m_processedItems = 0;
};
