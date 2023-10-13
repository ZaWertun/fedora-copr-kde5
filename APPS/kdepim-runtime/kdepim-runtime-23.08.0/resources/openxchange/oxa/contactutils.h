/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "object.h"

class KJob;

class QDomDocument;
class QDomElement;

namespace OXA
{
/**
 * Namespace that contains helper methods for handling contacts.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
namespace ContactUtils
{
/**
 * Parses the XML tree under @p propElement and fills the contact data of @p object.
 */
void parseContact(const QDomElement &propElement, Object &object);

/**
 * Adds the contact data of @p object to the @p document under the @p propElement.
 */
void addContactElements(QDomDocument &document, QDomElement &propElement, const Object &object, void *preloadedData);

KJob *preloadJob(const Object &object);
void *preloadData(const Object &object, KJob *job);
}
}
