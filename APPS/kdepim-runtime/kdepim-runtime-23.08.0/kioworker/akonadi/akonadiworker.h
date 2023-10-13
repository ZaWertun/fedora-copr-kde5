/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIO/WorkerBase>

namespace Akonadi
{
class Item;
class Collection;
}

class AkonadiWorker : public KIO::WorkerBase
{
public:
    explicit AkonadiWorker(const QByteArray &pool_socket, const QByteArray &app_socket);
    ~AkonadiWorker() override;

    /**
     * Reimplemented from WorkerBase
     */
    KIO::WorkerResult get(const QUrl &url) override;

    /**
     * Reimplemented from WorkerBase
     */
    KIO::WorkerResult stat(const QUrl &url) override;

    /**
     * Reimplemented from WorkerBase
     */
    KIO::WorkerResult listDir(const QUrl &url) override;

    /**
     * Reimplemented from WorkerBase
     */
    KIO::WorkerResult del(const QUrl &url, bool isFile) override;

private:
    static KIO::UDSEntry entryForItem(const Akonadi::Item &item);
    static KIO::UDSEntry entryForCollection(const Akonadi::Collection &collection);
};
