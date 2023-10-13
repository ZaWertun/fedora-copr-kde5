/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsfoldershape.h"

class EwsItemShape : public EwsFolderShape
{
public:
    enum Flag {
        IncludeMimeContent = 0x01,
        FilterHtmlContent = 0x02,
        ConvertHtmlToUtf8 = 0x04,
    };
    enum BodyType {
        BodyNone,
        BodyBest,
        BodyHtml,
        BodyText,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    explicit EwsItemShape(EwsBaseShape shape = EwsShapeDefault)
        : EwsFolderShape(shape)
        , mBodyType(BodyNone)
    {
    }

    EwsItemShape(const EwsItemShape &other)
        : EwsFolderShape(other)
        , mBodyType(BodyNone)
    {
    }

    explicit EwsItemShape(EwsFolderShape &&other)
        : EwsFolderShape(other)
        , mBodyType(BodyNone)
    {
    }

    EwsItemShape &operator=(EwsItemShape &&other)
    {
        mBaseShape = other.mBaseShape;
        mProps = std::move(other.mProps);
        mFlags = other.mFlags;
        mBodyType = other.mBodyType;
        return *this;
    }

    EwsItemShape &operator=(const EwsItemShape &other)
    {
        mBaseShape = other.mBaseShape;
        mProps = other.mProps;
        mFlags = other.mFlags;
        mBodyType = other.mBodyType;
        return *this;
    }

    void setFlags(Flags flags)
    {
        mFlags = flags;
    }

    void setBodyType(BodyType type)
    {
        mBodyType = type;
    }

    void write(QXmlStreamWriter &writer) const;

protected:
    Flags mFlags;
    BodyType mBodyType;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(EwsItemShape::Flags)
