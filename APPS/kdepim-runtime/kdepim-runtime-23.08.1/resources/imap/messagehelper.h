/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <KMime/Message>
#include <kimap/FetchJob>

class MessageHelper
{
public:
    using Ptr = QSharedPointer<MessageHelper>;

    virtual ~MessageHelper();
    virtual Akonadi::Item createItemFromMessage(const KMime::Message::Ptr &message,
                                                const qint64 uid,
                                                const qint64 size,
                                                const QMap<QByteArray, QVariant> &attrs,
                                                const QList<QByteArray> &flags,
                                                const KIMAP::FetchJob::FetchScope &scope,
                                                bool &ok) const;
};
