/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contactutils.h"

#include "davutils.h"
#include "oxutils.h"
#include "users.h"

#include <Akonadi/ContactGroupExpandJob>

#include <QBuffer>
#include <QDomElement>
#include <QImage>
#include <QRegularExpression>
using namespace OXA;

void OXA::ContactUtils::parseContact(const QDomElement &propElement, Object &object)
{
    bool isDistributionList = false;
    QDomElement distributionListElement = propElement.firstChildElement(QStringLiteral("distributionlist_flag"));
    if (!distributionListElement.isNull()) {
        if (OXUtils::readBoolean(distributionListElement.text()) == true) {
            isDistributionList = true;
        }
    }

    if (isDistributionList) {
        KContacts::ContactGroup contactGroup;

        QDomElement element = propElement.firstChildElement();
        while (!element.isNull()) {
            const QString tagName = element.tagName();
            const QString text = OXUtils::readString(element.text());

            if (tagName == QLatin1String("displayname")) {
                contactGroup.setName(text);
            } else if (tagName == QLatin1String("distributionlist")) {
                QDomElement emailElement = element.firstChildElement();
                while (!emailElement.isNull()) {
                    const QString tagName = emailElement.tagName();
                    const QString text = OXUtils::readString(emailElement.text());

                    if (tagName == QLatin1String("email")) {
                        const int emailField = OXUtils::readNumber(emailElement.attribute(QStringLiteral("emailfield")));
                        if (emailField == 0) { // internal data
                            KContacts::ContactGroup::Data data;
                            data.setName(OXUtils::readString(emailElement.attribute(QStringLiteral("displayname"))));
                            data.setEmail(text);

                            contactGroup.append(data);
                        } else { // external reference
                            // we convert them to internal data, seems like a more stable approach
                            KContacts::ContactGroup::Data data;
                            const qlonglong uid = OXUtils::readNumber(emailElement.attribute(QStringLiteral("id")));

                            const User user = Users::self()->lookupUid(uid);
                            if (user.isValid()) {
                                data.setName(user.name());
                                data.setEmail(user.email());
                            } else {
                                // fallback: use the data from the element
                                data.setName(OXUtils::readString(emailElement.attribute(QStringLiteral("displayname"))));
                                data.setEmail(text);
                            }

                            contactGroup.append(data);
                        }
                    }

                    emailElement = emailElement.nextSiblingElement();
                }
            }
            element = element.nextSiblingElement();
        }

        object.setContactGroup(contactGroup);
    } else {
        KContacts::Addressee contact;
        KContacts::Address homeAddress(KContacts::Address::Home);
        KContacts::Address workAddress(KContacts::Address::Work);
        KContacts::Address otherAddress(KContacts::Address::Dom);

        QDomElement element = propElement.firstChildElement();
        while (!element.isNull()) {
            const QString tagName = element.tagName();
            const QString text = OXUtils::readString(element.text());

            // name
            if (tagName == QLatin1String("title")) {
                contact.setTitle(text);
            } else if (tagName == QLatin1String("first_name")) {
                contact.setGivenName(text);
            } else if (tagName == QLatin1String("second_name")) {
                contact.setAdditionalName(text);
            } else if (tagName == QLatin1String("last_name")) {
                contact.setFamilyName(text);
            } else if (tagName == QLatin1String("suffix")) {
                contact.setSuffix(text);
            } else if (tagName == QLatin1String("displayname")) {
                contact.setFormattedName(text);
            } else if (tagName == QLatin1String("nickname")) {
                contact.setNickName(text);
                // dates
            } else if (tagName == QLatin1String("birthday")) {
                contact.setBirthday(OXUtils::readDateTime(element.text()));
            } else if (tagName == QLatin1String("anniversary")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"),
                                     QStringLiteral("X-Anniversary"),
                                     OXUtils::readDateTime(element.text()).toString(Qt::ISODate));
            } else if (tagName == QLatin1String("spouse_name")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), text);
                // addresses
            } else if (tagName == QLatin1String("street")) {
                homeAddress.setStreet(text);
            } else if (tagName == QLatin1String("postal_code")) {
                homeAddress.setPostalCode(text);
            } else if (tagName == QLatin1String("city")) {
                homeAddress.setLocality(text);
            } else if (tagName == QLatin1String("country")) {
                homeAddress.setCountry(text);
            } else if (tagName == QLatin1String("state")) {
                homeAddress.setRegion(text);
            } else if (tagName == QLatin1String("business_street")) {
                workAddress.setStreet(text);
            } else if (tagName == QLatin1String("business_postal_code")) {
                workAddress.setPostalCode(text);
            } else if (tagName == QLatin1String("business_city")) {
                workAddress.setLocality(text);
            } else if (tagName == QLatin1String("business_country")) {
                workAddress.setCountry(text);
            } else if (tagName == QLatin1String("business_state")) {
                workAddress.setRegion(text);
            } else if (tagName == QLatin1String("second_street")) {
                otherAddress.setStreet(text);
            } else if (tagName == QLatin1String("second_postal_code")) {
                otherAddress.setPostalCode(text);
            } else if (tagName == QLatin1String("second_city")) {
                otherAddress.setLocality(text);
            } else if (tagName == QLatin1String("second_country")) {
                otherAddress.setCountry(text);
            } else if (tagName == QLatin1String("second_state")) {
                otherAddress.setRegion(text);
            } else if (tagName == QLatin1String("defaultaddress")) {
                const int number = text.toInt();
                if (number == 1) {
                    workAddress.setType(workAddress.type() | KContacts::Address::Pref);
                } else if (number == 2) {
                    homeAddress.setType(homeAddress.type() | KContacts::Address::Pref);
                } else if (number == 3) {
                    otherAddress.setType(otherAddress.type() | KContacts::Address::Pref);
                }
                // further information
            } else if (tagName == QLatin1String("note")) {
                contact.setNote(text);
            } else if (tagName == QLatin1String("url")) {
                KContacts::ResourceLocatorUrl url;
                url.setUrl(QUrl(text));
                contact.setUrl(url);
            } else if (tagName == QLatin1String("image1")) {
                const QByteArray data = text.toUtf8();
                contact.setPhoto(KContacts::Picture(QImage::fromData(QByteArray::fromBase64(data))));
                // company information
            } else if (tagName == QLatin1String("company")) {
                contact.setOrganization(text);
            } else if (tagName == QLatin1String("department")) {
                contact.setDepartment(text);
            } else if (tagName == QLatin1String("assistants_name")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"), text);
            } else if (tagName == QLatin1String("managers_name")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"), text);
            } else if (tagName == QLatin1String("position")) {
                contact.setRole(text);
            } else if (tagName == QLatin1String("profession")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"), text);
            } else if (tagName == QLatin1String("room_number")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"), text);
                // communication
            } else if (tagName == QLatin1String("email1")) {
                KContacts::Email email(text);
                email.setPreferred(true);
                contact.addEmail(email);
            } else if (tagName == QLatin1String("email2") || tagName == QLatin1String("email3")) {
                KContacts::Email email(text);
                contact.addEmail(email);
            } else if (tagName == QLatin1String("mobile1")) {
                contact.insertPhoneNumber(KContacts::PhoneNumber(text, KContacts::PhoneNumber::Cell));
            } else if (tagName == QLatin1String("instant_messenger")) {
                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress"), text);
            } else if (tagName.startsWith(QLatin1String("phone_"))) {
                KContacts::PhoneNumber number;
                number.setNumber(text);
                bool supportedType = false;

                if (tagName.endsWith(QLatin1String("_business"))) {
                    number.setType(KContacts::PhoneNumber::Work);
                    supportedType = true;
                } else if (tagName.endsWith(QLatin1String("_home"))) {
                    number.setType(KContacts::PhoneNumber::Home);
                    supportedType = true;
                } else if (tagName.endsWith(QLatin1String("_other"))) {
                    number.setType(KContacts::PhoneNumber::Voice);
                    supportedType = true;
                } else if (tagName.endsWith(QLatin1String("_car"))) {
                    number.setType(KContacts::PhoneNumber::Car);
                    supportedType = true;
                }

                if (supportedType) {
                    contact.insertPhoneNumber(number);
                }
            } else if (tagName.startsWith(QLatin1String("fax_"))) {
                KContacts::PhoneNumber number;
                number.setNumber(text);
                bool supportedType = false;

                if (tagName.endsWith(QLatin1String("_business"))) {
                    number.setType(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Work);
                    supportedType = true;
                } else if (tagName.endsWith(QLatin1String("_home"))) {
                    number.setType(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Home);
                    supportedType = true;
                } else if (tagName.endsWith(QLatin1String("_other"))) {
                    number.setType(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Voice);
                    supportedType = true;
                }

                if (supportedType) {
                    contact.insertPhoneNumber(number);
                }
            } else if (tagName == QLatin1String("pager")) {
                contact.insertPhoneNumber(KContacts::PhoneNumber(text, KContacts::PhoneNumber::Pager));
            } else if (tagName == QLatin1String("categories")) {
                contact.setCategories(text.split(QRegularExpression(QStringLiteral(",\\s*"))));
            }

            element = element.nextSiblingElement();
        }

        if (!homeAddress.isEmpty()) {
            contact.insertAddress(homeAddress);
        }
        if (!workAddress.isEmpty()) {
            contact.insertAddress(workAddress);
        }
        if (!otherAddress.isEmpty()) {
            contact.insertAddress(otherAddress);
        }

        object.setContact(contact);
    }
}

