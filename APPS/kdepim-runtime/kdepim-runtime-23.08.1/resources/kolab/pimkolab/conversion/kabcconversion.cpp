/*
 * SPDX-FileCopyrightText: 2011 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kabcconversion.h"

#include "commonconversion.h"
#include "pimkolab_debug.h"
#include <QBuffer>
#include <QImageReader>
namespace Kolab
{
namespace Conversion
{
// The following was copied from kdepim/libkleo/kleo/enum.h,.cpp
enum CryptoMessageFormat {
    InlineOpenPGPFormat = 1,
    OpenPGPMIMEFormat = 2,
    SMIMEFormat = 4,
    SMIMEOpaqueFormat = 8,
    AnyOpenPGP = InlineOpenPGPFormat | OpenPGPMIMEFormat,
    AnySMIME = SMIMEOpaqueFormat | SMIMEFormat,
    AutoFormat = AnyOpenPGP | AnySMIME
};

enum EncryptionPreference {
    UnknownPreference = 0,
    NeverEncrypt = 1,
    AlwaysEncrypt = 2,
    AlwaysEncryptIfPossible = 3,
    AlwaysAskForEncryption = 4,
    AskWheneverPossible = 5,
    MaxEncryptionPreference = AskWheneverPossible
};

enum SigningPreference {
    UnknownSigningPreference = 0,
    NeverSign = 1,
    AlwaysSign = 2,
    AlwaysSignIfPossible = 3,
    AlwaysAskForSigning = 4,
    AskSigningWheneverPossible = 5,
    MaxSigningPreference = AskSigningWheneverPossible
};

static const struct {
    CryptoMessageFormat format;
    const char *displayName;
    const char *configName;
} cryptoMessageFormats[] = {
    {InlineOpenPGPFormat, ("Inline OpenPGP (deprecated)"), "inline openpgp"},
    {OpenPGPMIMEFormat, ("OpenPGP/MIME"), "openpgp/mime"},
    {SMIMEFormat, ("S/MIME"), "s/mime"},
    {SMIMEOpaqueFormat, ("S/MIME Opaque"), "s/mime opaque"},
};
static const unsigned int numCryptoMessageFormats = sizeof cryptoMessageFormats / sizeof *cryptoMessageFormats;

const char *cryptoMessageFormatToString(CryptoMessageFormat f)
{
    if (f == AutoFormat) {
        return "auto";
    }
    for (unsigned int i = 0; i < numCryptoMessageFormats; ++i) {
        if (f == cryptoMessageFormats[i].format) {
            return cryptoMessageFormats[i].configName;
        }
    }
    return nullptr;
}

QStringList cryptoMessageFormatsToStringList(unsigned int f)
{
    QStringList result;
    for (unsigned int i = 0; i < numCryptoMessageFormats; ++i) {
        if (f & cryptoMessageFormats[i].format) {
            result.push_back(QLatin1String(cryptoMessageFormats[i].configName));
        }
    }
    return result;
}

CryptoMessageFormat stringToCryptoMessageFormat(const QString &s)
{
    const QString t = s.toLower();
    for (unsigned int i = 0; i < numCryptoMessageFormats; ++i) {
        if (t == QLatin1String(cryptoMessageFormats[i].configName)) {
            return cryptoMessageFormats[i].format;
        }
    }
    return AutoFormat;
}

unsigned int stringListToCryptoMessageFormats(const QStringList &sl)
{
    unsigned int result = 0;
    const QStringList::const_iterator end(sl.end());
    for (QStringList::const_iterator it = sl.begin(); it != end; ++it) {
        result |= stringToCryptoMessageFormat(*it);
    }
    return result;
}

// For the config values used below, see also kaddressbook/editors/cryptowidget.cpp

const char *encryptionPreferenceToString(EncryptionPreference pref)
{
    switch (pref) {
    case UnknownPreference:
        return nullptr;
    case NeverEncrypt:
        return "never";
    case AlwaysEncrypt:
        return "always";
    case AlwaysEncryptIfPossible:
        return "alwaysIfPossible";
    case AlwaysAskForEncryption:
        return "askAlways";
    case AskWheneverPossible:
        return "askWhenPossible";
    }
    return nullptr; // keep the compiler happy
}

EncryptionPreference stringToEncryptionPreference(const QString &str)
{
    if (str == QLatin1String("never")) {
        return NeverEncrypt;
    } else if (str == QLatin1String("always")) {
        return AlwaysEncrypt;
    } else if (str == QLatin1String("alwaysIfPossible")) {
        return AlwaysEncryptIfPossible;
    } else if (str == QLatin1String("askAlways")) {
        return AlwaysAskForEncryption;
    } else if (str == QLatin1String("askWhenPossible")) {
        return AskWheneverPossible;
    }
    return UnknownPreference;
}

const char *signingPreferenceToString(SigningPreference pref)
{
    switch (pref) {
    case UnknownSigningPreference:
        return nullptr;
    case NeverSign:
        return "never";
    case AlwaysSign:
        return "always";
    case AlwaysSignIfPossible:
        return "alwaysIfPossible";
    case AlwaysAskForSigning:
        return "askAlways";
    case AskSigningWheneverPossible:
        return "askWhenPossible";
    }
    return nullptr; // keep the compiler happy
}

SigningPreference stringToSigningPreference(const QString &str)
{
    if (str == QLatin1String("never")) {
        return NeverSign;
    }
    if (str == QLatin1String("always")) {
        return AlwaysSign;
    }
    if (str == QLatin1String("alwaysIfPossible")) {
        return AlwaysSignIfPossible;
    }
    if (str == QLatin1String("askAlways")) {
        return AlwaysAskForSigning;
    }
    if (str == QLatin1String("askWhenPossible")) {
        return AskSigningWheneverPossible;
    }
    return UnknownSigningPreference;
}

int fromAddressType(int kabcType, bool &pref)
{
    int type = 0;
    if (kabcType & KContacts::Address::Dom) {
        qCWarning(PIMKOLAB_LOG) << "domestic address is not supported";
    }
    if (kabcType & KContacts::Address::Intl) {
        qCWarning(PIMKOLAB_LOG) << "international address is not supported";
    }
    if (kabcType & KContacts::Address::Pref) {
        pref = true;
    }
    if (kabcType & KContacts::Address::Postal) {
        qCWarning(PIMKOLAB_LOG) << "postal address is not supported";
    }
    if (kabcType & KContacts::Address::Parcel) {
        qCWarning(PIMKOLAB_LOG) << "parcel is not supported";
    }
    if (kabcType & KContacts::Address::Home) {
        type |= Kolab::Address::Home;
    }
    if (kabcType & KContacts::Address::Work) {
        type |= Kolab::Address::Work;
    }
    return type;
}

KContacts::Address::Type toAddressType(int types, bool pref)
{
    KContacts::Address::Type type = {};
    if (pref) {
        type |= KContacts::Address::Pref;
    }
    if (types & Kolab::Address::Home) {
        type |= KContacts::Address::Home;
    }
    if (types & Kolab::Address::Work) {
        type |= KContacts::Address::Work;
    }
    return type;
}

int fromPhoneType(int kabcType, bool &pref)
{
    int type = 0;
    if (kabcType & KContacts::PhoneNumber::Home) {
        type |= Kolab::Telephone::Home;
    }
    if (kabcType & KContacts::PhoneNumber::Work) {
        type |= Kolab::Telephone::Work;
    }
    if (kabcType & KContacts::PhoneNumber::Msg) {
        type |= Kolab::Telephone::Text;
    }
    if (kabcType & KContacts::PhoneNumber::Pref) {
        pref = true;
    }
    if (kabcType & KContacts::PhoneNumber::Voice) {
        type |= Kolab::Telephone::Voice;
    }
    if (kabcType & KContacts::PhoneNumber::Fax) {
        type |= Kolab::Telephone::Fax;
    }
    if (kabcType & KContacts::PhoneNumber::Cell) {
        type |= Kolab::Telephone::Cell;
    }
    if (kabcType & KContacts::PhoneNumber::Video) {
        type |= Kolab::Telephone::Video;
    }
    if (kabcType & KContacts::PhoneNumber::Bbs) {
        qCWarning(PIMKOLAB_LOG) << "mailbox number is not supported";
    }
    if (kabcType & KContacts::PhoneNumber::Modem) {
        qCWarning(PIMKOLAB_LOG) << "modem is not supported";
    }
    if (kabcType & KContacts::PhoneNumber::Car) {
        type |= Kolab::Telephone::Car;
    }
    if (kabcType & KContacts::PhoneNumber::Isdn) {
        qCWarning(PIMKOLAB_LOG) << "isdn number is not supported";
    }
    if (kabcType & KContacts::PhoneNumber::Pcs) {
        type |= Kolab::Telephone::Text;
    }
    if (kabcType & KContacts::PhoneNumber::Pager) {
        type |= Kolab::Telephone::Pager;
    }
    return type;
}

KContacts::PhoneNumber::Type toPhoneType(int types, bool pref)
{
    KContacts::PhoneNumber::Type type = {};
    if (types & Kolab::Telephone::Home) {
        type |= KContacts::PhoneNumber::Home;
    }
    if (types & Kolab::Telephone::Work) {
        type |= KContacts::PhoneNumber::Work;
    }
    if (types & Kolab::Telephone::Text) {
        type |= KContacts::PhoneNumber::Msg;
    }
    if (pref) {
        type |= KContacts::PhoneNumber::Pref;
    }
    if (types & Kolab::Telephone::Voice) {
        type |= KContacts::PhoneNumber::Voice;
    }
    if (types & Kolab::Telephone::Fax) {
        type |= KContacts::PhoneNumber::Fax;
    }
    if (types & Kolab::Telephone::Cell) {
        type |= KContacts::PhoneNumber::Cell;
    }
    if (types & Kolab::Telephone::Video) {
        type |= KContacts::PhoneNumber::Video;
    }
    if (types & Kolab::Telephone::Car) {
        type |= KContacts::PhoneNumber::Car;
    }
    if (types & Kolab::Telephone::Text) {
        type |= KContacts::PhoneNumber::Pcs;
    }
    if (types & Kolab::Telephone::Pager) {
        type |= KContacts::PhoneNumber::Pager;
    }
    return type;
}

std::string fromPicture(const KContacts::Picture &pic, std::string &mimetype)
{
    QByteArray input;
    QBuffer buffer(&input);
    buffer.open(QIODevice::WriteOnly);
    QImage img;

    if (pic.isIntern()) {
        if (!pic.data().isNull()) {
            img = pic.data();
        }
    } else if (!pic.url().isEmpty()) {
        qCWarning(PIMKOLAB_LOG) << "external pictures are currently not supported";
        // FIXME add kio support to libcalendaring or use libcurl
        //         if ( KIO::NetAccess::download( pic.url(), tmpFile, 0 /*no widget known*/ ) ) {
        //             img.load( tmpFile );
        //             KIO::NetAccess::removeTempFile( tmpFile );
        //         }
    }
    if (img.isNull()) {
        qCCritical(PIMKOLAB_LOG) << "invalid picture";
        return {};
    }
    if (!img.hasAlphaChannel()) {
        if (!img.save(&buffer, "JPEG")) {
            qCCritical(PIMKOLAB_LOG) << "error on jpeg save";
            return {};
        }
        mimetype = "image/jpeg";
    } else {
        if (!img.save(&buffer, "PNG")) {
            qCCritical(PIMKOLAB_LOG) << "error on png save";
            return {};
        }
        mimetype = "image/png";
    }
    return std::string(input.data(), input.size());
}

