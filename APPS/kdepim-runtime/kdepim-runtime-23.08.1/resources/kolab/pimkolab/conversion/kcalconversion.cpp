/*
 * SPDX-FileCopyrightText: 2011 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kcalconversion.h"

#include <KCalendarCore/Recurrence>
#include <QDate>
#include <QUrl>
#include <QVector>
#include <vector>

#include "commonconversion.h"
#include "pimkolab_debug.h"
namespace Kolab
{
namespace Conversion
{
// The uid of a contact which refers to the uuid of a contact in the addressbook
#define CUSTOM_KOLAB_CONTACT_UUID "X_KOLAB_CONTACT_UUID"
#define CUSTOM_KOLAB_CONTACT_CUTYPE "X_KOLAB_CONTACT_CUTYPE"
#define CUSTOM_KOLAB_URL "X-KOLAB-URL"

KCalendarCore::Duration toDuration(const Kolab::Duration &d)
{
    int value = 0;
    if (d.hours() || d.minutes() || d.seconds()) {
        value = ((((d.weeks() * 7 + d.days()) * 24 + d.hours()) * 60 + d.minutes()) * 60 + d.seconds());
        if (d.isNegative()) {
            value = -value;
        }
        return {value};
    }
    value = d.weeks() * 7 + d.days();
    if (d.isNegative()) {
        value = -value;
    }
    return {value, KCalendarCore::Duration::Days};
}

Kolab::Duration fromDuration(const KCalendarCore::Duration &d)
{
    int value = d.value();
    bool isNegative = false;
    if (value < 0) {
        isNegative = true;
        value = -value;
    }
    // We don't know how the seconds/days were distributed before, so no point in distributing them (probably)
    if (d.isDaily()) {
        int days = value;
        return {days, 0, 0, 0, isNegative};
    }
    int seconds = value;
    //         int minutes = seconds / 60;
    //         seconds = seconds % 60;
    //         int hours = minutes / 60;
    //         minutes = minutes % 60;
    return {0, 0, 0, seconds, isNegative};
}

KCalendarCore::Incidence::Secrecy toSecrecy(Kolab::Classification c)
{
    switch (c) {
    case Kolab::ClassPublic:
        return KCalendarCore::Incidence::SecrecyPublic;
    case Kolab::ClassPrivate:
        return KCalendarCore::Incidence::SecrecyPrivate;
    case Kolab::ClassConfidential:
        return KCalendarCore::Incidence::SecrecyConfidential;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return KCalendarCore::Incidence::SecrecyPublic;
}

Kolab::Classification fromSecrecy(KCalendarCore::Incidence::Secrecy c)
{
    switch (c) {
    case KCalendarCore::Incidence::SecrecyPublic:
        return Kolab::ClassPublic;
    case KCalendarCore::Incidence::SecrecyPrivate:
        return Kolab::ClassPrivate;
    case KCalendarCore::Incidence::SecrecyConfidential:
        return Kolab::ClassConfidential;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::ClassPublic;
}

int toPriority(int priority)
{
    // Same mapping
    return priority;
}

int fromPriority(int priority)
{
    // Same mapping
    return priority;
}

KCalendarCore::Incidence::Status toStatus(Kolab::Status s)
{
    switch (s) {
    case Kolab::StatusUndefined:
        return KCalendarCore::Incidence::StatusNone;
    case Kolab::StatusNeedsAction:
        return KCalendarCore::Incidence::StatusNeedsAction;
    case Kolab::StatusCompleted:
        return KCalendarCore::Incidence::StatusCompleted;
    case Kolab::StatusInProcess:
        return KCalendarCore::Incidence::StatusInProcess;
    case Kolab::StatusCancelled:
        return KCalendarCore::Incidence::StatusCanceled;
    case Kolab::StatusTentative:
        return KCalendarCore::Incidence::StatusTentative;
    case Kolab::StatusConfirmed:
        return KCalendarCore::Incidence::StatusConfirmed;
    case Kolab::StatusDraft:
        return KCalendarCore::Incidence::StatusDraft;
    case Kolab::StatusFinal:
        return KCalendarCore::Incidence::StatusFinal;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return KCalendarCore::Incidence::StatusNone;
}

Kolab::Status fromStatus(KCalendarCore::Incidence::Status s)
{
    switch (s) {
    case KCalendarCore::Incidence::StatusNone:
        return Kolab::StatusUndefined;
    case KCalendarCore::Incidence::StatusNeedsAction:
        return Kolab::StatusNeedsAction;
    case KCalendarCore::Incidence::StatusCompleted:
        return Kolab::StatusCompleted;
    case KCalendarCore::Incidence::StatusInProcess:
        return Kolab::StatusInProcess;
    case KCalendarCore::Incidence::StatusCanceled:
        return Kolab::StatusCancelled;
    case KCalendarCore::Incidence::StatusTentative:
        return Kolab::StatusTentative;
    case KCalendarCore::Incidence::StatusConfirmed:
        return Kolab::StatusConfirmed;
    case KCalendarCore::Incidence::StatusDraft:
        return Kolab::StatusDraft;
    case KCalendarCore::Incidence::StatusFinal:
        return Kolab::StatusFinal;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::StatusUndefined;
}

KCalendarCore::Attendee::PartStat toPartStat(Kolab::PartStatus p)
{
    switch (p) {
    case Kolab::PartNeedsAction:
        return KCalendarCore::Attendee::NeedsAction;
    case Kolab::PartAccepted:
        return KCalendarCore::Attendee::Accepted;
    case Kolab::PartDeclined:
        return KCalendarCore::Attendee::Declined;
    case Kolab::PartTentative:
        return KCalendarCore::Attendee::Tentative;
    case Kolab::PartDelegated:
        return KCalendarCore::Attendee::Delegated;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return KCalendarCore::Attendee::NeedsAction;
}

Kolab::PartStatus fromPartStat(KCalendarCore::Attendee::PartStat p)
{
    switch (p) {
    case KCalendarCore::Attendee::NeedsAction:
        return Kolab::PartNeedsAction;
    case KCalendarCore::Attendee::Accepted:
        return Kolab::PartAccepted;
    case KCalendarCore::Attendee::Declined:
        return Kolab::PartDeclined;
    case KCalendarCore::Attendee::Tentative:
        return Kolab::PartTentative;
    case KCalendarCore::Attendee::Delegated:
        return Kolab::PartDelegated;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::PartNeedsAction;
}

KCalendarCore::Attendee::Role toRole(Kolab::Role r)
{
    switch (r) {
    case Kolab::Required:
        return KCalendarCore::Attendee::ReqParticipant;
    case Kolab::Chair:
        return KCalendarCore::Attendee::Chair;
    case Kolab::Optional:
        return KCalendarCore::Attendee::OptParticipant;
    case Kolab::NonParticipant:
        return KCalendarCore::Attendee::NonParticipant;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return KCalendarCore::Attendee::ReqParticipant;
}

Kolab::Role fromRole(KCalendarCore::Attendee::Role r)
{
    switch (r) {
    case KCalendarCore::Attendee::ReqParticipant:
        return Kolab::Required;
    case KCalendarCore::Attendee::Chair:
        return Kolab::Chair;
    case KCalendarCore::Attendee::OptParticipant:
        return Kolab::Optional;
    case KCalendarCore::Attendee::NonParticipant:
        return Kolab::NonParticipant;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::Required;
}

template<typename T>
QString getCustomProperty(const QString &id, const T &e)
{
    const std::vector<Kolab::CustomProperty> &props = e.customProperties();
    for (const Kolab::CustomProperty &p : props) {
        if (fromStdString(p.identifier) == id) {
            return fromStdString(p.value);
        }
    }
}

template<typename T>
void setIncidence(KCalendarCore::Incidence &i, const T &e)
{
    if (!e.uid().empty()) {
        i.setUid(fromStdString(e.uid()));
    }

    i.setCreated(toDate(e.created()));
    i.setLastModified(toDate(e.lastModified()));
    i.setRevision(e.sequence());
    i.setSecrecy(toSecrecy(e.classification()));
    i.setCategories(toStringList(e.categories()));

    if (e.start().isValid()) {
        i.setDtStart(toDate(e.start()));
        i.setAllDay(e.start().isDateOnly());
    }

    i.setSummary(fromStdString(e.summary())); // TODO detect richtext
    i.setDescription(fromStdString(e.description())); // TODO detect richtext
    i.setStatus(toStatus(e.status()));
    const auto attendees{e.attendees()};
    for (const Kolab::Attendee &a : attendees) {
        /*
         * KCalendarCore always sets a UID if empty, but that's just a pointer, and not the uid of a real contact.
         * Since that means the semantics of the two are different, we have to store the kolab uid as a custom property.
         */
        KCalendarCore::Attendee attendee(fromStdString(a.contact().name()),
                                         fromStdString(a.contact().email()),
                                         a.rsvp(),
                                         toPartStat(a.partStat()),
                                         toRole(a.role()));
        if (!a.contact().uid().empty()) { // TODO Identify contact from addressbook based on uid
            attendee.customProperties().setNonKDECustomProperty(CUSTOM_KOLAB_CONTACT_UUID, fromStdString(a.contact().uid()));
        }
        if (!a.delegatedTo().empty()) {
            if (a.delegatedTo().size() > 1) {
                qCWarning(PIMKOLAB_LOG) << "multiple delegatees are not supported";
            }
            attendee.setDelegate(toMailto(a.delegatedTo().front().email(), a.delegatedTo().front().name()).toString());
        }
        if (!a.delegatedFrom().empty()) {
            if (a.delegatedFrom().size() > 1) {
                qCWarning(PIMKOLAB_LOG) << "multiple delegators are not supported";
            }
            attendee.setDelegator(toMailto(a.delegatedFrom().front().email(), a.delegatedFrom().front().name()).toString());
        }
        if (a.cutype() != Kolab::CutypeIndividual) {
            attendee.customProperties().setNonKDECustomProperty(CUSTOM_KOLAB_CONTACT_CUTYPE, QString::number(a.cutype()));
        }
        i.addAttendee(attendee);
    }
    const auto attachments{e.attachments()};
    for (const Kolab::Attachment &a : attachments) {
        KCalendarCore::Attachment att;
        if (!a.uri().empty()) {
            att = KCalendarCore::Attachment(fromStdString(a.uri()), fromStdString(a.mimetype()));
        } else {
            att = KCalendarCore::Attachment(QByteArray::fromRawData(a.data().c_str(), a.data().size()).toBase64(), fromStdString(a.mimetype()));
        }
        if (!a.label().empty()) {
            att.setLabel(fromStdString(a.label()));
        }
        i.addAttachment(att);
    }

    QMap<QByteArray, QString> props;
    const auto customProperties{e.customProperties()};
    for (const Kolab::CustomProperty &prop : customProperties) {
        QString key;
        if (prop.identifier.compare(0, 5, "X-KDE")) {
            key.append(QLatin1String("X-KOLAB-"));
        }
        key.append(fromStdString(prop.identifier));
        props.insert(key.toLatin1(), fromStdString(prop.value));
        //         i.setCustomProperty("KOLAB", fromStdString(prop.identifier).toLatin1(), fromStdString(prop.value));
    }
    i.setCustomProperties(props);
}

