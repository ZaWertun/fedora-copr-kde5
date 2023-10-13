/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QSharedDataPointer>

#include "ewstypes.h"

class EwsAttendeePrivate;
class EwsMailbox;
class QXmlStreamReader;

class EwsAttendee
{
public:
    typedef QVector<EwsAttendee> List;

    EwsAttendee();
    explicit EwsAttendee(QXmlStreamReader &reader);
    EwsAttendee(const EwsAttendee &other);
    EwsAttendee(EwsAttendee &&other);
    virtual ~EwsAttendee();

    EwsAttendee &operator=(const EwsAttendee &other);
    EwsAttendee &operator=(EwsAttendee &&other);

    bool isValid() const;
    const EwsMailbox &mailbox() const;
    EwsEventResponseType response() const;
    QDateTime responseTime() const;

protected:
    QSharedDataPointer<EwsAttendeePrivate> d;
};

Q_DECLARE_METATYPE(EwsAttendee)
Q_DECLARE_METATYPE(EwsAttendee::List)
