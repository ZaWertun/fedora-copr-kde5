/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QDomDocument;
class QDomElement;

namespace OXA
{
class Folder;

/**
 * Namespace that contains helper methods for handling folders.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
namespace FolderUtils
{
/**
 * Parses the XML tree under @p propElement and return the folder.
 */
Folder parseFolder(const QDomElement &propElement);

/**
 * Adds the @p folder data to the @p document under the @p propElement.
 */
void addFolderElements(QDomDocument &document, QDomElement &propElement, const Folder &folder);
}
}
