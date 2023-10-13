/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Recurrence>

class QXmlStreamReader;

class EwsRecurrence : public KCalendarCore::Recurrence
{
public:
    EwsRecurrence();
    explicit EwsRecurrence(QXmlStreamReader &reader);
    EwsRecurrence(const EwsRecurrence &other);
    EwsRecurrence &operator=(const EwsRecurrence &other) = delete;

private:
    bool readRelativeYearlyRecurrence(QXmlStreamReader &reader);
    bool readAbsoluteYearlyRecurrence(QXmlStreamReader &reader);
    bool readRelativeMonthlyRecurrence(QXmlStreamReader &reader);
    bool readAbsoluteMonthlyRecurrence(QXmlStreamReader &reader);
    bool readWeeklyRecurrence(QXmlStreamReader &reader);
    bool readDailyRecurrence(QXmlStreamReader &reader);
    bool readEndDateRecurrence(QXmlStreamReader &reader);
    bool readNumberedRecurrence(QXmlStreamReader &reader);

    bool readDow(QXmlStreamReader &reader, QBitArray &dow);
};

Q_DECLARE_METATYPE(EwsRecurrence)
