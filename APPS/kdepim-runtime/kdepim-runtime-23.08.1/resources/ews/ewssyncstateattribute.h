/*
    SPDX-FileCopyrightText: 2020 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Attribute>

/**
 *  @brief  Custom attribute to keep the EWS sync state with the collection.
 *
 *  The EWS resource uses the SyncFolderItems operation to synchronize
 *  items of a collection with the server. This operation works in an
 *  incremental way and lets the client keep a state, so that further
 *  calls to the SyncFolderItems are able to retrieve changes relative
 *  to that state.
 */
class EwsSyncStateAttribute : public Akonadi::Attribute
{
public:
    EwsSyncStateAttribute() = default;
    explicit EwsSyncStateAttribute(const QString &syncState);
    void setSyncState(const QString &syncState);
    const QString &syncState() const;

    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    QString mSyncState;
};
