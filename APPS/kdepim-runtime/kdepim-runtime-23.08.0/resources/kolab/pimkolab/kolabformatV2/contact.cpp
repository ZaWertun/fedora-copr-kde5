/*
    This file is part of libkabc and/or kaddressbook.
    SPDX-FileCopyrightText: 2004 Klarälvdalens Datakonsult AB <info@klaralvdalens-datakonsult.se>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contact.h"
#include "pimkolab_debug.h"
#include <KContacts/Addressee>
#include <QFile>
#include <cfloat>

using namespace KolabV2;

namespace
{
inline QString defaultPictureAttachmentName()
{
    return QStringLiteral("kolab-picture.png");
}

inline QString defaultLogoAttachmentName()
{
    return QStringLiteral("kolab-logo.png");
}

inline QString defaultSoundAttachmentName()
{
    return QStringLiteral("sound");
}

inline QString unhandledTagAppName()
{
    return QStringLiteral("KOLABUNHANDLED");
} // no hyphens in appnames!
}

// saving (addressee->xml)
Contact::Contact(const KContacts::Addressee *addr)
    : mHasGeo(false)
{
    setFields(addr);
}

// loading (xml->addressee)
Contact::Contact(const QString &xml)
{
    load(xml);
}

Contact::~Contact() = default;

void Contact::setGivenName(const QString &name)
{
    mGivenName = name;
}

QString Contact::givenName() const
{
    return mGivenName;
}

void Contact::setMiddleNames(const QString &names)
{
    mMiddleNames = names;
}

QString Contact::middleNames() const
{
    return mMiddleNames;
}

void Contact::setLastName(const QString &name)
{
    mLastName = name;
}

QString Contact::lastName() const
{
    return mLastName;
}

void Contact::setFullName(const QString &name)
{
    mFullName = name;
}

QString Contact::fullName() const
{
    return mFullName;
}

void Contact::setInitials(const QString &initials)
{
    mInitials = initials;
}

QString Contact::initials() const
{
    return mInitials;
}

void Contact::setPrefix(const QString &prefix)
{
    mPrefix = prefix;
}

QString Contact::prefix() const
{
    return mPrefix;
}

void Contact::setSuffix(const QString &suffix)
{
    mSuffix = suffix;
}

QString Contact::suffix() const
{
    return mSuffix;
}

void Contact::setRole(const QString &role)
{
    mRole = role;
}

QString Contact::role() const
{
    return mRole;
}

void Contact::setFreeBusyUrl(const QString &fbUrl)
{
    mFreeBusyUrl = fbUrl;
}

QString Contact::freeBusyUrl() const
{
    return mFreeBusyUrl;
}

void Contact::setOrganization(const QString &organization)
{
    mOrganization = organization;
}

QString Contact::organization() const
{
    return mOrganization;
}

void Contact::setWebPage(const QString &url)
{
    mWebPage = url;
}

QString Contact::webPage() const
{
    return mWebPage;
}

void Contact::setIMAddress(const QString &imAddress)
{
    mIMAddress = imAddress;
}

QString Contact::imAddress() const
{
    return mIMAddress;
}

void Contact::setDepartment(const QString &department)
{
    mDepartment = department;
}

QString Contact::department() const
{
    return mDepartment;
}

void Contact::setOfficeLocation(const QString &location)
{
    mOfficeLocation = location;
}

QString Contact::officeLocation() const
{
    return mOfficeLocation;
}

void Contact::setProfession(const QString &profession)
{
    mProfession = profession;
}

QString Contact::profession() const
{
    return mProfession;
}

void Contact::setTitle(const QString &title)
{
    mTitle = title;
}

QString Contact::title() const
{
    return mTitle;
}

void Contact::setManagerName(const QString &name)
{
    mManagerName = name;
}

QString Contact::managerName() const
{
    return mManagerName;
}

void Contact::setAssistant(const QString &name)
{
    mAssistant = name;
}

QString Contact::assistant() const
{
    return mAssistant;
}

void Contact::setNickName(const QString &name)
{
    mNickName = name;
}

QString Contact::nickName() const
{
    return mNickName;
}

void Contact::setSpouseName(const QString &name)
{
    mSpouseName = name;
}

QString Contact::spouseName() const
{
    return mSpouseName;
}

void Contact::setBirthday(QDate date)
{
    mBirthday = date;
}

QDate Contact::birthday() const
{
    return mBirthday;
}

void Contact::setAnniversary(QDate date)
{
    mAnniversary = date;
}

QDate Contact::anniversary() const
{
    return mAnniversary;
}

void Contact::setChildren(const QString &children)
{
    mChildren = children;
}

QString Contact::children() const
{
    return mChildren;
}

void Contact::setGender(const QString &gender)
{
    mGender = gender;
}

QString Contact::gender() const
{
    return mGender;
}

void Contact::setLanguage(const QString &language)
{
    mLanguage = language;
}

QString Contact::language() const
{
    return mLanguage;
}

void Contact::addPhoneNumber(const PhoneNumber &number)
{
    mPhoneNumbers.append(number);
}

QList<Contact::PhoneNumber> &Contact::phoneNumbers()
{
    return mPhoneNumbers;
}

const QList<Contact::PhoneNumber> &Contact::phoneNumbers() const
{
    return mPhoneNumbers;
}

void Contact::addEmail(const Email &email)
{
    mEmails.append(email);
}

QList<Contact::Email> &Contact::emails()
{
    return mEmails;
}

QString Contact::fullEmail() const
{
    return mFullEmail;
}

const QList<Contact::Email> &Contact::emails() const
{
    return mEmails;
}

void Contact::addAddress(const Contact::Address &address)
{
    mAddresses.append(address);
}

QList<Contact::Address> &Contact::addresses()
{
    return mAddresses;
}

const QList<Contact::Address> &Contact::addresses() const
{
    return mAddresses;
}

void Contact::setPreferredAddress(const QString &address)
{
    mPreferredAddress = address;
}

QString Contact::preferredAddress() const
{
    return mPreferredAddress;
}

bool Contact::loadNameAttribute(QDomElement &element)
{
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            QString tagName = e.tagName();

            if (tagName == QLatin1String("given-name")) {
                setGivenName(e.text());
            } else if (tagName == QLatin1String("middle-names")) {
                setMiddleNames(e.text());
            } else if (tagName == QLatin1String("last-name")) {
                setLastName(e.text());
            } else if (tagName == QLatin1String("full-name")) {
                setFullName(e.text());
            } else if (tagName == QLatin1String("initials")) {
                setInitials(e.text());
            } else if (tagName == QLatin1String("prefix")) {
                setPrefix(e.text());
            } else if (tagName == QLatin1String("suffix")) {
                setSuffix(e.text());
            } else {
                // TODO: Unhandled tag - save for later storage
                qCDebug(PIMKOLAB_LOG) << "Warning: Unhandled tag" << e.tagName();
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

void Contact::saveNameAttribute(QDomElement &element) const
{
    QDomElement e = element.ownerDocument().createElement(QStringLiteral("name"));
    element.appendChild(e);

    writeString(e, QStringLiteral("given-name"), givenName());
    writeString(e, QStringLiteral("middle-names"), middleNames());
    writeString(e, QStringLiteral("last-name"), lastName());
    writeString(e, QStringLiteral("full-name"), fullName());
    writeString(e, QStringLiteral("initials"), initials());
    writeString(e, QStringLiteral("prefix"), prefix());
    writeString(e, QStringLiteral("suffix"), suffix());
}

bool Contact::loadPhoneAttribute(QDomElement &element)
{
    PhoneNumber number;
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            QString tagName = e.tagName();

            if (tagName == QLatin1String("type")) {
                number.type = e.text();
            } else if (tagName == QLatin1String("number")) {
                number.number = e.text();
            } else {
                // TODO: Unhandled tag - save for later storage
                qCDebug(PIMKOLAB_LOG) << "Warning: Unhandled tag" << e.tagName();
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    addPhoneNumber(number);
    return true;
}

void Contact::savePhoneAttributes(QDomElement &element) const
{
    QList<PhoneNumber>::ConstIterator it = mPhoneNumbers.constBegin();
    const QList<PhoneNumber>::ConstIterator end = mPhoneNumbers.constEnd();
    for (; it != end; ++it) {
        QDomElement e = element.ownerDocument().createElement(QStringLiteral("phone"));
        element.appendChild(e);
        const PhoneNumber &p = *it;
        writeString(e, QStringLiteral("type"), p.type);
        writeString(e, QStringLiteral("number"), p.number);
    }
}

void Contact::saveEmailAttributes(QDomElement &element) const
{
    QList<Email>::ConstIterator it = mEmails.constBegin();
    QList<Email>::ConstIterator end = mEmails.constEnd();
    for (; it != end; ++it) {
        saveEmailAttribute(element, *it);
    }
}

void Contact::loadCustomAttributes(QDomElement &element)
{
    Custom custom;
    custom.app = element.attribute(QStringLiteral("app"));
    custom.name = element.attribute(QStringLiteral("name"));
    custom.value = element.attribute(QStringLiteral("value"));
    mCustomList.append(custom);
}

void Contact::saveCustomAttributes(QDomElement &element) const
{
    QList<Custom>::ConstIterator it = mCustomList.constBegin();
    const QList<Custom>::ConstIterator total = mCustomList.constEnd();
    for (; it != total; ++it) {
        Q_ASSERT(!(*it).name.isEmpty());
        if ((*it).app == unhandledTagAppName()) {
            writeString(element, (*it).name, (*it).value);
        } else {
            // Let's use attributes so that other tag-preserving-code doesn't need sub-elements
            QDomElement e = element.ownerDocument().createElement(QStringLiteral("x-custom"));
            element.appendChild(e);
            e.setAttribute(QStringLiteral("app"), (*it).app);
            e.setAttribute(QStringLiteral("name"), (*it).name);
            e.setAttribute(QStringLiteral("value"), (*it).value);
        }
    }
}

bool Contact::loadAddressAttribute(QDomElement &element)
{
    Address address;

    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            QString tagName = e.tagName();

            if (tagName == QLatin1String("type")) {
                address.type = e.text();
            } else if (tagName == QLatin1String("x-kde-type")) {
                address.kdeAddressType = e.text().toInt();
            } else if (tagName == QLatin1String("street")) {
                address.street = e.text();
            } else if (tagName == QLatin1String("pobox")) {
                address.pobox = e.text();
            } else if (tagName == QLatin1String("locality")) {
                address.locality = e.text();
            } else if (tagName == QLatin1String("region")) {
                address.region = e.text();
            } else if (tagName == QLatin1String("postal-code")) {
                address.postalCode = e.text();
            } else if (tagName == QLatin1String("country")) {
                address.country = e.text();
            } else {
                // TODO: Unhandled tag - save for later storage
                qCDebug(PIMKOLAB_LOG) << "Warning: Unhandled tag" << e.tagName();
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    addAddress(address);
    return true;
}

void Contact::saveAddressAttributes(QDomElement &element) const
{
    QList<Address>::ConstIterator it = mAddresses.constBegin();
    const QList<Address>::ConstIterator end = mAddresses.constEnd();
    for (; it != end; ++it) {
        QDomElement e = element.ownerDocument().createElement(QStringLiteral("address"));
        element.appendChild(e);
        const Address &a = *it;
        writeString(e, QStringLiteral("type"), a.type);
        writeString(e, QStringLiteral("x-kde-type"), QString::number(a.kdeAddressType));
        if (!a.street.isEmpty()) {
            writeString(e, QStringLiteral("street"), a.street);
        }
        if (!a.pobox.isEmpty()) {
            writeString(e, QStringLiteral("pobox"), a.pobox);
        }
        if (!a.locality.isEmpty()) {
            writeString(e, QStringLiteral("locality"), a.locality);
        }
        if (!a.region.isEmpty()) {
            writeString(e, QStringLiteral("region"), a.region);
        }
        if (!a.postalCode.isEmpty()) {
            writeString(e, QStringLiteral("postal-code"), a.postalCode);
        }
        if (!a.country.isEmpty()) {
            writeString(e, QStringLiteral("country"), a.country);
        }
    }
}

bool Contact::loadAttribute(QDomElement &element)
{
    const QString tagName = element.tagName();
    switch (tagName[0].toLatin1()) {
    case 'a':
        if (tagName == QLatin1String("address")) {
            return loadAddressAttribute(element);
        }
        if (tagName == QLatin1String("assistant")) {
            setAssistant(element.text());
            return true;
        }
        if (tagName == QLatin1String("anniversary")) {
            if (!element.text().isEmpty()) {
                setAnniversary(stringToDate(element.text()));
            }
            return true;
        }
        break;
    case 'b':
        if (tagName == QLatin1String("birthday")) {
            if (!element.text().isEmpty()) {
                setBirthday(stringToDate(element.text()));
            }
            return true;
        }
        break;
    case 'c':
        if (tagName == QLatin1String("children")) {
            setChildren(element.text());
            return true;
        }
        break;
    case 'd':
        if (tagName == QLatin1String("department")) {
            setDepartment(element.text());
            return true;
        }
        break;
    case 'e':
        if (tagName == QLatin1String("email")) {
            Email email;
            if (loadEmailAttribute(element, email)) {
                addEmail(email);
                return true;
            } else {
                return false;
            }
        }
        break;
    case 'f':
        if (tagName == QLatin1String("free-busy-url")) {
            setFreeBusyUrl(element.text());
            return true;
        }
        break;
    case 'g':
        if (tagName == QLatin1String("gender")) {
            setGender(element.text());
            return true;
        }
        break;
    case 'i':
        if (tagName == QLatin1String("im-address")) {
            setIMAddress(element.text());
            return true;
        }
        break;
    case 'j':
        if (tagName == QLatin1String("job-title")) {
            // see saveAttributes: <job-title> is mapped to the Role field
            setTitle(element.text());
            return true;
        }
        break;
    case 'l':
        if (tagName == QLatin1String("language")) {
            setLanguage(element.text());
            return true;
        }
        if (tagName == QLatin1String("latitude")) {
            setLatitude(element.text().toFloat());
            mHasGeo = true;
            return true;
        }
        if (tagName == QLatin1String("longitude")) {
            setLongitude(element.text().toFloat());
            mHasGeo = true;
        }
        break;
    case 'm':
        if (tagName == QLatin1String("manager-name")) {
            setManagerName(element.text());
            return true;
        }
        break;
    case 'n':
        if (tagName == QLatin1String("name")) {
            return loadNameAttribute(element);
        }
        if (tagName == QLatin1String("nick-name")) {
            setNickName(element.text());
            return true;
        }
        break;
    case 'o':
        if (tagName == QLatin1String("organization")) {
            setOrganization(element.text());
            return true;
        }
        if (tagName == QLatin1String("office-location")) {
            setOfficeLocation(element.text());
            return true;
        }
        break;
    case 'p':
        if (tagName == QLatin1String("profession")) {
            setProfession(element.text());
            return true;
        }
        if (tagName == QLatin1String("picture")) {
            mPictureAttachmentName = element.text();
            return true;
        }
        if (tagName == QLatin1String("phone")) {
            return loadPhoneAttribute(element);
        }
        if (tagName == QLatin1String("preferred-address")) {
            setPreferredAddress(element.text());
            return true;
        }
        break;
    case 'r':
        if (tagName == QLatin1String("role")) {
            setRole(element.text());
            return true;
        }
        break;
    case 's':
        if (tagName == QLatin1String("spouse-name")) {
            setSpouseName(element.text());
            return true;
        }
        break;
    case 'x':
        if (tagName == QLatin1String("x-logo")) {
            mLogoAttachmentName = element.text();
            return true;
        }
        if (tagName == QLatin1String("x-sound")) {
            mSoundAttachmentName = element.text();
            return true;
        }
        if (tagName == QLatin1String("x-custom")) {
            loadCustomAttributes(element);
            return true;
        }
        if (tagName == QLatin1String("x-title")) {
            setTitle(element.text());
            return true;
        }
        break;
    case 'w':
        if (tagName == QLatin1String("web-page")) {
            setWebPage(element.text());
            return true;
        }
        break;
    default:
        break;
    }
    return KolabBase::loadAttribute(element);
}

bool Contact::saveAttributes(QDomElement &element) const
{
    // Save the base class elements
    KolabBase::saveAttributes(element);
    saveNameAttribute(element);
    writeString(element, QStringLiteral("free-busy-url"), freeBusyUrl());
    writeString(element, QStringLiteral("organization"), organization());
    writeString(element, QStringLiteral("web-page"), webPage());
    writeString(element, QStringLiteral("im-address"), imAddress());
    writeString(element, QStringLiteral("department"), department());
    writeString(element, QStringLiteral("office-location"), officeLocation());
    writeString(element, QStringLiteral("profession"), profession());
    writeString(element, QStringLiteral("role"), role());
    writeString(element, QStringLiteral("job-title"), title());
    writeString(element, QStringLiteral("manager-name"), managerName());
    writeString(element, QStringLiteral("assistant"), assistant());
    writeString(element, QStringLiteral("nick-name"), nickName());
    writeString(element, QStringLiteral("spouse-name"), spouseName());
    writeString(element, QStringLiteral("birthday"), dateToString(birthday()));
    writeString(element, QStringLiteral("anniversary"), dateToString(anniversary()));
    if (!picture().isNull()) {
        writeString(element, QStringLiteral("picture"), mPictureAttachmentName);
    }
    if (!logo().isNull()) {
        writeString(element, QStringLiteral("x-logo"), mLogoAttachmentName);
    }
    if (!sound().isNull()) {
        writeString(element, QStringLiteral("x-sound"), mSoundAttachmentName);
    }
    writeString(element, QStringLiteral("children"), children());
    writeString(element, QStringLiteral("gender"), gender());
    writeString(element, QStringLiteral("language"), language());
    savePhoneAttributes(element);
    saveEmailAttributes(element);
    saveAddressAttributes(element);
    writeString(element, QStringLiteral("preferred-address"), preferredAddress());
    if (mHasGeo) {
        writeString(element, QStringLiteral("latitude"), QString::number(latitude(), 'g', DBL_DIG));
        writeString(element, QStringLiteral("longitude"), QString::number(longitude(), 'g', DBL_DIG));
    }
    saveCustomAttributes(element);

    return true;
}

bool Contact::loadXML(const QDomDocument &document)
{
    QDomElement top = document.documentElement();

    if (top.tagName() != QLatin1String("contact")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected contact").arg(top.tagName());
        return false;
    }

    for (QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (!loadAttribute(e)) {
                // Unhandled tag - save for later storage
                // qCDebug(PIMKOLAB_LOG) <<"Saving unhandled tag" << e.tagName();
                Custom c;
                c.app = unhandledTagAppName();
                c.name = e.tagName();
                c.value = e.text();
                mCustomList.append(c);
            }
        } else {
            qCDebug(PIMKOLAB_LOG) << "Node is not a comment or an element???";
        }
    }

    return true;
}

QString Contact::saveXML() const
{
    QDomDocument document = domTree();
    QDomElement element = document.createElement(QStringLiteral("contact"));
    element.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    saveAttributes(element);
    document.appendChild(element);
    return document.toString();
}

static QString addressTypeToString(int /*KContacts::Address::Type*/ type)
{
    if (type & KContacts::Address::Home) {
        return QStringLiteral("home");
    }
    if (type & KContacts::Address::Work) {
        return QStringLiteral("business");
    }
    return QStringLiteral("other");
}

