/*
    SPDX-FileCopyrightText: 2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewssyncstateattribute.h"

EwsSyncStateAttribute::EwsSyncStateAttribute(const QString &syncState)
    : mSyncState(syncState)
{
}

void EwsSyncStateAttribute::setSyncState(const QString &syncState)
{
    mSyncState = syncState;
}

const QString &EwsSyncStateAttribute::syncState() const
{
    return mSyncState;
}

QByteArray EwsSyncStateAttribute::type() const
{
    static const QByteArray attrType("ewssyncstate");
    return attrType;
}

Akonadi::Attribute *EwsSyncStateAttribute::clone() const
{
    return new EwsSyncStateAttribute(mSyncState);
}

QByteArray EwsSyncStateAttribute::serialized() const
{
    return mSyncState.toUtf8();
}

void EwsSyncStateAttribute::deserialize(const QByteArray &data)
{
    mSyncState = QString::fromUtf8(data);
}
