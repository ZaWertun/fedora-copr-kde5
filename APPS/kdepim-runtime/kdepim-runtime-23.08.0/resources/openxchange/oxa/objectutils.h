/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "folder.h"
#include "object.h"

class KJob;

class QDomDocument;
class QDomElement;

namespace OXA
{
namespace ObjectUtils
{
Object parseObject(const QDomElement &propElement, Folder::Module module);
void addObjectElements(QDomDocument &document, QDomElement &propElement, const Object &object, void *preloadedData = nullptr);

/**
 * Returns the dav path that is used for the given @p module.
 */
QString davPath(Folder::Module module);

/**
 * On some actions (e.g. creating or modifying items) we have to preload
 * data asynchronously. The following methods allow to do that in a generic way.
 */

/**
 * Checks whether the @p object needs preloading of data.
 */
bool needsPreloading(const Object &object);

/**
 * Creates a preloading job for the @p object.
 */
KJob *preloadJob(const Object &object);

/**
 * Converts the data loaded by the preloading @p job into pointer
 * that will be passed to addObjectElements later on.
 */
void *preloadData(const Object &object, KJob *job);
}
}
