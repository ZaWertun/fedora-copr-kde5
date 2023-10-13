/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <memory>

namespace Akonadi
{
class Collection;
class Item;

using ItemList = QVector<Item>;
}

class CompactChangeHelperPrivate;

class CompactChangeHelper : public QObject
{
    Q_OBJECT

public:
    explicit CompactChangeHelper(const QByteArray &sessionId, QObject *parent = nullptr);

    ~CompactChangeHelper() override;

    void addChangedItems(const Akonadi::ItemList &items);

    QString currentRemoteId(const Akonadi::Item &item) const;

    void checkCollectionChanged(const Akonadi::Collection &collection);

private:
    std::unique_ptr<CompactChangeHelperPrivate> const d;

    Q_PRIVATE_SLOT(d, void processNextBatch())
    Q_PRIVATE_SLOT(d, void processNextItem())
};