static int addressTypeFromString(const QString &type)
{
    if (type == QLatin1String("home")) {
        return KContacts::Address::Home;
    }
    if (type == QLatin1String("business")) {
        return KContacts::Address::Work;
    }
    // well, this shows "other" in the editor, which is what we want...
    return KContacts::Address::Dom | KContacts::Address::Intl | KContacts::Address::Postal | KContacts::Address::Parcel;
}

static QStringList phoneTypeToString(KContacts::PhoneNumber::Type type)
{
    // KABC has a bitfield, i.e. the same phone number can be used for work and home
    // and fax and cellphone etc. etc.
    // So when saving we need to create as many tags as bits that were set.
    QStringList types;
    if (type & KContacts::PhoneNumber::Fax) {
        if (type & KContacts::PhoneNumber::Home) {
            types << QStringLiteral("homefax");
        } else { // assume work -- if ( type & KContacts::PhoneNumber::Work )
            types << QStringLiteral("businessfax");
        }
        type = type & ~KContacts::PhoneNumber::Home;
        type = type & ~KContacts::PhoneNumber::Work;
    }

    // To support both "home1" and "home2", map Home+Pref to home1
    if ((type & KContacts::PhoneNumber::Home) && (type & KContacts::PhoneNumber::Pref)) {
        types << QStringLiteral("home1");
        type = type & ~KContacts::PhoneNumber::Home;
        type = type & ~KContacts::PhoneNumber::Pref;
    }
    // To support both "business1" and "business2", map Work+Pref to business1
    if ((type & KContacts::PhoneNumber::Work) && (type & KContacts::PhoneNumber::Pref)) {
        types << QStringLiteral("business1");
        type = type & ~KContacts::PhoneNumber::Work;
        type = type & ~KContacts::PhoneNumber::Pref;
    }

    if (type & KContacts::PhoneNumber::Home) {
        types << QStringLiteral("home2");
    }
    if (type & KContacts::PhoneNumber::Msg) { // Msg==messaging
        types << QStringLiteral("company");
    }
    if (type & KContacts::PhoneNumber::Work) {
        types << QStringLiteral("business2");
    }
    if (type & KContacts::PhoneNumber::Pref) {
        types << QStringLiteral("primary");
    }
    if (type & KContacts::PhoneNumber::Voice) {
        types << QStringLiteral("callback"); // ##
    }
    if (type & KContacts::PhoneNumber::Cell) {
        types << QStringLiteral("mobile");
    }
    if (type & KContacts::PhoneNumber::Video) {
        types << QStringLiteral("radio"); // ##
    }
    if (type & KContacts::PhoneNumber::Bbs) {
        types << QStringLiteral("ttytdd");
    }
    if (type & KContacts::PhoneNumber::Modem) {
        types << QStringLiteral("telex"); // #
    }
    if (type & KContacts::PhoneNumber::Car) {
        types << QStringLiteral("car");
    }
    if (type & KContacts::PhoneNumber::Isdn) {
        types << QStringLiteral("isdn");
    }
    if (type & KContacts::PhoneNumber::Pcs) {
        types << QStringLiteral("assistant"); // ## Assistant is e.g. secretary
    }
    if (type & KContacts::PhoneNumber::Pager) {
        types << QStringLiteral("pager");
    }
    return types;
}

