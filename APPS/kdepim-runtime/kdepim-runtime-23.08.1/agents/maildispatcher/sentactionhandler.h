/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/SentActionAttribute>

#include <QObject>

class KJob;

class SentActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit SentActionHandler(QObject *parent = nullptr);

    void runAction(const Akonadi::SentActionAttribute::Action &action);

private:
    void itemFetchResult(KJob *job);
};
