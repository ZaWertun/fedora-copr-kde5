/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "object.h"

class QDomDocument;
class QDomElement;

namespace OXA
{
/**
 * Namespace that contains helper methods for handling events and tasks.
 */
namespace IncidenceUtils
{
/**
 * Parses the XML tree under @p propElement and fills the event data of @p object.
 */
void parseEvent(const QDomElement &propElement, Object &object);

/**
 * Parses the XML tree under @p propElement and fills the task data of @p object.
 */
void parseTask(const QDomElement &propElement, Object &object);

/**
 * Adds the event data of @p object to the @p document under the @p propElement.
 */
void addEventElements(QDomDocument &document, QDomElement &propElement, const Object &object);

/**
 * Adds the task data of @p object to the @p document under the @p propElement.
 */
void addTaskElements(QDomDocument &document, QDomElement &propElement, const Object &object);
}
}