static KContacts::PhoneNumber::Type phoneTypeFromString(const QString &type)
{
    if (type == QLatin1String("homefax")) {
        return KContacts::PhoneNumber::Home | KContacts::PhoneNumber::Fax;
    }
    if (type == QLatin1String("businessfax")) {
        return KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Fax;
    }
    if (type == QLatin1String("business1")) {
        return KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Pref;
    }
    if (type == QLatin1String("business2")) {
        return KContacts::PhoneNumber::Work;
    }
    if (type == QLatin1String("home1")) {
        return KContacts::PhoneNumber::Home | KContacts::PhoneNumber::Pref;
    }
    if (type == QLatin1String("home2")) {
        return KContacts::PhoneNumber::Home;
    }
    if (type == QLatin1String("company")) {
        return KContacts::PhoneNumber::Msg;
    }
    if (type == QLatin1String("primary")) {
        return KContacts::PhoneNumber::Pref;
    }
    if (type == QLatin1String("callback")) {
        return KContacts::PhoneNumber::Voice;
    }
    if (type == QLatin1String("mobile")) {
        return KContacts::PhoneNumber::Cell;
    }
    if (type == QLatin1String("radio")) {
        return KContacts::PhoneNumber::Video;
    }
    if (type == QLatin1String("ttytdd")) {
        return KContacts::PhoneNumber::Bbs;
    }
    if (type == QLatin1String("telex")) {
        return KContacts::PhoneNumber::Modem;
    }
    if (type == QLatin1String("car")) {
        return KContacts::PhoneNumber::Car;
    }
    if (type == QLatin1String("isdn")) {
        return KContacts::PhoneNumber::Isdn;
    }
    if (type == QLatin1String("assistant")) {
        return KContacts::PhoneNumber::Pcs;
    }
    if (type == QLatin1String("pager")) {
        return KContacts::PhoneNumber::Pager;
    }
    return KContacts::PhoneNumber::Home; // whatever
}

