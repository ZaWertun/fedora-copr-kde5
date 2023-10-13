/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009, 2010 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job.h"

#include <Akonadi/Item>

#include <memory>

namespace Akonadi
{
namespace FileStore
{
class AbstractJobSession;
class ItemModifyJobPrivate;

/**
 */
class AKONADI_FILESTORE_EXPORT ItemModifyJob : public Job
{
    friend class AbstractJobSession;

    Q_OBJECT

public:
    explicit ItemModifyJob(const Item &item, AbstractJobSession *session = nullptr);

    ~ItemModifyJob() override;

    void setIgnorePayload(bool ignorePayload);

    Q_REQUIRED_RESULT bool ignorePayload() const;

    Q_REQUIRED_RESULT Item item() const;

    const QSet<QByteArray> &parts() const;
    void setParts(const QSet<QByteArray> &parts);

    bool accept(Visitor *visitor) override;

private:
    void handleItemModified(const Akonadi::Item &item);

private:
    std::unique_ptr<ItemModifyJobPrivate> const d;
};
}
}
