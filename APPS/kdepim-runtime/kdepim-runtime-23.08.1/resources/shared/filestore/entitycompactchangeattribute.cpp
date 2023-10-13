/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "entitycompactchangeattribute.h"

#include <QDataStream>
#include <QIODevice>

using namespace Akonadi;

class FileStore::EntityCompactChangeAttributePrivate
{
public:
    EntityCompactChangeAttributePrivate &operator=(const EntityCompactChangeAttributePrivate &other)
    {
        if (&other == this) {
            return *this;
        }

        mRemoteId = other.mRemoteId;
        mRemoteRev = other.mRemoteRev;
        return *this;
    }

public:
    QString mRemoteId;
    QString mRemoteRev;
};

FileStore::EntityCompactChangeAttribute::EntityCompactChangeAttribute()
    : Attribute()
    , d(new EntityCompactChangeAttributePrivate())
{
}

FileStore::EntityCompactChangeAttribute::~EntityCompactChangeAttribute() = default;

void FileStore::EntityCompactChangeAttribute::setRemoteId(const QString &remoteId)
{
    d->mRemoteId = remoteId;
}

QString FileStore::EntityCompactChangeAttribute::remoteId() const
{
    return d->mRemoteId;
}

void FileStore::EntityCompactChangeAttribute::setRemoteRevision(const QString &remoteRev)
{
    d->mRemoteRev = remoteRev;
}

QString FileStore::EntityCompactChangeAttribute::remoteRevision() const
{
    return d->mRemoteRev;
}

QByteArray FileStore::EntityCompactChangeAttribute::type() const
{
    static const QByteArray sType("ENTITYCOMPACTCHANGE");
    return sType;
}

FileStore::EntityCompactChangeAttribute *FileStore::EntityCompactChangeAttribute::clone() const
{
    auto copy = new FileStore::EntityCompactChangeAttribute();
    *(copy->d) = *d;
    return copy;
}

QByteArray FileStore::EntityCompactChangeAttribute::serialized() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << d->mRemoteId;
    stream << d->mRemoteRev;

    return data;
}

void FileStore::EntityCompactChangeAttribute::deserialize(const QByteArray &data)
{
    QDataStream stream(data);
    stream >> d->mRemoteId;
    stream >> d->mRemoteRev;
}
