/*
    SPDX-FileCopyrightText: 2015-2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsjob.h"
#include <Akonadi/Item>

class EwsClient;

class EwsModifyItemJob : public EwsJob
{
    Q_OBJECT
public:
    EwsModifyItemJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent);
    ~EwsModifyItemJob() override;

    void setModifiedFlags(const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags);

    const Akonadi::Item::List &items() const;
Q_SIGNALS:
    void reportStatus(int status, const QString &message = QString());
    void reportPercent(int progress);

protected:
    Akonadi::Item::List mItems;
    const QSet<QByteArray> mParts;
    EwsClient &mClient;
    QSet<QByteArray> mAddedFlags;
    QSet<QByteArray> mRemovedFlags;
};