template<typename T, typename I>
void getIncidence(T &i, const I &e)
{
    i.setUid(toStdString(e.uid()));
    i.setCreated(fromDate(e.created(), false));
    i.setLastModified(fromDate(e.lastModified(), false));
    i.setSequence(e.revision());
    i.setClassification(fromSecrecy(e.secrecy()));
    i.setCategories(fromStringList(e.categories()));

    i.setStart(fromDate(e.dtStart(), e.allDay()));
    i.setSummary(toStdString(e.summary()));
    i.setDescription(toStdString(e.description()));
    i.setStatus(fromStatus(e.status()));
    std::vector<Kolab::Attendee> attendees;
    const auto eAttendees{e.attendees()};
    for (const KCalendarCore::Attendee &ptr : eAttendees) {
        const QString &uid = ptr.customProperties().nonKDECustomProperty(CUSTOM_KOLAB_CONTACT_UUID);
        Kolab::Attendee a(Kolab::ContactReference(toStdString(ptr.email()), toStdString(ptr.name()), toStdString(uid)));
        a.setRSVP(ptr.RSVP());
        a.setPartStat(fromPartStat(ptr.status()));
        a.setRole(fromRole(ptr.role()));
        if (!ptr.delegate().isEmpty()) {
            std::string name;
            const std::string &email = fromMailto(QUrl(ptr.delegate()), name);
            a.setDelegatedTo(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(email, name));
        }
        if (!ptr.delegator().isEmpty()) {
            std::string name;
            const std::string &email = fromMailto(QUrl(ptr.delegator()), name);
            a.setDelegatedFrom(std::vector<Kolab::ContactReference>() << Kolab::ContactReference(email, name));
        }
        const QString &cutype = ptr.customProperties().nonKDECustomProperty(CUSTOM_KOLAB_CONTACT_CUTYPE);
        if (!cutype.isEmpty()) {
            a.setCutype(static_cast<Kolab::Cutype>(cutype.toInt()));
        }

        attendees.push_back(a);
    }
    i.setAttendees(attendees);
    std::vector<Kolab::Attachment> attachments;
    const auto eAttachments{e.attachments()};
    for (const KCalendarCore::Attachment &att : eAttachments) {
        Kolab::Attachment a;
        if (att.isUri()) {
            a.setUri(toStdString(att.uri()), toStdString(att.mimeType()));
        } else {
            a.setData(std::string(att.decodedData().data(), att.decodedData().size()), toStdString(att.mimeType()));
        }
        a.setLabel(toStdString(att.label()));
        attachments.push_back(a);
    }
    i.setAttachments(attachments);

    std::vector<Kolab::CustomProperty> customProperties;
    const QMap<QByteArray, QString> &props = e.customProperties();
    for (QMap<QByteArray, QString>::const_iterator it = props.cbegin(), end(props.cend()); it != end; ++it) {
        QString key(QString::fromUtf8(it.key()));
        if (key == QLatin1String(CUSTOM_KOLAB_URL)) {
            continue;
        }
        customProperties.push_back(Kolab::CustomProperty(toStdString(key.remove(QStringLiteral("X-KOLAB-"))), toStdString(it.value())));
    }
    i.setCustomProperties(customProperties);
}

