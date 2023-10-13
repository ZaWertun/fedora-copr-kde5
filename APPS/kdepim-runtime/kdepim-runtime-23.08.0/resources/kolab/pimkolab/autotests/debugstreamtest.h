/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <QObject>

class DebugStreamTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDebugstream();
    void testDebugNotLogged();
    void testHasError();
};
