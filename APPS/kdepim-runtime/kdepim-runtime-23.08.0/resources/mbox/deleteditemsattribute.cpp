/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deleteditemsattribute.h"

DeletedItemsAttribute::DeletedItemsAttribute() = default;

DeletedItemsAttribute::DeletedItemsAttribute(const DeletedItemsAttribute &other)
    : Akonadi::Attribute()
{
    if (&other == this) {
        return;
    }

    mDeletedItemOffsets = other.mDeletedItemOffsets;
}

DeletedItemsAttribute::~DeletedItemsAttribute() = default;

void DeletedItemsAttribute::addDeletedItemOffset(quint64 offset)
{
    mDeletedItemOffsets.insert(offset);
}

DeletedItemsAttribute *DeletedItemsAttribute::clone() const
{
    return new DeletedItemsAttribute(*this);
}

QSet<quint64> DeletedItemsAttribute::deletedItemOffsets() const
{
    return mDeletedItemOffsets;
}

KMBox::MBoxEntry::List DeletedItemsAttribute::deletedItemEntries() const
{
    KMBox::MBoxEntry::List entries;
    entries.reserve(mDeletedItemOffsets.count());
    for (quint64 offset : std::as_const(mDeletedItemOffsets)) {
        entries << KMBox::MBoxEntry(offset);
    }

    return entries;
}

void DeletedItemsAttribute::deserialize(const QByteArray &data)
{
    const QList<QByteArray> offsets = data.split(',');
    mDeletedItemOffsets.clear();
    mDeletedItemOffsets.reserve(offsets.count());
    for (const QByteArray &offset : offsets) {
        mDeletedItemOffsets.insert(offset.toULongLong());
    }
}

QByteArray DeletedItemsAttribute::serialized() const
{
    QByteArray serialized;

    for (quint64 offset : std::as_const(mDeletedItemOffsets)) {
        serialized += QByteArray::number(offset) + ',';
    }

    serialized.chop(1); // Remove the last ','

    return serialized;
}

int DeletedItemsAttribute::offsetCount() const
{
    return mDeletedItemOffsets.size();
}

QByteArray DeletedItemsAttribute::type() const
{
    static const QByteArray sType("DeletedMboxItems");
    return sType;
}

bool DeletedItemsAttribute::operator==(const DeletedItemsAttribute &other) const
{
    return mDeletedItemOffsets == other.deletedItemOffsets();
}
