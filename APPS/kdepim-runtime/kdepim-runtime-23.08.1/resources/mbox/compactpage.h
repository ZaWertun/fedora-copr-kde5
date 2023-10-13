/*
    SPDX-FileCopyrightText: 2009 Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include "ui_compactpage.h"

class KJob;

class CompactPage : public QWidget
{
    Q_OBJECT

public:
    explicit CompactPage(const QString &collectionId, QWidget *parent = nullptr);

private:
    void compact();
    void onCollectionFetchCheck(KJob *);
    void onCollectionFetchCompact(KJob *);
    void onCollectionModify(KJob *);

private: // Methods
    void checkCollectionId();

private: // Members
    const QString mCollectionId;
    Ui::CompactPage ui;
};
