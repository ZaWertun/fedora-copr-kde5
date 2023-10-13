/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "tomboyjobbase.h"
#include <Akonadi/Collection>

class TomboyCollectionsDownloadJob : public TomboyJobBase
{
    Q_OBJECT
public:
    // ctor
    explicit TomboyCollectionsDownloadJob(const QString &collectionName, QNetworkAccessManager *manager, int refreshInterval, QObject *parent = nullptr);
    // returns the parsed results wrapped in Akonadi::Collection::List, see below
    Akonadi::Collection::List collections() const;

    // automatically called by KJob
    void start() override;

private:
    // This will be called once the request is finished.
    void onRequestFinished();
    Akonadi::Collection::List mResultCollections;
    const QString mCollectionName;
    const int mRefreshInterval;
};