static const char *s_knownCustomFields[] =
    {"X-IMAddress", "X-Office", "X-Profession", "X-ManagersName", "X-AssistantsName", "X-SpousesName", "X-Anniversary", "DistributionList", nullptr};

// The saving is addressee -> Contact -> xml, this is the first part
void Contact::setFields(const KContacts::Addressee *addressee)
{
    KolabBase::setFields(addressee);

    setGivenName(addressee->givenName());
    setMiddleNames(addressee->additionalName());
    setLastName(addressee->familyName());
    setFullName(addressee->formattedName());
    setPrefix(addressee->prefix());
    setSuffix(addressee->suffix());
    setOrganization(addressee->organization());
    setWebPage(addressee->url().url().url());
    setIMAddress(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress")));
    setDepartment(addressee->department());
    setOfficeLocation(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office")));
    setProfession(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession")));
    setRole(addressee->role());
    setTitle(addressee->title());
    setManagerName(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName")));
    setAssistant(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName")));
    setNickName(addressee->nickName());
    setSpouseName(addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName")));
    if (!addressee->birthday().isNull()) {
        setBirthday(addressee->birthday().date());
    }
    const QString &anniversary = addressee->custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"));
    if (!anniversary.isEmpty()) {
        setAnniversary(stringToDate(anniversary));
    }

    const QStringList emails = addressee->emails();
    // Conversion problem here:
    // KContacts::Addressee has only one full name and N addresses, but the XML format
    // has N times (fullname+address). So we just copy the fullname over and ignore it on loading.
    for (QStringList::ConstIterator it = emails.constBegin(), end = emails.constEnd(); it != end; ++it) {
        Email email;
        email.displayName = fullName();
        email.smtpAddress = *it;
        addEmail(email);
    }

    // save formatted full email for later usage
    mFullEmail = addressee->fullEmail();

    // Now the real-world addresses
    QString preferredAddress = QStringLiteral("home");
    const KContacts::Address::List addresses = addressee->addresses();
    for (KContacts::Address::List::ConstIterator it = addresses.constBegin(), end = addresses.constEnd(); it != end; ++it) {
        Address address;
        address.kdeAddressType = (*it).type();
        address.type = addressTypeToString(address.kdeAddressType);
        address.street = (*it).street();
        address.pobox = (*it).postOfficeBox();
        address.locality = (*it).locality();
        address.region = (*it).region();
        address.postalCode = (*it).postalCode();
        address.country = (*it).country();
        // ## TODO not in the XML format: extended address info.
        // ## KDE-specific tags? Or hiding those fields? Or adding a warning?
        addAddress(address);
        if (address.kdeAddressType & KContacts::Address::Pref) {
            preferredAddress = address.type; // home, business or other
        }
    }
    setPreferredAddress(preferredAddress);

    const KContacts::PhoneNumber::List phones = addressee->phoneNumbers();
    for (KContacts::PhoneNumber::List::ConstIterator it = phones.constBegin(), endIt = phones.constEnd(); it != endIt; ++it) {
        // Create a tag per phone type set in the bitfield
        QStringList types = phoneTypeToString((*it).type());
        for (QStringList::ConstIterator typit = types.constBegin(), end = types.constEnd(); typit != end; ++typit) {
            PhoneNumber phoneNumber;
            phoneNumber.type = *typit;
            phoneNumber.number = (*it).number();
            addPhoneNumber(phoneNumber);
        }
    }

    setPicture(loadPictureFromAddressee(addressee->photo()), addressee->photo().type());
    mPictureAttachmentName = addressee->custom(QStringLiteral("KOLAB"), QStringLiteral("PictureAttachmentName"));
    if (mPictureAttachmentName.isEmpty()) {
        mPictureAttachmentName = defaultPictureAttachmentName();
    }

    setLogo(loadPictureFromAddressee(addressee->logo()), addressee->logo().type());
    mLogoAttachmentName = addressee->custom(QStringLiteral("KOLAB"), QStringLiteral("LogoAttachmentName"));
    if (mLogoAttachmentName.isEmpty()) {
        mLogoAttachmentName = defaultLogoAttachmentName();
    }

    setSound(loadSoundFromAddressee(addressee->sound()));
    mSoundAttachmentName = addressee->custom(QStringLiteral("KOLAB"), QStringLiteral("SoundAttachmentName"));
    if (mSoundAttachmentName.isEmpty()) {
        mSoundAttachmentName = defaultSoundAttachmentName();
    }

    if (addressee->geo().isValid()) {
        setLatitude(addressee->geo().latitude());
        setLongitude(addressee->geo().longitude());
        mHasGeo = true;
    }

    // Other KADDRESSBOOK custom fields than those already handled
    //    (includes e.g. crypto settings, and extra im addresses)
    QStringList knownCustoms;
    for (const char **p = s_knownCustomFields; *p; ++p) {
        knownCustoms << QString::fromLatin1(*p);
    }
    const QStringList customs = addressee->customs();
    for (QStringList::ConstIterator it = customs.constBegin(), end = customs.constEnd(); it != end; ++it) {
        // KContacts::Addressee doesn't offer a real way to iterate over customs, other than splitting strings ourselves
        // The format is "app-name:value".
        int pos = (*it).indexOf(QLatin1Char('-'));
        if (pos == -1) {
            continue;
        }
        QString app = (*it).left(pos);
        if (app == QLatin1String("KOLAB")) {
            continue;
        }
        QString name = (*it).mid(pos + 1);
        pos = name.indexOf(QLatin1Char(':'));
        if (pos == -1) {
            continue;
        }
        QString value = name.mid(pos + 1);
        name.truncate(pos);
        if (!knownCustoms.contains(name)) {
            // qCDebug(PIMKOLAB_LOG) <<"app=" << app <<" name=" << name <<" value=" << value;
            Custom c;
            if (app != QLatin1String("KADDRESSBOOK")) { // that's the default
                c.app = app;
            }
            c.name = name;
            c.value = value;
            mCustomList.append(c);
        }
    }

    const QString url = addressee->custom(QStringLiteral("KOLAB"), QStringLiteral("FreebusyUrl"));
    if (!url.isEmpty()) {
        setFreeBusyUrl(url);
    }

    // Those fields, although defined in Addressee, are not used in KDE
    // (e.g. not visible in kaddressbook/addresseeeditorwidget.cpp)
    // So it doesn't matter much if we don't have them in the XML.
    // mailer, timezone, productId, sortString, agent, rfc2426 name()

    // Things KAddressBook can't handle, so they are saved as unhandled tags:
    // initials, children, gender, language
}

// The loading is: xml -> Contact -> addressee, this is the second part
void Contact::saveTo(KContacts::Addressee *addressee)
{
    // TODO: This needs the same set of TODOs as the setFields method
    KolabBase::saveTo(addressee);
    KContacts::ResourceLocatorUrl url;

    url.setUrl(QUrl(webPage()));

    addressee->setGivenName(givenName());
    addressee->setAdditionalName(middleNames());
    addressee->setFamilyName(lastName());
    addressee->setFormattedName(fullName());
    addressee->setPrefix(prefix());
    addressee->setSuffix(suffix());
    addressee->setOrganization(organization());
    addressee->setUrl(url);
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress"), imAddress());
    addressee->setDepartment(department());
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), officeLocation());
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), profession());
    addressee->setRole(role());
    addressee->setTitle(title());
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), managerName());
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), assistant());
    addressee->setNickName(nickName());
    addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), spouseName());
    if (birthday().isValid()) {
        addressee->setBirthday(QDateTime(birthday().startOfDay()));
    }

    if (anniversary().isValid()) {
        addressee->insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), dateToString(anniversary()));
    } else {
        addressee->removeCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"));
    }

    addressee->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("FreebusyUrl"), freeBusyUrl());

    // We need to store both the original attachment name and the picture data into the addressee.
    // This is important, otherwise we would save the image under another attachment name w/o deleting the original one!
    if (!mPicture.isNull()) {
        KContacts::Picture picture(mPicture);
        addressee->setPhoto(picture);
    }
    // Note that we must save the filename in all cases, so that removing the picture
    // actually deletes the attachment.
    addressee->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("PictureAttachmentName"), mPictureAttachmentName);
    if (!mLogo.isNull()) {
        KContacts::Picture picture(mLogo);
        addressee->setLogo(picture);
    }
    addressee->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("LogoAttachmentName"), mLogoAttachmentName);
    if (!mSound.isNull()) {
        addressee->setSound(KContacts::Sound(mSound));
    }
    addressee->insertCustom(QStringLiteral("KOLAB"), QStringLiteral("SoundAttachmentName"), mSoundAttachmentName);

    if (mHasGeo) {
        addressee->setGeo(KContacts::Geo(mLatitude, mLongitude));
    }

    QStringList emailAddresses;
    for (QList<Email>::ConstIterator it = mEmails.constBegin(), end = mEmails.constEnd(); it != end; ++it) {
        // we can't do anything with (*it).displayName
        emailAddresses.append((*it).smtpAddress);
    }
    addressee->setEmails(emailAddresses);

    for (QList<Address>::ConstIterator it = mAddresses.constBegin(), end = mAddresses.constEnd(); it != end; ++it) {
        KContacts::Address address;
        int type = (*it).kdeAddressType;
        if (type == -1) { // no kde-specific type available
            type = addressTypeFromString((*it).type);
            if ((*it).type == mPreferredAddress) {
                type |= KContacts::Address::Pref;
            }
        }
        address.setType(static_cast<KContacts::Address::Type>(type));
        address.setStreet((*it).street);
        address.setPostOfficeBox((*it).pobox);
        address.setLocality((*it).locality);
        address.setRegion((*it).region);
        address.setPostalCode((*it).postalCode);
        address.setCountry((*it).country);
        addressee->insertAddress(address);
    }

    for (QList<PhoneNumber>::ConstIterator it = mPhoneNumbers.constBegin(), end = mPhoneNumbers.constEnd(); it != end; ++it) {
        KContacts::PhoneNumber number;
        number.setType(phoneTypeFromString((*it).type));
        number.setNumber((*it).number);
        addressee->insertPhoneNumber(number);
    }

    for (QList<Custom>::ConstIterator it = mCustomList.constBegin(), end = mCustomList.constEnd(); it != end; ++it) {
        QString app = (*it).app.isEmpty() ? QStringLiteral("KADDRESSBOOK") : (*it).app;
        addressee->insertCustom(app, (*it).name, (*it).value);
    }
    // qCDebug(PIMKOLAB_LOG) << addressee->customs();
}

