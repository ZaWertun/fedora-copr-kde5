/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class MoveCollectionTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit MoveCollectionTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~MoveCollectionTask() override;

private Q_SLOTS:
    void onExamineDone(KJob *job);
    void onRenameDone(KJob *job);
    void onSubscribeDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void doRename(KIMAP::Session *session);
    QString mailBoxForCollections(const Akonadi::Collection &parent, const Akonadi::Collection &child) const;

    Akonadi::Collection m_collection;
};
