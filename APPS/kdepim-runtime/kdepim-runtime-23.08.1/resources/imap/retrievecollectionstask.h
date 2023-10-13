/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>

#include "resourcetask.h"

class RetrieveCollectionsTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit RetrieveCollectionsTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~RetrieveCollectionsTask() override;

private Q_SLOTS:
    void onMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);
    void onMailBoxesReceiveDone(KJob *job);
    void onFullMailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);
    void onFullMailBoxesReceiveDone(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

protected:
    QHash<QString, Akonadi::Collection> m_reportedCollections;
    QHash<QString, Akonadi::Collection> m_dummyCollections;
    QSet<QString> m_fullReportedCollections;
};
