/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "xmlobjecttest.h"

#include <QTest>
#include <iostream>

#include "kolabformat/xmlobject.h"

void XMLObjectTest::testEvent()
{
    Kolab::Event event;
    event.setStart(Kolab::cDateTime(2012, 01, 01));

    Kolab::XMLObject xmlobject;
    const std::string output = xmlobject.writeEvent(event, Kolab::KolabV2, "productid");
    QVERIFY(!output.empty());
    std::cout << output;

    const Kolab::Event resultEvent = xmlobject.readEvent(output, Kolab::KolabV2);
    QVERIFY(resultEvent.isValid());
}

void XMLObjectTest::testDontCrash()
{
    Kolab::XMLObject ob;
    ob.writeEvent(Kolab::Event(), Kolab::KolabV2, "");
    ob.writeTodo(Kolab::Todo(), Kolab::KolabV2, "");
    ob.writeJournal(Kolab::Journal(), Kolab::KolabV2, "");
    ob.writeFreebusy(Kolab::Freebusy(), Kolab::KolabV2, "");
    ob.writeContact(Kolab::Contact(), Kolab::KolabV2, "");
    ob.writeDistlist(Kolab::DistList(), Kolab::KolabV2, "");
    ob.writeNote(Kolab::Note(), Kolab::KolabV2, "");
    ob.writeConfiguration(Kolab::Configuration(), Kolab::KolabV2, "");

    ob.readEvent("", Kolab::KolabV2);
    ob.readTodo("", Kolab::KolabV2);
    ob.readJournal("", Kolab::KolabV2);
    ob.readFreebusy("", Kolab::KolabV2);
    ob.readContact("", Kolab::KolabV2);
    ob.readDistlist("", Kolab::KolabV2);
    ob.readNote("", Kolab::KolabV2);
    ob.readConfiguration("", Kolab::KolabV2);
}

QTEST_MAIN(XMLObjectTest)
