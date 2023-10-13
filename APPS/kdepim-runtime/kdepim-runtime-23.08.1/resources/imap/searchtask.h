/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "resourcetask.h"

class SearchTask : public ResourceTask
{
    Q_OBJECT
public:
    SearchTask(const ResourceStateInterface::Ptr &state, const QString &query, QObject *parent);
    ~SearchTask() override;

protected:
    void doStart(KIMAP::Session *session) override;

private Q_SLOTS:
    void onSelectDone(KJob *job);
    void onSearchDone(KJob *job);

private:
    void doSearch(KIMAP::Session *session);

    const QString m_query;
};
