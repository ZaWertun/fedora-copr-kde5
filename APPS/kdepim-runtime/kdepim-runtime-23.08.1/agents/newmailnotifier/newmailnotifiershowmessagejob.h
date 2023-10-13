/*
   SPDX-FileCopyrightText: 2014-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <KJob>

class NewMailNotifierShowMessageJob : public KJob
{
    Q_OBJECT
public:
    explicit NewMailNotifierShowMessageJob(Akonadi::Item::Id id, QObject *parent = nullptr);
    ~NewMailNotifierShowMessageJob() override;

    void start() override;

private:
    const Akonadi::Item::Id mId;
};
