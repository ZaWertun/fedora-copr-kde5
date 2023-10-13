/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once
#include <QObject>

/**
 * Test complete serialization from mime message to KCalendarCore/KContacts containers.
 *
 * The .ics .vcf files serve as reference, under the assumption that the iCal/vCard implementations don't have bugs.
 *
 * The .ics.mime/.vcf.mime messages are first used to test parsing, and then to compare the result after serialization.
 * The mime files are of course just a snapshot of how it is thought that the result should be.
 * The files may have bugs themselves and will need to be updated to reflect future changes.
 *
 * kompare (the kde diff tool) is launched if a test fails to show you the difference between expected and actual conversion.
 * Comment the code if you don't want it.
 *
 * Note that some parts of the mime message are not compared, but show up in kompare as difference:
 * * Date-Header
 * * Content-Type-Header
 * * next-part markers
 * * Content-ID Header
 *
 * These are safe to be ignored as they are randomly generated.
 *
 * To generate the reference files, you can write the vcf/ics by hand and generate the mime message from that one (using the generateMimefile() function).
 * You may need to sort some things manually (addresses/phonenumbers), and generally check manually if everything is as it should be.
 * The test will then remain to ensure the same for future versions.
 *
 */
class FormatTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();

    void testIncidence_data();
    void testIncidence();

    void testContact_data();
    void testContact();

    void testDistlist_data();
    void testDistlist();

    void testNote_data();
    void testNote();

    // Some pseudo tests and helper functions
    void generateMimefile();
    void generateVCard();
    void proveJPGisLossy();
};