QImage Contact::loadPictureFromAddressee(const KContacts::Picture &picture)
{
    QImage img;
    if (!picture.isIntern() && !picture.url().isEmpty()) {
        qCWarning(PIMKOLAB_LOG) << "external pictures are currently not supported";
        // FIXME add kio support to libcalendaring or use libcurl
        //     if ( KIO::NetAccess::download( picture.url(), tmpFile, 0 /*no widget known*/ ) ) {
        //       img.load( tmpFile );
        //       KIO::NetAccess::removeTempFile( tmpFile );
        //     }
    } else {
        img = picture.data();
    }
    return img;
}

QByteArray KolabV2::Contact::loadSoundFromAddressee(const KContacts::Sound &sound)
{
    QByteArray data;
    if (!sound.isIntern() && !sound.url().isEmpty()) {
        //     if ( KIO::NetAccess::download( sound.url(), tmpFile, 0 /*no widget known*/ ) ) {
        //       QFile f( tmpFile );
        //       if ( f.open( QIODevice::ReadOnly ) ) {
        //         data = f.readAll();
        //         f.close();
        //       }
        //       KIO::NetAccess::removeTempFile( tmpFile );
        //     }
    } else {
        data = sound.data();
    }
    return data;
}

QString KolabV2::Contact::productID() const
{
    // TODO: When KAB has the version number in a header file, don't hardcode (Bo)
    // Or we could use Addressee::productID? (David)
    return QStringLiteral("KAddressBook 3.3, Kolab resource");
}
