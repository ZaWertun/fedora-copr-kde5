/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QVector>
#include <QXmlStreamWriter>

#include "ewspropertyfield.h"
#include "ewstypes.h"

class EwsFolderShape
{
public:
    explicit EwsFolderShape(EwsBaseShape shape = EwsShapeDefault)
        : mBaseShape(shape)
    {
    }

    ~EwsFolderShape()
    {
    }

    EwsFolderShape(const EwsFolderShape &other)
        : mBaseShape(other.mBaseShape)
        , mProps(other.mProps)
    {
    }

    EwsFolderShape(EwsFolderShape &&other)
        : mBaseShape(other.mBaseShape)
        , mProps(other.mProps)
    {
    }

    EwsFolderShape &operator=(EwsFolderShape &&other)
    {
        mBaseShape = other.mBaseShape;
        mProps = std::move(other.mProps);
        return *this;
    }

    EwsFolderShape &operator=(const EwsFolderShape &other)
    {
        mBaseShape = other.mBaseShape;
        mProps = other.mProps;
        return *this;
    }

    void write(QXmlStreamWriter &writer) const;

    friend EwsFolderShape &operator<<(EwsFolderShape &shape, const EwsPropertyField &prop);

protected:
    void writeBaseShape(QXmlStreamWriter &writer) const;
    void writeProperties(QXmlStreamWriter &writer) const;

    EwsBaseShape mBaseShape;
    QVector<EwsPropertyField> mProps;
};

inline EwsFolderShape &operator<<(EwsFolderShape &shape, const EwsPropertyField &prop)
{
    shape.mProps.append(prop);
    return shape;
}
