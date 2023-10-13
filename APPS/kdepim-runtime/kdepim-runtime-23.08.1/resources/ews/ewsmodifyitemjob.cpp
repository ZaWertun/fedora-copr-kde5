/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifyitemjob.h"

EwsModifyItemJob::EwsModifyItemJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsJob(parent)
    , mItems(items)
    , mParts(parts)
    , mClient(client)
{
}

EwsModifyItemJob::~EwsModifyItemJob() = default;

void EwsModifyItemJob::setModifiedFlags(const QSet<QByteArray> &addedFlags, const QSet<QByteArray> &removedFlags)
{
    mAddedFlags = addedFlags;
    mRemovedFlags = removedFlags;
}

const Akonadi::Item::List &EwsModifyItemJob::items() const
{
    return mItems;
}
