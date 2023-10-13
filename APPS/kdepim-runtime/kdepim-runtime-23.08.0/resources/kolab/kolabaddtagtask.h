/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kolabrelationresourcetask.h"

class KolabAddTagTask : public KolabRelationResourceTask
{
    Q_OBJECT
public:
    explicit KolabAddTagTask(const ResourceStateInterface::Ptr &resource, QObject *parent = nullptr);

protected:
    void startRelationTask(KIMAP::Session *session) override;

private:
    QByteArray mMessageId;

private:
    void applyFoundUid(qint64 uid);
    void triggerSearchJob(KIMAP::Session *session);

private Q_SLOTS:
    void onAppendMessageDone(KJob *job);
    void onPreSearchSelectDone(KJob *job);
    void onSearchDone(KJob *job);
};
