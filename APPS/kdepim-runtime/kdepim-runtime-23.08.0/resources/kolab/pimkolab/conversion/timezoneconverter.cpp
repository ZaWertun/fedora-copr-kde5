/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "timezoneconverter.h"
#include <QRegularExpression>

#include "pimkolab_debug.h"
#include <QTimeZone>
QString TimezoneConverter::normalizeTimezone(const QString &tz)
{
    if (QTimeZone::isTimeZoneIdAvailable(tz.toLatin1())) {
        return tz;
    }
    auto normalizedId = QTimeZone::windowsIdToDefaultIanaId(tz.toLatin1());
    if (!normalizedId.isEmpty()) {
        return QString::fromUtf8(normalizedId);
    }
    // We're dealing with an invalid or unknown timezone, try to parse it
    QString guessedTimezone = fromCityName(tz);
    if (guessedTimezone.isEmpty()) {
        guessedTimezone = fromHardcodedList(tz);
    }
    if (guessedTimezone.isEmpty()) {
        guessedTimezone = fromGMTOffsetTimezone(tz);
    }
    qCDebug(PIMKOLAB_LOG) << "Guessed timezone and found: " << guessedTimezone;
    return guessedTimezone;
}

QString TimezoneConverter::fromGMTOffsetTimezone(const QString &tz)
{
    Q_UNUSED(tz)
    return {};
}

QString TimezoneConverter::fromCityName(const QString &tz)
{
    const auto zones = QTimeZone::availableTimeZoneIds();
    QHash<QString, QString> countryMap;
    for (const auto &zone : zones) {
        const QString cityName = QString::fromUtf8(zone.split('/').last());
        Q_ASSERT(!countryMap.contains(cityName));
        countryMap.insert(cityName, QString::fromUtf8(zone));
    }

    static const QRegularExpression locationFinder(QStringLiteral("\\b([a-zA-Z])+\\b"));
    QRegularExpressionMatchIterator iter = locationFinder.globalMatch(tz);
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        const QString location = match.captured(0);
        qCDebug(PIMKOLAB_LOG) << "location " << location;
        if (countryMap.contains(location)) {
            qCDebug(PIMKOLAB_LOG) << "found match " << countryMap.value(location);
            return countryMap.value(location);
        }
    }
    return {};
}

