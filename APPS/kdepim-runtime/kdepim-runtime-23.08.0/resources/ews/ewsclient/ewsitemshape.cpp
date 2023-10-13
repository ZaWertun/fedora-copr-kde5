/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitemshape.h"

void EwsItemShape::write(QXmlStreamWriter &writer) const
{
    writer.writeStartElement(ewsMsgNsUri, QStringLiteral("ItemShape"));

    // Write the base shape
    writeBaseShape(writer);

    // Write the IncludeMimeContent element (if applicable)
    if (mFlags.testFlag(IncludeMimeContent)) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("IncludeMimeContent"), QStringLiteral("true"));
    }

    // Write the BodyType element
    if (mBodyType != BodyNone) {
        QString bodyTypeText;

        switch (mBodyType) {
        case BodyHtml:
            bodyTypeText = QStringLiteral("HTML");
            break;
        case BodyText:
            bodyTypeText = QStringLiteral("Text");
            break;
        default:
            // case BodyBest:
            bodyTypeText = QStringLiteral("Best");
            break;
        }
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("BodyType"), bodyTypeText);
    }

    // Write the FilterHtmlContent element (if applicable)
    if (mBodyType == BodyHtml && mFlags.testFlag(FilterHtmlContent)) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("FilterHtmlContent"), QStringLiteral("true"));
    }

    // Write the ConvertHtmlCodePageToUTF8 element (if applicable)
    if (mBodyType == BodyHtml && mFlags.testFlag(ConvertHtmlToUtf8)) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("ConvertHtmlCodePageToUTF8"), QStringLiteral("true"));
    }

    // Write properties (if any)
    writeProperties(writer);

    writer.writeEndElement();
}
