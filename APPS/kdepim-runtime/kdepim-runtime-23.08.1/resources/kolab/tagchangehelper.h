/*
    SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Krammer <kevin.krammer@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KMime/Message>

#include <QObject>

namespace KIMAP
{
class Session;
}

namespace Akonadi
{
class Tag;
}

class KolabRelationResourceTask;

struct TagConverter {
    KMime::Message::Ptr createMessage(const Akonadi::Tag &tag, const Akonadi::Item::List &items, const QString &username);
};

class TagChangeHelper : public QObject
{
    Q_OBJECT
public:
    explicit TagChangeHelper(KolabRelationResourceTask *parent = nullptr);

    void start(const Akonadi::Tag &tag, const KMime::Message::Ptr &message, KIMAP::Session *session);

Q_SIGNALS:
    void applyCollectionChanges(const Akonadi::Collection &collection);
    void cancelTask(const QString &errorText);
    void changeCommitted();

private:
    KolabRelationResourceTask *const mTask;

private:
    void recordNewUid(qint64 newUid, const Akonadi::Tag &tag);

private Q_SLOTS:
    void onReplaceDone(KJob *job);
    void onModifyDone(KJob *job);
};