KContacts::Picture toPicture(const std::string &data, const std::string &mimetype)
{
    QImage img;
    bool ret = false;
    QByteArray type(mimetype.data(), mimetype.size());
    type = type.split('/').last(); // extract "jpeg" from "image/jpeg"
    if (QImageReader::supportedImageFormats().contains(type)) {
        ret = img.loadFromData(QByteArray::fromRawData(data.data(), data.size()), type.constData());
    } else {
        ret = img.loadFromData(QByteArray::fromRawData(data.data(), data.size()));
    }
    if (!ret) {
        qCWarning(PIMKOLAB_LOG) << "failed to load picture";
        return {};
    }

    KContacts::Picture logo(img);
    if (logo.isEmpty()) {
        qCWarning(PIMKOLAB_LOG) << "failed to read picture";
        return {};
    }
    return logo;
}

template<typename T>
void setCustom(const std::string &value, const std::string &id, T &object)
{
    std::vector<Kolab::CustomProperty> properties = object.customProperties();
    properties.emplace_back(id, value);
    object.setCustomProperties(properties);
}

template<typename T>
std::string getCustom(const std::string &id, T &object)
{
    const std::vector<Kolab::CustomProperty> &properties = object.customProperties();
    for (const Kolab::CustomProperty &prop : properties) {
        if (prop.identifier == id) {
            return prop.value;
        }
    }
    return {};
}

