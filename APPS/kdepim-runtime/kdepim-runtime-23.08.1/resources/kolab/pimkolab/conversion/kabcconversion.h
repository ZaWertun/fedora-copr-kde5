/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <kolabcontact.h>

namespace Kolab
{
/**
 * Conversion of Kolab-Containers to/from KABC Containers.
 *
 */
namespace Conversion
{
KOLAB_EXPORT KContacts::Addressee toKABC(const Kolab::Contact &);
KOLAB_EXPORT Kolab::Contact fromKABC(const KContacts::Addressee &);

KOLAB_EXPORT KContacts::ContactGroup toKABC(const Kolab::DistList &);
KOLAB_EXPORT Kolab::DistList fromKABC(const KContacts::ContactGroup &);
}
}
