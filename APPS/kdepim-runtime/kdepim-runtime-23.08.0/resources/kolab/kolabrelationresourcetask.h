/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <resourcetask.h>

class KolabRelationResourceTask : public ResourceTask
{
    Q_OBJECT
public:
    explicit KolabRelationResourceTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);

    Akonadi::Collection relationCollection() const;

    using ResourceTask::mailBoxForCollection;
    using ResourceTask::resourceState;

protected:
    Akonadi::Collection mRelationCollection;

protected:
    void doStart(KIMAP::Session *session) override;

    virtual void startRelationTask(KIMAP::Session *session) = 0;

private:
    KIMAP::Session *mImapSession = nullptr;

private Q_SLOTS:
    void onCollectionFetchResult(KJob *job);
    void onCreateDone(KJob *job);
    void onSetMetaDataDone(KJob *job);
    void onLocalCreateDone(KJob *job);
};
