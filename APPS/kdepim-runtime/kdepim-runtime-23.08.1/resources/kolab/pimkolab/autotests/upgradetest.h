/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <QObject>

/**
 * Test format upgrade from v2 to v3
 *
 * - Parse using v2 implementation
 * - Serialize and reparse using v3 implementation
 * - Compare results
 *
 * This ensures that all properties which are interpreted by the v2 implementation can be serialized and deserialized by the v3 implementation.
 *
 * Depends on serialization used for comparison to be correct (i.e. the ical implementation).
 */

class UpgradeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testIncidence_data();
    void testIncidence();

    void testContact_data();
    void testContact();
};
