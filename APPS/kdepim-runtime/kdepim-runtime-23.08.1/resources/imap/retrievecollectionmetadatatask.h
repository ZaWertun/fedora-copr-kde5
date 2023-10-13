/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcetask.h"

class RetrieveCollectionMetadataTask : public ResourceTask
{
    Q_OBJECT

public:
    explicit RetrieveCollectionMetadataTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);
    ~RetrieveCollectionMetadataTask() override;

private Q_SLOTS:
    void onGetMetaDataDone(KJob *job);
    void onGetAclDone(KJob *job);
    void onRightsReceived(KJob *job);
    void onQuotasReceived(KJob *job);

protected:
    void doStart(KIMAP::Session *session) override;

private:
    void endTaskIfNeeded();

    int m_pendingMetaDataJobs = 0;

    Akonadi::Collection m_collection;
    KIMAP::Session *m_session = nullptr;
};