static QString emailTypesToStringList(int emailTypes)
{
    QStringList types;
    if (emailTypes & Kolab::Email::Home) {
        types << QStringLiteral("home");
    }
    if (emailTypes & Kolab::Email::Work) {
        types << QStringLiteral("work");
    }
    return types.join(QLatin1Char(','));
}

static int emailTypesFromStringlist(const QString &types)
{
    int emailTypes = Kolab::Email::NoType;
    if (types.contains(QLatin1String("home"))) {
        emailTypes |= Kolab::Email::Home;
    }
    if (types.contains(QLatin1String("work"))) {
        emailTypes |= Kolab::Email::Work;
    }
    return emailTypes;
}

KContacts::Addressee toKABC(const Kolab::Contact &contact)
{
    KContacts::Addressee addressee;
    addressee.setUid(fromStdString(contact.uid()));
    addressee.setCategories(toStringList(contact.categories()));
    // addressee.setName(fromStdString(contact.name()));//This one is only for compatibility (and results in a non-existing name property)
    addressee.setFormattedName(fromStdString(contact.name())); // This on corresponds to fn

    const Kolab::NameComponents &nc = contact.nameComponents();
    if (!nc.surnames().empty()) {
        addressee.setFamilyName(fromStdString(nc.surnames().front()));
    }
    if (!nc.given().empty()) {
        addressee.setGivenName(fromStdString(nc.given().front()));
    }
    if (!nc.additional().empty()) {
        addressee.setAdditionalName(fromStdString(nc.additional().front()));
    }
    if (!nc.prefixes().empty()) {
        addressee.setPrefix(fromStdString(nc.prefixes().front()));
    }
    if (!nc.suffixes().empty()) {
        addressee.setSuffix(fromStdString(nc.suffixes().front()));
    }

    addressee.setNote(fromStdString(contact.note()));

    addressee.setSecrecy(KContacts::Secrecy::Public); // We don't have any privacy setting in xCard

    QString preferredEmail;

    if (!contact.emailAddresses().empty()) {
        QStringList emails;
        const auto contactEmailAddresses{contact.emailAddresses()};
        emails.reserve(contactEmailAddresses.size());
        for (const Kolab::Email &email : contactEmailAddresses) {
            emails << fromStdString(email.address());
            const QString types = emailTypesToStringList(email.types());
            if (!types.isEmpty()) {
                addressee.insertCustom(QStringLiteral("KOLAB"), QStringLiteral("EmailTypes%1").arg(fromStdString(email.address())), types);
            }
        }
        addressee.setEmails(emails);
        if ((contact.emailAddressPreferredIndex() >= 0) && (contact.emailAddressPreferredIndex() < static_cast<int>(contact.emailAddresses().size()))) {
            preferredEmail = fromStdString(contact.emailAddresses().at(contact.emailAddressPreferredIndex()).address());
        } else {
            preferredEmail = fromStdString(contact.emailAddresses().at(0).address());
        }
        KContacts::Email email(preferredEmail);
        email.setPreferred(true);
        addressee.addEmail(email);
    }

    if (!contact.freeBusyUrl().empty()) {
        if (preferredEmail.isEmpty()) {
            qCCritical(PIMKOLAB_LOG) << "f/b url is set but no email address available, skipping";
        } else {
            addressee.insertCustom(QStringLiteral("KOLAB"), QStringLiteral("FreebusyUrl"), fromStdString(contact.freeBusyUrl()));
        }
    }

    if (!contact.nickNames().empty()) {
        addressee.setNickName(fromStdString(contact.nickNames().at(0))); // TODO support multiple
    }

    if (contact.bDay().isValid()) {
        addressee.setBirthday(toDate(contact.bDay()));
    }
    if (!contact.titles().empty()) {
        addressee.setTitle(fromStdString(contact.titles().at(0))); // TODO support multiple
    }
    if (!contact.urls().empty()) {
        KContacts::ResourceLocatorUrl url;
        url.setUrl(QUrl(fromStdString(contact.urls().at(0).url()))); // TODO support multiple
        addressee.setUrl(url);
        const auto urls{contact.urls()};
        for (const Kolab::Url &u : urls) {
            if (u.type() == Kolab::Url::Blog) {
                addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"), fromStdString(u.url()));
            }
        }
    }

    if (!contact.affiliations().empty()) {
        // Storing only a const reference leads to segfaults. No idea why.
        const Kolab::Affiliation aff = contact.affiliations().at(0); // TODO support multiple
        if (!aff.organisation().empty()) {
            addressee.setOrganization(fromStdString(aff.organisation()));
        }
        if (!aff.organisationalUnits().empty()) {
            addressee.setDepartment(fromStdString(aff.organisationalUnits().at(0))); // TODO support multiple
        }
        if (!aff.roles().empty()) {
            addressee.setRole(fromStdString(aff.roles().at(0))); // TODO support multiple
        }
        if (!aff.logo().empty()) {
            addressee.setLogo(toPicture(aff.logo(), aff.logoMimetype()));
        }
        const auto affRelateds{aff.relateds()};
        for (const Kolab::Related &related : affRelateds) {
            if (related.type() != Kolab::Related::Text) {
                qCCritical(PIMKOLAB_LOG) << "invalid relation type";
                continue;
            }
            if (related.relationTypes() & Kolab::Related::Assistant) {
                addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), fromStdString(related.text()));
            }
            if (related.relationTypes() & Kolab::Related::Manager) {
                addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), fromStdString(related.text()));
            }
        }
        const auto addresses{aff.addresses()};
        for (const Kolab::Address &address : addresses) {
            addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), fromStdString(address.label())); // TODO support proper addresses
        }
    }
    const std::string &prof = getCustom("X-Profession", contact);
    if (!prof.empty()) {
        addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), fromStdString(prof));
    }

    const std::string &adrBook = getCustom("X-AddressBook", contact);
    if (!adrBook.empty()) {
        addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AddressBook"), fromStdString(prof));
    }

    if (!contact.photo().empty()) {
        addressee.setPhoto(toPicture(contact.photo(), contact.photoMimetype()));
    }

    if (!contact.telephones().empty()) {
        int index = 0;
        const auto contactTelephones{contact.telephones()};
        for (const Kolab::Telephone &tel : contactTelephones) {
            bool pref = false;
            if (index == contact.telephonesPreferredIndex()) {
                pref = true;
            }
            KContacts::PhoneNumber number(fromStdString(tel.number()), toPhoneType(tel.types(), pref));
            index++;
            addressee.insertPhoneNumber(number);
        }
    }

    if (!contact.addresses().empty()) {
        int index = 0;
        const auto contactAddresses{contact.addresses()};
        for (const Kolab::Address &a : contactAddresses) {
            bool pref = false;
            if (index == contact.addressPreferredIndex()) {
                pref = true;
            }
            KContacts::Address adr(toAddressType(a.types(), pref));
            adr.setLabel(fromStdString(a.label()));
            adr.setStreet(fromStdString(a.street()));
            adr.setLocality(fromStdString(a.locality()));
            adr.setRegion(fromStdString(a.region()));
            adr.setPostalCode(fromStdString(a.code()));
            adr.setCountry(fromStdString(a.country()));

            index++;
            addressee.insertAddress(adr);
        }
    }

    if (contact.anniversary().isValid()) {
        addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), toDate(contact.anniversary()).toString(Qt::ISODate));
    }

    if (!contact.imAddresses().empty()) {
        addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress"), fromStdString(contact.imAddresses()[0])); // TODO support multiple
    }

    if (!contact.relateds().empty()) {
        const auto relateds{contact.relateds()};
        for (const Kolab::Related &rel : relateds) {
            if (rel.type() != Kolab::Related::Text) {
                qCCritical(PIMKOLAB_LOG) << "relation type not supported";
                continue;
            }
            if (rel.relationTypes() & Kolab::Related::Spouse) {
                addressee.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), fromStdString(rel.text())); // TODO support multiple
            } else {
                qCWarning(PIMKOLAB_LOG) << "relation not supported";
                continue;
            }
        }
    }

    return addressee;
}

