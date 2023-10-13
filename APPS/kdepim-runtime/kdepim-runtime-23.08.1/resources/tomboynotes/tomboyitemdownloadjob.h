/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "tomboyjobbase.h"
#include <Akonadi/Item>

class TomboyItemDownloadJob : public TomboyJobBase
{
    Q_OBJECT
public:
    explicit TomboyItemDownloadJob(const Akonadi::Item &item, QNetworkAccessManager *manager, QObject *parent = nullptr);
    // returns the parsed results wrapped in Akonadi::Item, see below
    Akonadi::Item item() const;

    // automatically called by KJob
    void start() override;

private:
    // This will be called once the request is finished.
    void onRequestFinished();
    Akonadi::Item mResultItem;
};