// Based on
// * http://msdn.microsoft.com/en-us/library/ms912391(v=winembedded.11).aspx
// * http://technet.microsoft.com/en-us/library/cc749073(v=ws.10).aspx
// * http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml
// * http://stackoverflow.com/questions/4967903/linux-windows-timezone-mapping
static const struct WindowsTimezone {
    //   const int gmtOffset;
    const char *timezoneSpecifier; // This one should be stable and always in english
    const char *name; // The display name (which is in some cases still useful to try guessing)
    const char *olson[28]; // Corresponding olson timezones we can map to
} windowsTimezones[] = {
    {"Afghanistan Standard Time", "Kabul", {"Asia/Kabul", "Asia/Kabul"}},
    {"Alaskan Standard Time", "Alaska", {"America/Anchorage", "America/Anchorage America/Juneau America/Nome America/Sitka America/Yakutat"}},
    {"Arab Standard Time", "Kuwait, Riyadh", {"Asia/Riyadh", "Asia/Bahrain", "Asia/Kuwait", "Asia/Qatar", "Asia/Riyadh", "Asia/Aden"}},
    {"Arabian Standard Time", "Abu Dhabi, Muscat", {"Asia/Dubai", "Asia/Dubai", "Asia/Muscat", "Etc/GMT-4"}},
    {"Arabic Standard Time", "Baghdad", {"Asia/Baghdad", "Asia/Baghdad"}},
    {"Atlantic Standard Time",
     "Atlantic Time (Canada)",
     {"America/Halifax", "Atlantic/Bermuda", "America/Halifax America/Glace_Bay America/Goose_Bay America/Moncton", "America/Thule"}},
    {"AUS Central Standard Time", "Darwin", {"Australia/Darwin", "Australia/Darwin"}},
    {"AUS Eastern Standard Time", "Canberra, Melbourne, Sydney", {"Australia/Sydney", "Australia/Sydney Australia/Melbourne"}},
    {"Azerbaijan Standard Time", "Baku", {"Asia/Baku", "Asia/Baku"}},
    {"Azores Standard Time", "Azores", {"Atlantic/Azores", "America/Scoresbysund", "Atlantic/Azores"}},
    {"Canada Central Standard Time", "Saskatchewan", {"America/Regina", "America/Regina America/Swift_Current"}},
    {"Cape Verde Standard Time", "Cape Verde Islands", {"Atlantic/Cape_Verde", "Atlantic/Cape_Verde", "Etc/GMT+1"}},
    {"Caucasus Standard Time", "Yerevan", {"Asia/Yerevan", "Asia/Yerevan"}},
    {"Cen. Australia Standard Time", "Adelaide", {"Australia/Adelaide", "Australia/Adelaide Australia/Broken_Hill"}},
    {"Central America Standard Time",
     "Central America",
     {"America/Guatemala",
      "America/Belize",
      "America/Costa_Rica",
      "Pacific/Galapagos",
      "America/Guatemala",
      "America/Tegucigalpa",
      "America/Managua",
      "America/El_Salvador",
      "Etc/GMT+6"}},
    {"Central Asia Standard Time",
     "Astana, Dhaka",
     {"Asia/Almaty", "Antarctica/Vostok", "Indian/Chagos", "Asia/Bishkek", "Asia/Almaty Asia/Qyzylorda", "Etc/GMT-6"}},
    {"Central Brazilian Standard Time", "Manaus", {"America/Cuiaba", "America/Cuiaba America/Campo_Grande"}},
    {"Central Europe Standard Time",
     "Belgrade, Bratislava, Budapest, Ljubljana, Prague",
     {"Europe/Budapest", "Europe/Tirane", "Europe/Prague", "Europe/Budapest", "Europe/Podgorica", "Europe/Belgrade", "Europe/Ljubljana", "Europe/Bratislava"}},
    {"Central European Standard Time",
     "Sarajevo, Skopje, Warsaw, Zagreb",
     {"Europe/Warsaw", "Europe/Sarajevo", "Europe/Zagreb", "Europe/Skopje", "Europe/Warsaw"}},
    {"Central Pacific Standard Time",
     "Magadan, Solomon Islands, New Caledonia",
     {"Pacific/Guadalcanal", "Antarctica/Macquarie", "Pacific/Ponape Pacific/Kosrae", "Pacific/Noumea", "Pacific/Guadalcanal", "Pacific/Efate", "Etc/GMT-11"}},
    {"Central Standard Time",
     "Central Time (US and Canada)",
     {"America/Chicago",
      "America/Winnipeg America/Rainy_River America/Rankin_Inlet America/Resolute",
      "America/Matamoros",
      "America/Chicago America/Indiana/Knox America/Indiana/Tell_City America/Menominee America/North_Dakota/Beulah America/North_Dakota/Center "
      "America/North_Dakota/New_Salem",
      "CST6CDT"}},
    {"Central Standard Time (Mexico)",
     "Guadalajara, Mexico City, Monterrey",
     {"America/Mexico_City", "America/Mexico_City America/Bahia_Banderas America/Cancun America/Merida America/Monterrey"}},
    {"China Standard Time",
     "Beijing, Chongqing, Hong Kong SAR, Urumqi",
     {"Asia/Shanghai", "Asia/Shanghai Asia/Chongqing Asia/Harbin Asia/Kashgar Asia/Urumqi", "Asia/Hong_Kong", "Asia/Macau"}},
    {"Dateline Standard Time", "International Date Line West", {"Etc/GMT+12", "Etc/GMT+12"}},
    {"E. Africa Standard Time",
     "Nairobi",
     {"Africa/Nairobi",
      "Antarctica/Syowa",
      "Africa/Djibouti",
      "Africa/Asmera",
      "Africa/Addis_Ababa",
      "Africa/Nairobi",
      "Indian/Comoro",
      "Indian/Antananarivo",
      "Africa/Khartoum",
      "Africa/Mogadishu",
      "Africa/Juba",
      "Africa/Dar_es_Salaam",
      "Africa/Kampala",
      "Indian/Mayotte",
      "Etc/GMT-3"}},
    {"E. Australia Standard Time", "Brisbane", {"Australia/Brisbane", "Australia/Brisbane Australia/Lindeman"}},
    {"E. Europe Standard Time", "Minsk", {"Asia/Nicosia", "Asia/Nicosia"}},
    {"E. South America Standard Time", "Brasilia", {"America/Sao_Paulo", "America/Sao_Paulo"}},
    {"Eastern Standard Time",
     "Eastern Time (US and Canada)",
     {"America/New_York",
      "America/Nassau",
      "America/Toronto America/Iqaluit America/Montreal America/Nipigon America/Pangnirtung America/Thunder_Bay",
      "America/Grand_Turk",
      "America/New_York America/Detroit America/Indiana/Petersburg America/Indiana/Vincennes America/Indiana/Winamac America/Kentucky/Monticello "
      "America/Louisville",
      "EST5EDT"}},
    {"Egypt Standard Time", "Cairo", {"Africa/Cairo", "Africa/Cairo", "Asia/Gaza Asia/Hebron"}},
    {"Ekaterinburg Standard Time", "Ekaterinburg", {"Asia/Yekaterinburg", "Asia/Yekaterinburg"}},
    {"Fiji Standard Time", "Fiji Islands, Kamchatka, Marshall Islands", {"Pacific/Fiji", "Pacific/Fiji"}},
    {"FLE Standard Time",
     "Helsinki, Kiev, Riga, Sofia, Tallinn, Vilnius",
     {"Europe/Kiev",
      "Europe/Mariehamn",
      "Europe/Sofia",
      "Europe/Tallinn",
      "Europe/Helsinki",
      "Europe/Vilnius",
      "Europe/Riga",
      "Europe/Kiev Europe/Simferopol Europe/Uzhgorod Europe/Zaporozhye"}},
    {"Georgian Standard Time", "Tblisi", {"Asia/Tbilisi", "Asia/Tbilisi"}},
    {"GMT Standard Time",
     "Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London",
     {"Europe/London",
      "Atlantic/Canary",
      "Atlantic/Faeroe",
      "Europe/London",
      "Europe/Guernsey",
      "Europe/Dublin",
      "Europe/Isle_of_Man",
      "Europe/Jersey",
      "Europe/Lisbon Atlantic/Madeira"}},
    {"Greenland Standard Time", "Greenland", {"America/Godthab", "America/Godthab"}},
    {"Greenwich Standard Time",
     "Casablanca, Monrovia",
     {"Atlantic/Reykjavik",
      "Africa/Ouagadougou",
      "Africa/Abidjan",
      "Africa/El_Aaiun",
      "Africa/Accra",
      "Africa/Banjul",
      "Africa/Conakry",
      "Africa/Bissau",
      "Atlantic/Reykjavik",
      "Africa/Monrovia",
      "Africa/Bamako",
      "Africa/Nouakchott",
      "Atlantic/St_Helena",
      "Africa/Freetown",
      "Africa/Dakar",
      "Africa/Sao_Tome",
      "Africa/Lome"}},
    {"GTB Standard Time", "Athens, Bucharest, Istanbul", {"Europe/Bucharest", "Europe/Athens", "Europe/Chisinau", "Europe/Bucharest"}},
    {"Hawaiian Standard Time", "Hawaii", {"Pacific/Honolulu", "Pacific/Rarotonga", "Pacific/Tahiti", "Pacific/Johnston", "Pacific/Honolulu", "Etc/GMT+10"}},
    {"India Standard Time", "Chennai, Kolkata, Mumbai, New Delhi", {"Asia/Calcutta", "Asia/Calcutta"}},
    {"Iran Standard Time", "Tehran", {"Asia/Tehran", "Asia/Tehran"}},
    {"Israel Standard Time", "Jerusalem", {"Asia/Jerusalem", "Asia/Jerusalem"}},
    {"Korea Standard Time", "Seoul", {"Asia/Seoul", "Asia/Pyongyang", "Asia/Seoul"}},
    //    {"Mid-Atlantic Standard Time", "Mid-Atlantic", {"}},
    {"Mountain Standard Time",
     "Mountain Time (US and Canada)",
     {"America/Denver",
      "America/Edmonton America/Cambridge_Bay America/Inuvik America/Yellowknife",
      "America/Ojinaga",
      "America/Denver America/Boise America/Shiprock",
      "MST7MDT"}},
    {"Mountain Standard Time (Mexico)", "Chihuahua, La Paz, Mazatlan", {"America/Chihuahua", "America/Chihuahua America/Mazatlan"}},
    {"Myanmar Standard Time", "Yangon (Rangoon)", {"Asia/Rangoon", "Indian/Cocos", "Asia/Rangoon"}},
    {"N. Central Asia Standard Time", "Almaty, Novosibirsk", {"Asia/Novosibirsk", "Asia/Novosibirsk Asia/Novokuznetsk Asia/Omsk"}},
    {"Namibia Standard Time", "Windhoek", {"Africa/Windhoek", "Africa/Windhoek"}},
    {"Nepal Standard Time", "Kathmandu", {"Asia/Katmandu", "Asia/Katmandu"}},
    {"New Zealand Standard Time", "Auckland, Wellington", {"Pacific/Auckland", "Antarctica/South_Pole Antarctica/McMurdo", "Pacific/Auckland"}},
    {"Newfoundland Standard Time", "Newfoundland and Labrador", {"America/St_Johns", "America/St_Johns"}},
    {"North Asia East Standard Time", "Irkutsk, Ulaanbaatar", {"Asia/Irkutsk", "Asia/Irkutsk"}},
    {"North Asia Standard Time", "Krasnoyarsk", {"Asia/Krasnoyarsk", "Asia/Krasnoyarsk"}},
    {"Pacific SA Standard Time", "Santiago", {"America/Santiago", "Antarctica/Palmer", "America/Santiago"}},
    {"Pacific Standard Time",
     "Pacific Time (US and Canada); Tijuana",
     {"America/Los_Angeles", "America/Vancouver America/Dawson America/Whitehorse", "America/Tijuana", "America/Los_Angeles", "PST8PDT"}},
    {"Romance Standard Time",
     "Brussels, Copenhagen, Madrid, Paris",
     {"Europe/Paris", "Europe/Brussels", "Europe/Copenhagen", "Europe/Madrid Africa/Ceuta", "Europe/Paris"}},
    {"Russian Standard Time", "Moscow, St. Petersburg, Volgograd", {"Europe/Moscow", "Europe/Moscow Europe/Samara Europe/Volgograd"}},
    {"SA Eastern Standard Time",
     "Buenos Aires, Georgetown",
     {"America/Cayenne",
      "Antarctica/Rothera",
      "America/Fortaleza America/Araguaina America/Belem America/Maceio America/Recife America/Santarem",
      "Atlantic/Stanley",
      "America/Cayenne",
      "America/Paramaribo",
      "Etc/GMT+3"}},
    {"SA Pacific Standard Time",
     "Bogota, Lima, Quito",
     {"America/Bogota",
      "America/Coral_Harbour",
      "America/Bogota",
      "America/Guayaquil",
      "America/Port-au-Prince",
      "America/Jamaica",
      "America/Cayman",
      "America/Panama",
      "America/Lima",
      "Etc/GMT+5"}},
    {"SA Western Standard Time",
     "Caracas, La Paz",
     {"America/La_Paz",
      "America/Antigua",
      "America/Anguilla",
      "America/Aruba",
      "America/Barbados",
      "America/St_Barthelemy",
      "America/La_Paz",
      "America/Kralendijk",
      "America/Manaus America/Boa_Vista America/Eirunepe America/Porto_Velho America/Rio_Branco",
      "America/Blanc-Sablon",
      "America/Curacao",
      "America/Dominica",
      "America/Santo_Domingo",
      "America/Grenada",
      "America/Guadeloupe",
      "America/Guyana",
      "America/St_Kitts",
      "America/St_Lucia",
      "America/Marigot",
      "America/Martinique",
      "America/Montserrat",
      "America/Puerto_Rico",
      "America/Lower_Princes",
      "America/Port_of_Spain",
      "America/St_Vincent",
      "America/Tortola",
      "America/St_Thomas",
      "Etc/GMT+4"}},
    {"Samoa Standard Time", "Midway Island, Samoa", {"Pacific/Apia", "Pacific/Apia"}},
    {"SE Asia Standard Time",
     "Bangkok, Hanoi, Jakarta",
     {"Asia/Bangkok",
      "Antarctica/Davis",
      "Indian/Christmas",
      "Asia/Jakarta Asia/Pontianak",
      "Asia/Phnom_Penh",
      "Asia/Vientiane",
      "Asia/Hovd",
      "Asia/Bangkok",
      "Asia/Saigon",
      "Etc/GMT-7"}},
    {"Singapore Standard Time",
     "Kuala Lumpur, Singapore",
     {"Asia/Singapore", "Asia/Brunei", "Asia/Makassar", "Asia/Kuala_Lumpur Asia/Kuching", "Asia/Manila", "Asia/Singapore", "Etc/GMT-8"}},
    {"South Africa Standard Time",
     "Harare, Pretoria",
     {"Africa/Johannesburg",
      "Africa/Bujumbura",
      "Africa/Gaborone",
      "Africa/Lubumbashi",
      "Africa/Maseru",
      "Africa/Blantyre",
      "Africa/Maputo",
      "Africa/Kigali",
      "Africa/Mbabane",
      "Africa/Johannesburg",
      "Africa/Lusaka",
      "Africa/Harare",
      "Etc/GMT-2"}},
    {"Sri Lanka Standard Time", "Sri Jayawardenepura", {"Asia/Colombo", "Asia/Colombo"}},
    {"Taipei Standard Time", "Taipei", {"Asia/Taipei", "Asia/Taipei"}},
    {"Tasmania Standard Time", "Hobart", {"Australia/Hobart", "Australia/Hobart Australia/Currie"}},
    {"Tokyo Standard Time", "Osaka, Sapporo, Tokyo", {"Asia/Tokyo", "Asia/Jayapura", "Asia/Tokyo", "Pacific/Palau", "Asia/Dili", "Etc/GMT-9"}},
    {"Tonga Standard Time", "Nuku'alofa", {"Pacific/Tongatapu", "Pacific/Enderbury", "Pacific/Fakaofo", "Pacific/Tongatapu", "Etc/GMT-13"}},
    {"US Eastern Standard Time", "Indiana (East)", {"America/Indianapolis", "America/Indianapolis America/Indiana/Marengo America/Indiana/Vevay"}},
    {"US Mountain Standard Time", "Arizona", {"America/Phoenix", "America/Dawson_Creek America/Creston", "America/Hermosillo", "America/Phoenix", "Etc/GMT+7"}},
    {"Vladivostok Standard Time", "Vladivostok", {"Asia/Vladivostok", "Asia/Vladivostok Asia/Sakhalin"}},
    {"W. Australia Standard Time", "Perth", {"Australia/Perth", "Antarctica/Casey", "Australia/Perth"}},
    {"W. Central Africa Standard Time",
     "West Central Africa",
     {"Africa/Lagos",
      "Africa/Luanda",
      "Africa/Porto-Novo",
      "Africa/Kinshasa",
      "Africa/Bangui",
      "Africa/Brazzaville",
      "Africa/Douala",
      "Africa/Algiers",
      "Africa/Libreville",
      "Africa/Malabo",
      "Africa/Niamey",
      "Africa/Lagos",
      "Africa/Ndjamena",
      "Africa/Tunis",
      "Etc/GMT-1"}},
    {"W. Europe Standard Time",
     "Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",
     {"Europe/Berlin",
      "Europe/Andorra",
      "Europe/Vienna",
      "Europe/Zurich",
      "Europe/Berlin",
      "Europe/Gibraltar",
      "Europe/Rome",
      "Europe/Vaduz",
      "Europe/Luxembourg",
      "Africa/Tripoli",
      "Europe/Monaco",
      "Europe/Malta",
      "Europe/Amsterdam",
      "Europe/Oslo",
      "Europe/Stockholm",
      "Arctic/Longyearbyen",
      "Europe/San_Marino",
      "Europe/Vatican"}},
    {"West Asia Standard Time",
     "Islamabad, Karachi, Tashkent",
     {"Asia/Tashkent",
      "Antarctica/Mawson",
      "Asia/Oral Asia/Aqtau Asia/Aqtobe",
      "Indian/Maldives",
      "Indian/Kerguelen",
      "Asia/Dushanbe",
      "Asia/Ashgabat",
      "Asia/Tashkent Asia/Samarkand",
      "Etc/GMT-5"}},
    {"West Pacific Standard Time",
     "Guam, Port Moresby",
     {"Pacific/Port_Moresby", "Antarctica/DumontDUrville", "Pacific/Truk", "Pacific/Guam", "Pacific/Saipan", "Pacific/Port_Moresby", "Etc/GMT-10"}},
    {"Yakutsk Standard Time", "Yakuts", {"Asia/Yakutsk", "Asia/Yakutsk"}}};
static const int numWindowsTimezones = sizeof windowsTimezones / sizeof *windowsTimezones;

QString TimezoneConverter::fromHardcodedList(const QString &tz)
{
    for (int i = 0; i < numWindowsTimezones; i++) {
        const WindowsTimezone &windowsTimezone = windowsTimezones[i];
        const QByteArray specifier(windowsTimezone.timezoneSpecifier);
        const QByteArray windowsName(windowsTimezone.name);
        if ((!specifier.isEmpty() && tz.contains(QString::fromUtf8(specifier))) || (!windowsName.isEmpty() && tz.contains(QString::fromUtf8(windowsName)))) {
            // TODO find the olson timezone matching the local timezone if we have multiple to map to
            return QString::fromLatin1(windowsTimezone.olson[0]);
        }
    }
    return {};
}
