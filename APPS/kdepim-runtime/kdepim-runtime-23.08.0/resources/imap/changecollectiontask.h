/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class ChangeCollectionTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit ChangeCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~ChangeCollectionTask() override;

    void syncEnabledState(bool);

private Q_SLOTS:
    void onRenameDone(KJob *job);
    void onSubscribeDone(KJob *job);
    void onSetAclDone(KJob *job);
    void onSetMetaDataDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void endTaskIfNeeded();

    int m_pendingJobs = 0;
    Akonadi::Collection m_collection;
    bool m_syncEnabledState = true;
};
