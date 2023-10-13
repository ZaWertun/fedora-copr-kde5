/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include "ewsclient.h"
#include "ewsjob.h"
#include "ewstypes.h"

class EwsModifyItemFlagsJob : public EwsJob
{
    Q_OBJECT
public:
    EwsModifyItemFlagsJob(EwsClient &client,
                          QObject *parent,
                          const Akonadi::Item::List &,
                          const QSet<QByteArray> &addedFlags,
                          const QSet<QByteArray> &removedFlags);
    ~EwsModifyItemFlagsJob() override;

    Akonadi::Item::List items() const
    {
        return mResultItems;
    }

    void start() override;

Q_SIGNALS:
    void reportStatus(int status, const QString &message = QString());
    void reportPercent(int progress);

protected:
    const Akonadi::Item::List mItems;
    Akonadi::Item::List mResultItems;
    EwsClient &mClient;
    QSet<QByteArray> mAddedFlags;
    const QSet<QByteArray> mRemovedFlags;
private Q_SLOTS:
    void itemModifyFinished(KJob *job);
};
