/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "tomboyjobbase.h"
#include <Akonadi/Item>

class TomboyItemsDownloadJob : public TomboyJobBase
{
    Q_OBJECT
public:
    // ctor
    explicit TomboyItemsDownloadJob(Akonadi::Collection::Id id, QNetworkAccessManager *manager, QObject *parent = nullptr);
    // returns the parsed results wrapped in Akonadi::Item::List, see below
    Akonadi::Item::List items() const;

    // automatically called by KJob
    void start() override;

private:
    const Akonadi::Collection::Id mCollectionId;
    // This will be called once the request is finished.
    void onRequestFinished();
    Akonadi::Item::List mResultItems;
};
