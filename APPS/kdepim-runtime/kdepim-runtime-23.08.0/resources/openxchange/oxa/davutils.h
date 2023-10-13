/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QString>

namespace OXA
{
/**
 * Namespace that contains methods for creating or modifying DAV XML documents.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
namespace DAVUtils
{
/**
 * Adds a new element with the given @p tag inside the DAV namespace under @p parentNode
 * to the @p document.
 *
 * @return The newly added element.
 */
QDomElement addDavElement(QDomDocument &document, QDomNode &parentNode, const QString &tag);

/**
 * Adds a new element with the given @p tag and @p value inside the OX namespace under @p parentNode
 * to the @p document.
 *
 * @return The newly added element.
 */
QDomElement addOxElement(QDomDocument &document, QDomNode &parentNode, const QString &tag, const QString &text = QString());

/**
 * Sets the attribute of @p element inside the OX namespace with the given @p name to @p value.
 */
void setOxAttribute(QDomElement &element, const QString &name, const QString &value);

/**
 * Checks whether the response @p document contains an error message.
 * If so, @c true is returned, @p errorText set to the error message and @p errorStatus set to error status.
 */
bool davErrorOccurred(const QDomDocument &document, QString &errorText, QString &errorStatus);
}
}
