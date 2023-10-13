/*
    SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "noinferiorsattribute.h"

#include <QByteArray>

NoInferiorsAttribute::NoInferiorsAttribute(bool noInferiors)
    : mNoInferiors(noInferiors)
{
}

void NoInferiorsAttribute::setNoInferiors(bool noInferiors)
{
    mNoInferiors = noInferiors;
}

bool NoInferiorsAttribute::noInferiors() const
{
    return mNoInferiors;
}

QByteArray NoInferiorsAttribute::type() const
{
    static const QByteArray sType("noinferiors");
    return sType;
}

Akonadi::Attribute *NoInferiorsAttribute::clone() const
{
    return new NoInferiorsAttribute(mNoInferiors);
}

QByteArray NoInferiorsAttribute::serialized() const
{
    return mNoInferiors ? QByteArray::number(1) : QByteArray::number(0);
}

void NoInferiorsAttribute::deserialize(const QByteArray &data)
{
    mNoInferiors = (data.toInt() == 0) ? false : true;
}
