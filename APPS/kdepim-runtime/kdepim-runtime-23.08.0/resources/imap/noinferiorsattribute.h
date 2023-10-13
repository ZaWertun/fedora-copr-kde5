/*
    SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <Akonadi/Attribute>

class NoInferiorsAttribute : public Akonadi::Attribute
{
public:
    explicit NoInferiorsAttribute(bool noInferiors = false);
    void setNoInferiors(bool noInferiors);
    Q_REQUIRED_RESULT bool noInferiors() const;
    Q_REQUIRED_RESULT QByteArray type() const override;
    Attribute *clone() const override;
    Q_REQUIRED_RESULT QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    bool mNoInferiors;
};
