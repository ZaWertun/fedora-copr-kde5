/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QSharedDataPointer>

#include "ewstypes.h"

class EwsOccurrencePrivate;
class EwsId;
class QXmlStreamReader;

class EwsOccurrence
{
public:
    typedef QList<EwsOccurrence> List;

    EwsOccurrence();
    explicit EwsOccurrence(QXmlStreamReader &reader);
    EwsOccurrence(const EwsOccurrence &other);
    EwsOccurrence(EwsOccurrence &&other);
    virtual ~EwsOccurrence();

    EwsOccurrence &operator=(const EwsOccurrence &other);
    EwsOccurrence &operator=(EwsOccurrence &&other);

    bool isValid() const;
    const EwsId &itemId() const;
    QDateTime start() const;
    QDateTime end() const;
    QDateTime originalStart() const;

protected:
    QSharedDataPointer<EwsOccurrencePrivate> d;
};

Q_DECLARE_METATYPE(EwsOccurrence)
Q_DECLARE_METATYPE(EwsOccurrence::List)
