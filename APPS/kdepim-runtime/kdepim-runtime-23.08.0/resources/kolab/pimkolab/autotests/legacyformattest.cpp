/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "legacyformattest.h"
#include "kolabformat/errorhandler.h"
#include "kolabformat/xmlobject.h"
#include "pimkolab_debug.h"
#include "testutils.h"
#include <QTest>
#include <fstream>
#include <sstream>

void V2Test::testReadDistlistUID()
{
    std::ifstream t((TESTFILEDIR.toStdString() + "v2/contacts/distlistWithUID.xml").c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();

    Kolab::XMLObject xo;
    const Kolab::DistList distlist = xo.readDistlist(buffer.str(), Kolab::KolabV2);
    const auto members = distlist.members();
    for (const Kolab::ContactReference &contact : members) {
        QVERIFY(!contact.uid().empty());
    }
    QVERIFY(!Kolab::ErrorHandler::errorOccured());
}

void V2Test::testWriteDistlistUID()
{
    Kolab::DistList distlist;
    distlist.setUid("uid");
    distlist.setName("name");
    std::vector<Kolab::ContactReference> members;
    members.emplace_back(Kolab::ContactReference::UidReference, "memberuid", "membername");
    distlist.setMembers(members);

    Kolab::XMLObject xo;
    const std::string xml = xo.writeDistlist(distlist, Kolab::KolabV2);
    QVERIFY(QString::fromStdString(xml).contains(QLatin1String("memberuid")));
    QVERIFY(!Kolab::ErrorHandler::errorOccured());
}

QTEST_MAIN(V2Test)
