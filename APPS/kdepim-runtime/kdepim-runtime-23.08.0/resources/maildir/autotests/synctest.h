/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <Akonadi/Collection>

/**
  This syncs the resource again and again, watching out for "lost"
  synchronized() signals.
 */
class SyncTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testSync();

private:
    QString mMaildirIdentifier;
};
