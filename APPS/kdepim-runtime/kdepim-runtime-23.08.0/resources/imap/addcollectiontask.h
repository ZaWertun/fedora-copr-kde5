/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

namespace KIMAP
{
class Session;
}

class AddCollectionTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit AddCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~AddCollectionTask() override;

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void onCreateDone(KJob *job);
    void onSubscribeDone(KJob *job);
    void onSetMetaDataDone(KJob *job);
    Akonadi::Collection m_collection;
    uint m_pendingJobs = 0;
    KIMAP::Session *m_session = nullptr;
};
