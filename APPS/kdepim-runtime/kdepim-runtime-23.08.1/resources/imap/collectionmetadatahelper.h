/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <Akonadi/Collection>
#include <KIMAP/Acl>

class CollectionMetadataHelper
{
public:
    static Akonadi::Collection::Rights convertRights(const KIMAP::Acl::Rights imapRights, KIMAP::Acl::Rights parentRights);
    static bool applyRights(Akonadi::Collection &collection, const KIMAP::Acl::Rights imapRights, KIMAP::Acl::Rights parentRights);
};
