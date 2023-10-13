/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "davutils.h"

using namespace OXA;

QDomElement DAVUtils::addDavElement(QDomDocument &document, QDomNode &parentNode, const QString &tag)
{
    const QDomElement element = document.createElementNS(QStringLiteral("DAV:"), QLatin1String("D:") + tag);
    parentNode.appendChild(element);

    return element;
}

QDomElement DAVUtils::addOxElement(QDomDocument &document, QDomNode &parentNode, const QString &tag, const QString &text)
{
    QDomElement element = document.createElementNS(QStringLiteral("http://www.open-xchange.org"), QLatin1String("ox:") + tag);

    if (!text.isEmpty()) {
        const QDomText textNode = document.createTextNode(text);
        element.appendChild(textNode);
    }

    parentNode.appendChild(element);

    return element;
}

void DAVUtils::setOxAttribute(QDomElement &element, const QString &name, const QString &value)
{
    element.setAttributeNS(QStringLiteral("http://www.open-xchange.org"), QStringLiteral("ox:") + name, value);
}

bool DAVUtils::davErrorOccurred(const QDomDocument &document, QString &errorText, QString &errorStatus)
{
    const QDomElement documentElement = document.documentElement();
    const QDomNodeList propStats = documentElement.elementsByTagNameNS(QStringLiteral("DAV:"), QStringLiteral("propstat"));

    for (int i = 0; i < propStats.count(); ++i) {
        const QDomElement propStat = propStats.at(i).toElement();
        const QDomElement status = propStat.firstChildElement(QStringLiteral("status"));
        const QDomElement description = propStat.firstChildElement(QStringLiteral("responsedescription"));

        if (status.text() != QLatin1String("200")) {
            errorText = description.text();
            errorStatus = status.text();
            return true;
        }
    }

    return false;
}
