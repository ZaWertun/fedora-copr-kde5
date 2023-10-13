/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class AddItemTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit AddItemTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~AddItemTask() override;

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void onAppendMessageDone(KJob *job);
    void onPreSearchSelectDone(KJob *job);
    void onSearchDone(KJob *job);
    void triggerSearchJob(KIMAP::Session *session);
    void applyFoundUid(qint64 uid);

    QByteArray m_messageId;
};
