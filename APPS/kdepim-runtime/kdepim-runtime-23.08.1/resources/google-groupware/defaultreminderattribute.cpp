/*
    SPDX-FileCopyrightText: 2011-2013 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "defaultreminderattribute.h"

#include <QJsonDocument>
#include <QVariant>

#include <KGAPI/Calendar/Reminder>

using namespace KGAPI2;

DefaultReminderAttribute::DefaultReminderAttribute() = default;

Akonadi::Attribute *DefaultReminderAttribute::clone() const
{
    auto attr = new DefaultReminderAttribute();
    attr->setReminders(m_reminders);

    return attr;
}

void DefaultReminderAttribute::setReminders(const RemindersList &reminders)
{
    m_reminders = reminders;
}

void DefaultReminderAttribute::deserialize(const QByteArray &data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    if (json.isNull()) {
        return;
    }

    const QVariant var = json.toVariant();
    const QVariantList list = var.toList();
    for (const QVariant &l : list) {
        const QVariantMap reminder = l.toMap();

        KGAPI2::ReminderPtr rem(new KGAPI2::Reminder);

        const QString reminderType = reminder[QStringLiteral("type")].toString();
        if (reminderType == QLatin1String("display")) {
            rem->setType(KCalendarCore::Alarm::Display);
        } else if (reminderType == QLatin1String("email")) {
            rem->setType(KCalendarCore::Alarm::Email);
        }

        KCalendarCore::Duration offset(reminder[QStringLiteral("time")].toInt(), KCalendarCore::Duration::Seconds);
        rem->setStartOffset(offset);

        m_reminders << rem;
    }
}

QByteArray DefaultReminderAttribute::serialized() const
{
    QVariantList list;
    list.reserve(m_reminders.count());

    for (const ReminderPtr &rem : std::as_const(m_reminders)) {
        QVariantMap reminder;

        if (rem->type() == KCalendarCore::Alarm::Display) {
            reminder[QStringLiteral("type")] = QLatin1String("display");
        } else if (rem->type() == KCalendarCore::Alarm::Email) {
            reminder[QStringLiteral("type")] = QLatin1String("email");
        }

        reminder[QStringLiteral("time")] = rem->startOffset().asSeconds();

        list << reminder;
    }
    QJsonDocument serialized = QJsonDocument::fromVariant(list);
    return serialized.toJson();
}

KCalendarCore::Alarm::List DefaultReminderAttribute::alarms(KCalendarCore::Incidence *incidence) const
{
    KCalendarCore::Alarm::List alarms;
    alarms.reserve(m_reminders.count());
    for (const ReminderPtr &reminder : std::as_const(m_reminders)) {
        KCalendarCore::Alarm::Ptr alarm(new KCalendarCore::Alarm(incidence));

        alarm->setType(reminder->type());
        alarm->setTime(incidence->dtStart());
        alarm->setStartOffset(reminder->startOffset());
        alarm->setEnabled(true);

        alarms << alarm;
    }

    return alarms;
}

QByteArray DefaultReminderAttribute::type() const
{
    static const QByteArray sType("defaultReminders");
    return sType;
}