void OXA::ContactUtils::addContactElements(QDomDocument &document, QDomElement &propElement, const Object &object, void *preloadedData)
{
    if (!object.contact().isEmpty()) {
        // it is a contact payload

        const KContacts::Addressee contact = object.contact();

        // name
        DAVUtils::addOxElement(document, propElement, QStringLiteral("title"), OXUtils::writeString(contact.title()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("first_name"), OXUtils::writeString(contact.givenName()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("second_name"), OXUtils::writeString(contact.additionalName()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("last_name"), OXUtils::writeString(contact.familyName()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("suffix"), OXUtils::writeString(contact.suffix()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("displayname"), OXUtils::writeString(contact.formattedName()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("nickname"), OXUtils::writeString(contact.nickName()));

        // dates
        if (contact.birthday().date().isValid()) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("birthday"), OXUtils::writeDate(contact.birthday().date()));
        } else {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("birthday"));
        }

        // since QDateTime::to/fromString() doesn't carry timezone information, we have to fake it here
        const QDate anniversary = QDate::fromString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary")), Qt::ISODate);
        if (anniversary.isValid()) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("anniversary"), OXUtils::writeDate(anniversary));
        } else {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("anniversary"));
        }
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("spouse_name"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"))));

        // addresses
        const KContacts::Address homeAddress = contact.address(KContacts::Address::Home);
        if (!homeAddress.isEmpty()) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("street"), OXUtils::writeString(homeAddress.street()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("postal_code"), OXUtils::writeString(homeAddress.postalCode()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("city"), OXUtils::writeString(homeAddress.locality()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("state"), OXUtils::writeString(homeAddress.region()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("country"), OXUtils::writeString(homeAddress.country()));
        }
        const KContacts::Address workAddress = contact.address(KContacts::Address::Work);
        if (!workAddress.isEmpty()) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("business_street"), OXUtils::writeString(workAddress.street()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("business_postal_code"), OXUtils::writeString(workAddress.postalCode()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("business_city"), OXUtils::writeString(workAddress.locality()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("business_state"), OXUtils::writeString(workAddress.region()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("business_country"), OXUtils::writeString(workAddress.country()));
        }
        const KContacts::Address otherAddress = contact.address(KContacts::Address::Dom);
        if (!otherAddress.isEmpty()) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("second_street"), OXUtils::writeString(otherAddress.street()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("second_postal_code"), OXUtils::writeString(otherAddress.postalCode()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("second_city"), OXUtils::writeString(otherAddress.locality()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("second_state"), OXUtils::writeString(otherAddress.region()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("second_country"), OXUtils::writeString(otherAddress.country()));
        }

        // further information
        DAVUtils::addOxElement(document, propElement, QStringLiteral("note"), OXUtils::writeString(contact.note()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("url"), OXUtils::writeString(contact.url().url().url()));

        // image
        const KContacts::Picture photo = contact.photo();
        if (!photo.data().isNull()) {
            QByteArray imageData;
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);

            QString contentType;
            if (!photo.data().hasAlphaChannel()) {
                photo.data().save(&buffer, "JPEG");
                contentType = QStringLiteral("image/jpg");
            } else {
                photo.data().save(&buffer, "PNG");
                contentType = QStringLiteral("image/png");
            }

            buffer.close();

            DAVUtils::addOxElement(document, propElement, QStringLiteral("image1"), QString::fromLatin1(imageData.toBase64()));
            DAVUtils::addOxElement(document, propElement, QStringLiteral("image_content_type"), contentType);
        } else {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("image1"));
        }

        // company information
        DAVUtils::addOxElement(document, propElement, QStringLiteral("company"), OXUtils::writeString(contact.organization()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("department"), OXUtils::writeString(contact.department()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("assistants_name"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-AssistantsName"))));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("managers_name"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ManagersName"))));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("position"), OXUtils::writeString(contact.role()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("profession"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"))));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("room_number"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Office"))));

        // communication
        const QStringList emails = contact.emails();
        for (int i = 0; i < 3 && i < emails.count(); ++i) {
            DAVUtils::addOxElement(document, propElement, QStringLiteral("email%1").arg(i + 1), OXUtils::writeString(emails.at(i)));
        }

        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("mobile1"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Cell).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("instant_messenger"),
                               OXUtils::writeString(contact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-IMAddress"))));

        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("phone_business"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Work).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("phone_home"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Home).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("phone_other"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Voice).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("phone_car"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Car).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("fax_business"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Work).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("fax_home"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Home).number()));
        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("fax_other"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Fax | KContacts::PhoneNumber::Voice).number()));

        DAVUtils::addOxElement(document,
                               propElement,
                               QStringLiteral("pager"),
                               OXUtils::writeString(contact.phoneNumber(KContacts::PhoneNumber::Pager).number()));

        DAVUtils::addOxElement(document, propElement, QStringLiteral("categories"), OXUtils::writeString(contact.categories().join(QLatin1Char(','))));
    } else {
        // it is a distribution list payload

        const KContacts::ContactGroup contactGroup = object.contactGroup();

        DAVUtils::addOxElement(document, propElement, QStringLiteral("displayname"), OXUtils::writeString(contactGroup.name()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("last_name"), OXUtils::writeString(contactGroup.name()));
        DAVUtils::addOxElement(document, propElement, QStringLiteral("distributionlist_flag"), OXUtils::writeBoolean(true));

        QDomElement distributionList = DAVUtils::addOxElement(document, propElement, QStringLiteral("distributionlist"));

        if (preloadedData) {
            // the contact group contains contact references that has been preloaded
            auto contacts = static_cast<KContacts::Addressee::List *>(preloadedData);
            for (const KContacts::Addressee &contact : *contacts) {
                QDomElement email = DAVUtils::addOxElement(document, distributionList, QStringLiteral("email"), OXUtils::writeString(contact.preferredEmail()));

                DAVUtils::setOxAttribute(email, QStringLiteral("folder_id"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("emailfield"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("id"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("displayname"), OXUtils::writeString(contact.realName()));
            }

            delete contacts;
        } else {
            // the contact group contains only internal contact data
            for (int i = 0; i < contactGroup.dataCount(); ++i) {
                const KContacts::ContactGroup::Data &data = contactGroup.data(i);
                QDomElement email = DAVUtils::addOxElement(document, distributionList, QStringLiteral("email"), OXUtils::writeString(data.email()));

                DAVUtils::setOxAttribute(email, QStringLiteral("folder_id"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("emailfield"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("id"), OXUtils::writeNumber(0));
                DAVUtils::setOxAttribute(email, QStringLiteral("displayname"), OXUtils::writeString(data.name()));
            }
        }
    }
}

KJob *OXA::ContactUtils::preloadJob(const Object &object)
{
    auto job = new Akonadi::ContactGroupExpandJob(object.contactGroup());
    return job;
}

void *OXA::ContactUtils::preloadData(const Object &, KJob *job)
{
    auto expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);
    Q_ASSERT(expandJob);

    return new KContacts::Addressee::List(expandJob->contacts());
}
