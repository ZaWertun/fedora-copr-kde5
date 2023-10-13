/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "tomboyjobbase.h"
#include <Akonadi/Item>
#include <KMime/Message>

enum class JobType {
    AddItem,
    ModifyItem,
    DeleteItem,
};

class TomboyItemUploadJob : public TomboyJobBase
{
    Q_OBJECT
public:
    TomboyItemUploadJob(const Akonadi::Item &item, JobType jobType, QNetworkAccessManager *manager, QObject *parent = nullptr);

    // Returns mSourceItem for post-processing purposes
    Akonadi::Item item() const;

    JobType jobType() const;

    // automatically called by KJob
    void start() override;

private:
    // This will be called once the request is finished.
    void onRequestFinished();
    // Workaround for https://bugreports.qt-project.org/browse/QTBUG-26161 Qt bug
    QString getCurrentISOTime() const;

    Akonadi::Item mSourceItem;
    KMime::Message::Ptr mNoteContent;

    const JobType mJobType;

    int mRemoteRevision;
};
