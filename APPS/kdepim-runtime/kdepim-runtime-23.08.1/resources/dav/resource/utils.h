/*
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDAV/Enums>

#include <Akonadi/Item>

namespace KDAV
{
class DavItem;
}

namespace Akonadi
{
class Collection;
class Item;
}

/**
 * @short A namespace that contains helper methods for DAV functionality.
 */
namespace Utils
{
/**
 * Returns the i18n'ed name of the given DAV @p protocol dialect.
 */
QString translatedProtocolName(KDAV::Protocol protocol);

/**
 * Returns the protocol matching the given i18n'ed @p name. This is the opposite
 * of Utils::translatedProtocolName().
 */
KDAV::Protocol protocolByTranslatedName(const QString &name);

/**
 * Creates a new KDAV::DavItem from the Akonadi::Item @p item.
 *
 * The returned item will have no payload (DavItem::data() will return an empty
 * QByteArray) if the @p item payload is not recognized.
 */
KDAV::DavItem
createDavItem(const Akonadi::Item &item, const Akonadi::Collection &collection, const Akonadi::Item::List &dependentItems = Akonadi::Item::List());

/**
 * Parses the DAV data contained in @p source and puts it in @p target and @extraItems.
 */
bool parseDavData(const KDAV::DavItem &source, Akonadi::Item &target, Akonadi::Item::List &extraItems);
}
