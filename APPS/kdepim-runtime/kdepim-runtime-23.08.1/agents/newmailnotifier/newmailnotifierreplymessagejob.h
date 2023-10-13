/*
   SPDX-FileCopyrightText: 2021-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <KJob>

class NewMailNotifierReplyMessageJob : public KJob
{
    Q_OBJECT
public:
    explicit NewMailNotifierReplyMessageJob(Akonadi::Item::Id id, QObject *parent = nullptr);
    ~NewMailNotifierReplyMessageJob() override;

    void start() override;

    Q_REQUIRED_RESULT bool replyToAll() const;
    void setReplyToAll(bool newReplyToAll);

private:
    const Akonadi::Item::Id mId;
    bool mReplyToAll = false;
};
