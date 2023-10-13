/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-filestore_export.h"

#include <Akonadi/Attribute>

#include <memory>

namespace Akonadi
{
namespace FileStore
{
class EntityCompactChangeAttributePrivate;

class AKONADI_FILESTORE_EXPORT EntityCompactChangeAttribute : public Attribute
{
public:
    EntityCompactChangeAttribute();

    ~EntityCompactChangeAttribute() override;

    void setRemoteId(const QString &remoteId);

    Q_REQUIRED_RESULT QString remoteId() const;

    void setRemoteRevision(const QString &remoteRev);

    Q_REQUIRED_RESULT QString remoteRevision() const;

    Q_REQUIRED_RESULT QByteArray type() const override;

    EntityCompactChangeAttribute *clone() const override;

    Q_REQUIRED_RESULT QByteArray serialized() const override;

    void deserialize(const QByteArray &data) override;

private:
    //@cond PRIVATE
    std::unique_ptr<EntityCompactChangeAttributePrivate> const d;
    //@endcond
};
}
}
