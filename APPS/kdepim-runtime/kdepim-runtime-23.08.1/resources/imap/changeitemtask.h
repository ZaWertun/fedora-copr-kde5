/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class ChangeItemTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit ChangeItemTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~ChangeItemTask() override;

private Q_SLOTS:
    void onAppendMessageDone(KJob *job);

    void onPreStoreSelectDone(KJob *job);
    void onStoreFlagsDone(KJob *job);

    void onPreDeleteSelectDone(KJob *job);
    void onSearchDone(KJob *job);
    void onDeleteDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void triggerStoreJob();
    void triggerSearchJob();
    void triggerDeleteJob();

    void recordNewUid();

    KIMAP::Session *m_session = nullptr;
    QByteArray m_messageId;
    qint64 m_oldUid = 0;
    qint64 m_newUid = 0;
};
