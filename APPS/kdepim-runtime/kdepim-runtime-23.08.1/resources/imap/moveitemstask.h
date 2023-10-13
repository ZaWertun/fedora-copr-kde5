/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

#include <KIMAP/ImapSet>

class MoveItemsTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit MoveItemsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~MoveItemsTask() override;

private Q_SLOTS:
    void onSelectDone(KJob *job);
    void onCopyDone(KJob *job);
    void onStoreFlagsDone(KJob *job);
    void onMoveDone(KJob *job);

    void onPreSearchSelectDone(KJob *job);
    void onSearchDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void startMove(KIMAP::Session *session);
    void recordNewUid();
    QVector<qint64> imapSetToList(const KIMAP::ImapSet &set);

    KIMAP::ImapSet m_oldSet;
    QVector<qint64> m_newUids;
    QMap<Akonadi::Item::Id /* original ID */, QByteArray> m_messageIds;
};