Kolab::Contact fromKABC(const KContacts::Addressee &addressee)
{
    int prefNum = -1;
    int prefCounter = -1;
    Kolab::Contact c;
    c.setUid(toStdString(addressee.uid()));
    c.setCategories(fromStringList(addressee.categories()));
    c.setName(toStdString(addressee.formattedName()));
    Kolab::NameComponents nc;
    nc.setSurnames(std::vector<std::string>() << toStdString(addressee.familyName()));
    nc.setGiven(std::vector<std::string>() << toStdString(addressee.givenName()));
    nc.setAdditional(std::vector<std::string>() << toStdString(addressee.additionalName()));
    nc.setPrefixes(std::vector<std::string>() << toStdString(addressee.prefix()));
    nc.setSuffixes(std::vector<std::string>() << toStdString(addressee.suffix()));
    c.setNameComponents(nc);

    c.setNote(toStdString(addressee.note()));
    c.setFreeBusyUrl(toStdString(addressee.custom(QStringLiteral("KOLAB"), QStringLiteral("FreebusyUrl"))));

    if (!addressee.title().isEmpty()) {
        c.setTitles(std::vector<std::string>() << toStdString(addressee.title()));
    }

    Kolab::Affiliation businessAff;
    businessAff.setOrganisation(toStdString(addressee.organization()));
    if (!addressee.department().isEmpty()) {
        qCDebug(PIMKOLAB_LOG) << addressee.department() << addressee.department().toLatin1() << addressee.department().toUtf8();
        businessAff.setOrganisationalUnits(std::vector<std::string>() << toStdString(addressee.department()));
    }

    if (!addressee.logo().isEmpty()) {
        std::string logoMimetype;
        const std::string &logo = fromPicture(addressee.logo(), logoMimetype);
        businessAff.setLogo(logo, logoMimetype);
    }
    if (!addressee.role().isEmpty()) {
        businessAff.setRoles(std::vector<std::string>() << toStdString(addressee.role()));
    }
    const QString &office = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"));
    if (!office.isEmpty()) {
        Kolab::Address a;
        a.setTypes(Kolab::Address::Work);
        a.setLabel(toStdString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"))));
        businessAff.setAddresses(std::vector<Kolab::Address>() << a);
    }

    std::vector<Kolab::Related> relateds;
    const QString &manager = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"));
    if (!manager.isEmpty()) {
        relateds.emplace_back(Kolab::Related::Text, toStdString(manager), Kolab::Related::Manager);
    }
    const QString &assistant = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"));
    if (!assistant.isEmpty()) {
        relateds.emplace_back(Kolab::Related::Text, toStdString(assistant), Kolab::Related::Assistant);
    }
    if (!relateds.empty()) {
        businessAff.setRelateds(relateds);
    }
    if (!(businessAff == Kolab::Affiliation())) {
        c.setAffiliations(std::vector<Kolab::Affiliation>() << businessAff);
    }

    std::vector<Kolab::Url> urls;
    const QUrl url{addressee.url().url()};
    if (!url.isEmpty()) {
        urls.emplace_back(toStdString(url.url()));
    }
    const QString &blogUrl = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"));
    if (!blogUrl.isEmpty()) {
        urls.emplace_back(toStdString(blogUrl), Kolab::Url::Blog);
    }
    c.setUrls(urls);

    std::vector<Kolab::Address> addresses;
    prefNum = -1;
    prefCounter = -1;
    const auto contactAddresses{addressee.addresses()};
    for (const KContacts::Address &a : contactAddresses) {
        Kolab::Address adr;
        bool pref = false;
        adr.setTypes(fromAddressType(a.type(), pref));
        prefCounter++;
        if (pref) {
            prefNum = prefCounter;
        }
        adr.setLabel(toStdString(a.label()));
        adr.setStreet(toStdString(a.street()));
        adr.setLocality(toStdString(a.locality()));
        adr.setRegion(toStdString(a.region()));
        adr.setCode(toStdString(a.postalCode()));
        adr.setCountry(toStdString(a.country()));
        addresses.push_back(adr);
    }
    c.setAddresses(addresses, prefNum);

    if (!addressee.nickName().isEmpty()) {
        c.setNickNames(std::vector<std::string>() << toStdString(addressee.nickName()));
    }

    const QString &spouse = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"));
    if (!spouse.isEmpty()) {
        c.setRelateds(std::vector<Kolab::Related>() << Kolab::Related(Kolab::Related::Text, toStdString(spouse), Kolab::Related::Spouse));
    }
    c.setBDay(fromDate(addressee.birthday(), true));
    c.setAnniversary(
        fromDate(QDateTime(QDate::fromString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary")), Qt::ISODate), {}), true));
    if (!addressee.photo().isEmpty()) {
        std::string mimetype;
        const std::string &photo = fromPicture(addressee.photo(), mimetype);
        c.setPhoto(photo, mimetype);
    }
    // TODO
    // c.setGender(addressee.gender());
    std::vector<std::string> languages;
    const auto langs{addressee.langs()};
    for (const KContacts::Lang &n : langs) {
        languages.push_back(toStdString(n.language()));
    }
    c.setLanguages(languages);

    std::vector<Kolab::Telephone> phones;
    prefNum = -1;
    prefCounter = -1;
    const auto phoneNumbers{addressee.phoneNumbers()};
    for (const KContacts::PhoneNumber &n : phoneNumbers) {
        Kolab::Telephone p;
        p.setNumber(toStdString(n.number()));
        bool pref = false;
        p.setTypes(fromPhoneType(n.type(), pref));
        prefCounter++;
        if (pref) {
            prefNum = prefCounter;
        }
        phones.push_back(p);
    }
    c.setTelephones(phones, prefNum);

    const QString &imAddress = addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress"));
    if (!imAddress.isEmpty()) {
        c.setIMaddresses(std::vector<std::string>() << toStdString(imAddress), 0);
    }

    int prefEmail = -1;
    int count = 0;
    std::vector<Kolab::Email> emails;
    const auto addressesEmails{addressee.emails()};
    emails.reserve(addressesEmails.count());
    for (const QString &e : addressesEmails) {
        if ((prefEmail == -1) && (e == addressee.preferredEmail())) {
            prefEmail = count;
        }
        count++;
        emails.emplace_back(toStdString(e), emailTypesFromStringlist(addressee.custom(QStringLiteral("KOLAB"), QStringLiteral("EmailTypes%1").arg(e))));
    }
    c.setEmailAddresses(emails, prefEmail);

    if (addressee.geo().isValid()) {
        c.setGPSpos(std::vector<Kolab::Geo>() << Kolab::Geo(addressee.geo().latitude(), addressee.geo().longitude()));
    }

    Kolab::Crypto crypto;

    const QStringList protocolPrefs =
        addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOPROTOPREF")).split(QLatin1Char(','), Qt::SkipEmptyParts);
    const uint cryptoFormats = stringListToCryptoMessageFormats(protocolPrefs);
    int formats = 0;
    if (cryptoFormats & InlineOpenPGPFormat) {
        formats |= Kolab::Crypto::PGPinline;
    }
    if (cryptoFormats & OpenPGPMIMEFormat) {
        formats |= Kolab::Crypto::PGPmime;
    }
    if (cryptoFormats & SMIMEFormat) {
        formats |= Kolab::Crypto::SMIME;
    }
    if (cryptoFormats & SMIMEOpaqueFormat) {
        formats |= Kolab::Crypto::SMIMEopaque;
    }
    crypto.setAllowed(formats);

    Kolab::Crypto::CryptoPref signPref = Kolab::Crypto::Ask;
    switch (stringToSigningPreference(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOSIGNPREF")))) {
    case NeverSign:
        signPref = Kolab::Crypto::Never;
        break;
    case AlwaysSign:
        signPref = Kolab::Crypto::Always;
        break;
    case AlwaysSignIfPossible:
        signPref = Kolab::Crypto::IfPossible;
        break;
    case AlwaysAskForSigning:
    case AskSigningWheneverPossible:
        signPref = Kolab::Crypto::Ask;
        break;
    default:
        signPref = Kolab::Crypto::Ask;
    }
    crypto.setSignPref(signPref);

    Kolab::Crypto::CryptoPref encryptPref = Kolab::Crypto::Ask;
    switch (stringToSigningPreference(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOENCRYPTPREF")))) {
    case NeverEncrypt:
        encryptPref = Kolab::Crypto::Never;
        break;
    case AlwaysEncrypt:
        encryptPref = Kolab::Crypto::Always;
        break;
    case AlwaysEncryptIfPossible:
        encryptPref = Kolab::Crypto::IfPossible;
        break;
    case AlwaysAskForEncryption:
    case AskWheneverPossible:
        encryptPref = Kolab::Crypto::Ask;
        break;
    default:
        encryptPref = Kolab::Crypto::Ask;
    }
    crypto.setEncryptPref(encryptPref);

    c.setCrypto(crypto);

    // FIXME the keys are most certainly wrong, look at cryptopageplugin.cpp
    std::vector<Kolab::Key> keys;
    const std::string &pgpkey = toStdString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("OPENPGPFP")));
    if (!pgpkey.empty()) {
        keys.emplace_back(pgpkey, Kolab::Key::PGP);
    }
    const std::string &smimekey = toStdString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("SMIMEFP")));
    if (!smimekey.empty()) {
        keys.emplace_back(smimekey, Kolab::Key::PKCS7_MIME);
    }
    c.setKeys(keys);

    if (!addressee.sound().isEmpty()) {
        qCWarning(PIMKOLAB_LOG) << "sound is not supported";
    }

    const std::string &profession = toStdString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession")));
    if (!profession.empty()) {
        setCustom(profession, "X-Profession", c);
    }

    const std::string &adrBook = toStdString(addressee.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AddressBook")));
    if (!adrBook.empty()) {
        setCustom(adrBook, "X-AddressBook", c);
    }

    // TODO preserve all custom properties (also such which are unknown to us)

    return c;
}

DistList fromKABC(const KContacts::ContactGroup &cg)
{
    DistList dl;
    dl.setName(toStdString(cg.name()));
    dl.setUid(toStdString(cg.id()));

    std::vector<Kolab::ContactReference> members;
    for (int i = 0; i < cg.dataCount(); i++) {
        const KContacts::ContactGroup::Data &data = cg.data(i);
        members.emplace_back(Kolab::ContactReference::EmailReference, toStdString(data.email()), toStdString(data.name()));
    }
    for (int i = 0; i < cg.contactReferenceCount(); i++) {
        const KContacts::ContactGroup::ContactReference &ref = cg.contactReference(i);
        members.emplace_back(Kolab::ContactReference::UidReference, toStdString(ref.uid()));
    }

    if (cg.contactGroupReferenceCount() > 0) {
        qCWarning(PIMKOLAB_LOG) << "Tried to save contact group references, which should have been resolved already";
    }

    dl.setMembers(members);

    return dl;
}

KContacts::ContactGroup toKABC(const DistList &dl)
{
    KContacts::ContactGroup cg(fromStdString(dl.name()));
    cg.setId(fromStdString(dl.uid()));
    const auto members{dl.members()};
    for (const Kolab::ContactReference &m : members) {
        switch (m.type()) {
        case Kolab::ContactReference::EmailReference:
            cg.append(KContacts::ContactGroup::Data(fromStdString(m.name()), fromStdString(m.email())));
            break;
        case Kolab::ContactReference::UidReference:
            cg.append(KContacts::ContactGroup::ContactReference(fromStdString(m.uid())));
            break;
        default:
            qCCritical(PIMKOLAB_LOG) << "invalid contact reference";
        }
    }

    return cg;
}
} // Namespace
} // Namespace