int toWeekDay(Kolab::Weekday wday)
{
    switch (wday) {
    case Kolab::Monday:
        return 1;
    case Kolab::Tuesday:
        return 2;
    case Kolab::Wednesday:
        return 3;
    case Kolab::Thursday:
        return 4;
    case Kolab::Friday:
        return 5;
    case Kolab::Saturday:
        return 6;
    case Kolab::Sunday:
        return 7;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return 1;
}

Kolab::Weekday fromWeekDay(int wday)
{
    switch (wday) {
    case 1:
        return Kolab::Monday;
    case 2:
        return Kolab::Tuesday;
    case 3:
        return Kolab::Wednesday;
    case 4:
        return Kolab::Thursday;
    case 5:
        return Kolab::Friday;
    case 6:
        return Kolab::Saturday;
    case 7:
        return Kolab::Sunday;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::Monday;
}

KCalendarCore::RecurrenceRule::PeriodType toRecurrenceType(Kolab::RecurrenceRule::Frequency freq)
{
    switch (freq) {
    case Kolab::RecurrenceRule::FreqNone:
        qCWarning(PIMKOLAB_LOG) << "no recurrence?";
        break;
    case Kolab::RecurrenceRule::Yearly:
        return KCalendarCore::RecurrenceRule::rYearly;
    case Kolab::RecurrenceRule::Monthly:
        return KCalendarCore::RecurrenceRule::rMonthly;
    case Kolab::RecurrenceRule::Weekly:
        return KCalendarCore::RecurrenceRule::rWeekly;
    case Kolab::RecurrenceRule::Daily:
        return KCalendarCore::RecurrenceRule::rDaily;
    case Kolab::RecurrenceRule::Hourly:
        return KCalendarCore::RecurrenceRule::rHourly;
    case Kolab::RecurrenceRule::Minutely:
        return KCalendarCore::RecurrenceRule::rMinutely;
    case Kolab::RecurrenceRule::Secondly:
        return KCalendarCore::RecurrenceRule::rSecondly;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return KCalendarCore::RecurrenceRule::rNone;
}

Kolab::RecurrenceRule::Frequency fromRecurrenceType(KCalendarCore::RecurrenceRule::PeriodType freq)
{
    switch (freq) {
    case KCalendarCore::RecurrenceRule::rNone:
        qCWarning(PIMKOLAB_LOG) << "no recurrence?";
        break;
    case KCalendarCore::RecurrenceRule::rYearly:
        return Kolab::RecurrenceRule::Yearly;
    case KCalendarCore::RecurrenceRule::rMonthly:
        return Kolab::RecurrenceRule::Monthly;
    case KCalendarCore::RecurrenceRule::rWeekly:
        return Kolab::RecurrenceRule::Weekly;
    case KCalendarCore::RecurrenceRule::rDaily:
        return Kolab::RecurrenceRule::Daily;
    case KCalendarCore::RecurrenceRule::rHourly:
        return Kolab::RecurrenceRule::Hourly;
    case KCalendarCore::RecurrenceRule::rMinutely:
        return Kolab::RecurrenceRule::Minutely;
    case KCalendarCore::RecurrenceRule::rSecondly:
        return Kolab::RecurrenceRule::Secondly;
    default:
        qCCritical(PIMKOLAB_LOG) << "unhandled";
        Q_ASSERT(0);
    }
    return Kolab::RecurrenceRule::FreqNone;
}

KCalendarCore::RecurrenceRule::WDayPos toWeekDayPos(Kolab::DayPos dp)
{
    return KCalendarCore::RecurrenceRule::WDayPos(dp.occurence(), toWeekDay(dp.weekday()));
}

Kolab::DayPos fromWeekDayPos(KCalendarCore::RecurrenceRule::WDayPos dp)
{
    return {dp.pos(), fromWeekDay(dp.day())};
}

template<typename T>
void setRecurrence(KCalendarCore::Incidence &e, const T &event)
{
    const Kolab::RecurrenceRule &rrule = event.recurrenceRule();
    if (rrule.isValid()) {
        KCalendarCore::Recurrence *rec = e.recurrence();

        KCalendarCore::RecurrenceRule *defaultRR = rec->defaultRRule(true);
        Q_ASSERT(defaultRR);

        defaultRR->setWeekStart(toWeekDay(rrule.weekStart()));
        defaultRR->setRecurrenceType(toRecurrenceType(rrule.frequency()));
        defaultRR->setFrequency(rrule.interval());

        if (rrule.end().isValid()) {
            rec->setEndDateTime(toDate(rrule.end())); // TODO date/datetime setEndDate(). With date-only the start date has to be taken into account.
        } else {
            rec->setDuration(rrule.count());
        }

        if (!rrule.bysecond().empty()) {
            const std::vector<int> bySecond = rrule.bysecond();
            const QVector<int> stdVector = QVector<int>(bySecond.begin(), bySecond.end());
            defaultRR->setBySeconds(stdVector.toList());
        }
        if (!rrule.byminute().empty()) {
            const std::vector<int> byMinutes = rrule.byminute();
            const QVector<int> stdVector = QVector<int>(byMinutes.begin(), byMinutes.end());
            defaultRR->setByMinutes(stdVector.toList());
        }
        if (!rrule.byhour().empty()) {
            const std::vector<int> byHours = rrule.byhour();
            const QVector<int> stdVector = QVector<int>(byHours.begin(), byHours.end());
            defaultRR->setByHours(stdVector.toList());
        }
        if (!rrule.byday().empty()) {
            QList<KCalendarCore::RecurrenceRule::WDayPos> daypos;
            const auto bydays{rrule.byday()};
            for (const Kolab::DayPos &dp : bydays) {
                daypos.append(toWeekDayPos(dp));
            }
            defaultRR->setByDays(daypos);
        }
        if (!rrule.bymonthday().empty()) {
            const std::vector<int> byMonthDays = rrule.bymonthday();
            const QVector<int> stdVector = QVector<int>(byMonthDays.begin(), byMonthDays.end());
            defaultRR->setByMonthDays(stdVector.toList());
        }
        if (!rrule.byyearday().empty()) {
            const std::vector<int> byYearDays = rrule.byyearday();
            const QVector<int> stdVector = QVector<int>(byYearDays.begin(), byYearDays.end());
            defaultRR->setByYearDays(stdVector.toList());
        }
        if (!rrule.byweekno().empty()) {
            const std::vector<int> byWeekNumbers = rrule.byweekno();
            const QVector<int> stdVector = QVector<int>(byWeekNumbers.begin(), byWeekNumbers.end());
            defaultRR->setByWeekNumbers(stdVector.toList());
        }
        if (!rrule.bymonth().empty()) {
            const std::vector<int> byMonths = rrule.bymonth();
            const QVector<int> stdVector = QVector<int>(byMonths.begin(), byMonths.end());
            defaultRR->setByMonths(stdVector.toList());
        }
    }
    const auto recurrenceDates{event.recurrenceDates()};
    for (const Kolab::cDateTime &dt : recurrenceDates) {
        const QDateTime &date = toDate(dt);
        if (dt.isDateOnly()) {
            e.recurrence()->addRDate(date.date());
        } else {
            e.recurrence()->addRDateTime(date);
        }
    }
    const auto exceptionDates{event.exceptionDates()};
    for (const Kolab::cDateTime &dt : exceptionDates) {
        const QDateTime &date = toDate(dt);
        if (dt.isDateOnly()) {
            e.recurrence()->addExDate(date.date());
        } else {
            e.recurrence()->addExDateTime(date);
        }
    }
}

template<typename T, typename I>
void getRecurrence(T &i, const I &e)
{
    if (!e.recurs()) {
        return;
    }
    KCalendarCore::Recurrence *rec = e.recurrence();
    KCalendarCore::RecurrenceRule *defaultRR = rec->defaultRRule(false);
    if (!defaultRR) {
        qCWarning(PIMKOLAB_LOG) << "no recurrence";
        return;
    }
    Q_ASSERT(defaultRR);

    Kolab::RecurrenceRule rrule;
    rrule.setWeekStart(fromWeekDay(defaultRR->weekStart()));
    rrule.setFrequency(fromRecurrenceType(defaultRR->recurrenceType()));
    rrule.setInterval(defaultRR->frequency());

    if (defaultRR->duration() != 0) { // Inidcates if end date is set or not
        if (defaultRR->duration() > 0) {
            rrule.setCount(defaultRR->duration());
        }
    } else {
        rrule.setEnd(fromDate(defaultRR->endDt(), e.allDay()));
    }

    const QVector<int> bySecondsVector = defaultRR->bySeconds().toVector();
    const auto stdVectorBySeconds = std::vector<int>(bySecondsVector.begin(), bySecondsVector.end());
    rrule.setBysecond(stdVectorBySeconds);

    const QVector<int> byMinutesVector = defaultRR->byMinutes().toVector();
    const auto stdVectorByMinutes = std::vector<int>(byMinutesVector.begin(), byMinutesVector.end());
    rrule.setByminute(stdVectorByMinutes);

    const QVector<int> byHoursVector = defaultRR->byHours().toVector();
    const auto stdVectorByHours = std::vector<int>(byHoursVector.begin(), byHoursVector.end());
    rrule.setByhour(stdVectorByHours);

    std::vector<Kolab::DayPos> daypos;
    const auto defaultRRByDays{defaultRR->byDays()};
    daypos.reserve(defaultRRByDays.count());

    for (const KCalendarCore::RecurrenceRule::WDayPos &dp : defaultRRByDays) {
        daypos.push_back(fromWeekDayPos(dp));
    }
    rrule.setByday(daypos);

    const QVector<int> bymonthdayVector = defaultRR->byMonthDays().toVector();
    const auto stdByMonthDayVector = std::vector<int>(bymonthdayVector.begin(), bymonthdayVector.end());
    rrule.setBymonthday(stdByMonthDayVector);

    const QVector<int> byYearDaysVector = defaultRR->byYearDays().toVector();
    const auto stdByYearDayVector = std::vector<int>(byYearDaysVector.begin(), byYearDaysVector.end());
    rrule.setByyearday(stdByYearDayVector);

    const QVector<int> byWeekNumberVector = defaultRR->byWeekNumbers().toVector();
    const auto stdWeekNumberVector = std::vector<int>(byWeekNumberVector.begin(), byWeekNumberVector.end());
    rrule.setByweekno(stdWeekNumberVector);

    const QVector<int> byMonthVector = defaultRR->byMonths().toVector();
    const auto stdByMonthVector = std::vector<int>(byMonthVector.begin(), byMonthVector.end());
    rrule.setBymonth(stdByMonthVector);

    i.setRecurrenceRule(rrule);

    std::vector<Kolab::cDateTime> rdates;
    const auto rDateTimes{rec->rDateTimes()};
    for (const QDateTime &dt : rDateTimes) {
        rdates.push_back(fromDate(dt, e.allDay()));
    }
    const auto recRDates{rec->rDates()};
    for (const QDate &dt : recRDates) {
        rdates.push_back(fromDate(QDateTime(dt, {}), true));
    }
    i.setRecurrenceDates(rdates);

    std::vector<Kolab::cDateTime> exdates;
    const auto recExDateTimes{rec->exDateTimes()};
    for (const QDateTime &dt : recExDateTimes) {
        exdates.push_back(fromDate(dt, e.allDay()));
    }
    const auto exDates = rec->exDates();
    for (const QDate &dt : exDates) {
        exdates.push_back(fromDate(QDateTime(dt, {}), true));
    }
    i.setExceptionDates(exdates);

    if (!rec->exRules().empty()) {
        qCWarning(PIMKOLAB_LOG) << "exrules are not supported";
    }
}

template<typename T>
void setTodoEvent(KCalendarCore::Incidence &i, const T &e)
{
    i.setPriority(toPriority(e.priority()));
    if (!e.location().empty()) {
        i.setLocation(fromStdString(e.location())); // TODO detect richtext
    }
    if (e.organizer().isValid()) {
        i.setOrganizer(KCalendarCore::Person(fromStdString(e.organizer().name()), fromStdString(e.organizer().email()))); // TODO handle uid too
    }
    if (!e.url().empty()) {
        i.setNonKDECustomProperty(CUSTOM_KOLAB_URL, fromStdString(e.url()));
    }
    if (e.recurrenceID().isValid()) {
        i.setRecurrenceId(toDate(e.recurrenceID())); // TODO THISANDFUTURE
    }
    setRecurrence(i, e);
    const auto alarms{e.alarms()};
    for (const Kolab::Alarm &a : alarms) {
        KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(&i));
        switch (a.type()) {
        case Kolab::Alarm::EMailAlarm: {
            KCalendarCore::Person::List receipents;
            const auto aAttendees{a.attendees()};
            for (Kolab::ContactReference c : aAttendees) {
                KCalendarCore::Person person(fromStdString(c.name()), fromStdString(c.email()));
                receipents.append(person);
            }
            alarm->setEmailAlarm(fromStdString(a.summary()), fromStdString(a.description()), receipents);
            break;
        }
        case Kolab::Alarm::DisplayAlarm:
            alarm->setDisplayAlarm(fromStdString(a.text()));
            break;
        case Kolab::Alarm::AudioAlarm:
            alarm->setAudioAlarm(fromStdString(a.audioFile().uri()));
            break;
        default:
            qCCritical(PIMKOLAB_LOG) << "invalid alarm";
        }

        if (a.start().isValid()) {
            alarm->setTime(toDate(a.start()));
        } else if (a.relativeStart().isValid()) {
            if (a.relativeTo() == Kolab::End) {
                alarm->setEndOffset(toDuration(a.relativeStart()));
            } else {
                alarm->setStartOffset(toDuration(a.relativeStart()));
            }
        }

        alarm->setSnoozeTime(toDuration(a.duration()));
        alarm->setRepeatCount(a.numrepeat());
        alarm->setEnabled(true);
        i.addAlarm(alarm);
    }
}

template<typename T, typename I>
void getTodoEvent(T &i, const I &e)
{
    i.setPriority(fromPriority(e.priority()));
    i.setLocation(toStdString(e.location()));
    if (!e.organizer().email().isEmpty()) {
        i.setOrganizer(Kolab::ContactReference(Kolab::ContactReference::EmailReference,
                                               toStdString(e.organizer().email()),
                                               toStdString(e.organizer().name()))); // TODO handle uid too
    }
    i.setUrl(toStdString(e.nonKDECustomProperty(CUSTOM_KOLAB_URL)));
    i.setRecurrenceID(fromDate(e.recurrenceId(), e.allDay()), false); // TODO THISANDFUTURE
    getRecurrence(i, e);
    std::vector<Kolab::Alarm> alarms;
    const auto eAlarms{e.alarms()};
    for (const KCalendarCore::Alarm::Ptr &a : eAlarms) {
        Kolab::Alarm alarm;
        // TODO KCalendarCore disables alarms using KCalendarCore::Alarm::enabled() (X-KDE-KCALCORE-ENABLED) We should either delete the alarm, or store the
        // attribute . Ideally we would store the alarm somewhere and temporarily delete it, so we can restore it when parsing. For now we just remove disabled
        // alarms.
        if (!a->enabled()) {
            qCWarning(PIMKOLAB_LOG) << "skipping disabled alarm";
            continue;
        }
        switch (a->type()) {
        case KCalendarCore::Alarm::Display:
            alarm = Kolab::Alarm(toStdString(a->text()));
            break;
        case KCalendarCore::Alarm::Email: {
            std::vector<Kolab::ContactReference> receipents;
            const auto mailAddresses = a->mailAddresses();
            for (const KCalendarCore::Person &p : mailAddresses) {
                receipents.emplace_back(toStdString(p.email()), toStdString(p.name()));
            }
            alarm = Kolab::Alarm(toStdString(a->mailSubject()), toStdString(a->mailText()), receipents);
            break;
        }
        case KCalendarCore::Alarm::Audio: {
            Kolab::Attachment audioFile;
            audioFile.setUri(toStdString(a->audioFile()), std::string());
            alarm = Kolab::Alarm(audioFile);
            break;
        }
        default:
            qCCritical(PIMKOLAB_LOG) << "unhandled alarm";
        }

        if (a->hasTime()) {
            alarm.setStart(fromDate(a->time(), false));
        } else if (a->hasStartOffset()) {
            alarm.setRelativeStart(fromDuration(a->startOffset()), Kolab::Start);
        } else if (a->hasEndOffset()) {
            alarm.setRelativeStart(fromDuration(a->endOffset()), Kolab::End);
        } else {
            qCCritical(PIMKOLAB_LOG) << "alarm trigger is missing";
            continue;
        }

        alarm.setDuration(fromDuration(a->snoozeTime()), a->repeatCount());

        alarms.push_back(alarm);
    }
    i.setAlarms(alarms);
}

KCalendarCore::Event::Ptr toKCalendarCore(const Kolab::Event &event)
{
    KCalendarCore::Event::Ptr e(new KCalendarCore::Event);
    setIncidence(*e, event);
    setTodoEvent(*e, event);
    if (event.end().isValid()) {
        e->setDtEnd(toDate(event.end()));
    }
    if (event.duration().isValid()) {
        e->setDuration(toDuration(event.duration()));
    }
    if (event.transparency()) {
        e->setTransparency(KCalendarCore::Event::Transparent);
    } else {
        e->setTransparency(KCalendarCore::Event::Opaque);
    }
    return e;
}

Kolab::Event fromKCalendarCore(const KCalendarCore::Event &event)
{
    Kolab::Event e;
    getIncidence(e, event);
    getTodoEvent(e, event);
    if (event.hasEndDate()) {
        e.setEnd(fromDate(event.dtEnd(), event.allDay()));
    } else if (event.hasDuration()) {
        e.setDuration(fromDuration(event.duration()));
    }
    if (event.transparency() == KCalendarCore::Event::Transparent) {
        e.setTransparency(true);
    } else {
        e.setTransparency(false);
    }
    return e;
}

KCalendarCore::Todo::Ptr toKCalendarCore(const Kolab::Todo &todo)
{
    KCalendarCore::Todo::Ptr e(new KCalendarCore::Todo);
    setIncidence(*e, todo);
    setTodoEvent(*e, todo);
    if (todo.due().isValid()) {
        e->setDtDue(toDate(todo.due()));
    }
    if (!todo.relatedTo().empty()) {
        e->setRelatedTo(Kolab::Conversion::fromStdString(todo.relatedTo().front()), KCalendarCore::Incidence::RelTypeParent);
        if (todo.relatedTo().size() > 1) {
            qCCritical(PIMKOLAB_LOG) << "only one relation support but got multiple";
        }
    }
    e->setPercentComplete(todo.percentComplete());
    return e;
}

Kolab::Todo fromKCalendarCore(const KCalendarCore::Todo &todo)
{
    Kolab::Todo t;
    getIncidence(t, todo);
    getTodoEvent(t, todo);
    t.setDue(fromDate(todo.dtDue(true), todo.allDay()));
    t.setPercentComplete(todo.percentComplete());
    const QString relatedTo = todo.relatedTo(KCalendarCore::Incidence::RelTypeParent);
    if (!relatedTo.isEmpty()) {
        std::vector<std::string> relateds;
        relateds.push_back(Kolab::Conversion::toStdString(relatedTo));
        t.setRelatedTo(relateds);
    }
    return t;
}

KCalendarCore::Journal::Ptr toKCalendarCore(const Kolab::Journal &journal)
{
    KCalendarCore::Journal::Ptr e(new KCalendarCore::Journal);
    setIncidence(*e, journal);
    // TODO contacts
    return e;
}

Kolab::Journal fromKCalendarCore(const KCalendarCore::Journal &journal)
{
    Kolab::Journal j;
    getIncidence(j, journal);
    // TODO contacts
    return j;
}
}
}
