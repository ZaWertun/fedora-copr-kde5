/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIMAP/FetchJob>

#include "resourcetask.h"

class RetrieveItemTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit RetrieveItemTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~RetrieveItemTask() override;

private Q_SLOTS:
    void onSelectDone(KJob *job);
    void onMessagesReceived(const QMap<qint64, KIMAP::Message> &messages);
    void onContentFetchDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void triggerFetchJob();

    KIMAP::Session *m_session = nullptr;
    qint64 m_uid = 0;
    bool m_messageReceived = false;
};
