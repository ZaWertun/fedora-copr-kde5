/*
    This file is part of libkabc and/or kaddressbook.
    SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB <info@klaralvdalens-datakonsult.se>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kolabbase.h"
#include <QImage>

namespace KContacts
{
class Addressee;
class Picture;
class Sound;
}

namespace KolabV2
{
class Contact : public KolabBase
{
public:
    struct PhoneNumber {
    public:
        QString type;
        QString number;
    };

    struct Address {
    public:
        Address()
            : kdeAddressType(-1)
        {
        }

        int kdeAddressType; // KContacts::Address::Type
        QString type; // kolab-compliant address type: home, work or other
        QString street;
        QString pobox;
        QString locality;
        QString region;
        QString postalCode;
        QString country;
    };

    explicit Contact(const KContacts::Addressee *address);
    Contact(const QString &xml);
    ~Contact() override;

    void saveTo(KContacts::Addressee *address);

    QString type() const override
    {
        return QStringLiteral("Contact");
    }

    void setGivenName(const QString &name);
    QString givenName() const;

    void setMiddleNames(const QString &names);
    QString middleNames() const;

    void setLastName(const QString &name);
    QString lastName() const;

    void setFullName(const QString &name);
    QString fullName() const;

    void setInitials(const QString &initials);
    QString initials() const;

    void setPrefix(const QString &prefix);
    QString prefix() const;

    void setSuffix(const QString &suffix);
    QString suffix() const;

    void setRole(const QString &role);
    QString role() const;

    void setFreeBusyUrl(const QString &fbUrl);
    QString freeBusyUrl() const;

    void setOrganization(const QString &organization);
    QString organization() const;

    void setWebPage(const QString &url);
    QString webPage() const;

    void setIMAddress(const QString &imAddress);
    QString imAddress() const;

    void setDepartment(const QString &department);
    QString department() const;

    void setOfficeLocation(const QString &location);
    QString officeLocation() const;

    void setProfession(const QString &profession);
    QString profession() const;

    void setTitle(const QString &title);
    QString title() const;

    void setManagerName(const QString &name);
    QString managerName() const;

    void setAssistant(const QString &name);
    QString assistant() const;

    void setNickName(const QString &name);
    QString nickName() const;

    void setSpouseName(const QString &name);
    QString spouseName() const;

    void setBirthday(QDate date);
    QDate birthday() const;

    void setAnniversary(QDate date);
    QDate anniversary() const;

    void setPicture(const QImage &image, const QString &format)
    {
        mPicture = image;
        mPictureFormat = format;
    }

    QString pictureAttachmentName() const
    {
        return mPictureAttachmentName;
    }

    QString pictureFormat() const
    {
        return mPictureFormat;
    }

    QImage picture() const
    {
        return mPicture;
    }

    void setLogo(const QImage &image, const QString &format)
    {
        mLogo = image;
        mLogoFormat = format;
    }

    QString logoAttachmentName() const
    {
        return mLogoAttachmentName;
    }

    QString logoFormat() const
    {
        return mLogoFormat;
    }

    QImage logo() const
    {
        return mLogo;
    }

    void setSound(const QByteArray &sound)
    {
        mSound = sound;
    }

    QString soundAttachmentName() const
    {
        return mSoundAttachmentName;
    }

    QByteArray sound() const
    {
        return mSound;
    }

    void setChildren(const QString &children);
    QString children() const;

    void setGender(const QString &gender);
    QString gender() const;

    void setLanguage(const QString &language);
    QString language() const;

    void addPhoneNumber(const PhoneNumber &number);
    QList<PhoneNumber> &phoneNumbers();
    const QList<PhoneNumber> &phoneNumbers() const;

    void addEmail(const Email &email);
    QList<Email> &emails();
    const QList<Email> &emails() const;

    QString fullEmail() const;

    void addAddress(const Address &address);
    QList<Address> &addresses();
    const QList<Address> &addresses() const;

    // which address is preferred: home or business or other
    void setPreferredAddress(const QString &address);
    QString preferredAddress() const;

    float latitude() const
    {
        return mLatitude;
    }

    void setLatitude(float latitude)
    {
        mLatitude = latitude;
    }

    float longitude() const
    {
        return mLongitude;
    }

    void setLongitude(float longitude)
    {
        mLongitude = longitude;
    }

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

    // Load this note by reading the XML file
    bool loadXML(const QDomDocument &xml) override;

    // Serialize this note to an XML string
    QString saveXML() const override;

protected:
    void setFields(const KContacts::Addressee *);

private:
    bool loadNameAttribute(QDomElement &element);
    void saveNameAttribute(QDomElement &element) const;

    bool loadPhoneAttribute(QDomElement &element);
    void savePhoneAttributes(QDomElement &element) const;

    void saveEmailAttributes(QDomElement &element) const;

    bool loadAddressAttribute(QDomElement &element);
    void saveAddressAttributes(QDomElement &element) const;

    void loadCustomAttributes(QDomElement &element);
    void saveCustomAttributes(QDomElement &element) const;

    QImage loadPictureFromAddressee(const KContacts::Picture &picture);

    QByteArray loadSoundFromAddressee(const KContacts::Sound &sound);

    QString productID() const override;

    QString mGivenName;
    QString mMiddleNames;
    QString mLastName;
    QString mFullName;
    QString mInitials;
    QString mPrefix;
    QString mSuffix;
    QString mRole;
    QString mFreeBusyUrl;
    QString mOrganization;
    QString mWebPage;
    QString mIMAddress;
    QString mDepartment;
    QString mOfficeLocation;
    QString mProfession;
    QString mTitle;
    QString mManagerName;
    QString mAssistant;
    QString mNickName;
    QString mSpouseName;
    QDate mBirthday;
    QDate mAnniversary;
    QImage mPicture;
    QString mPictureFormat;
    QImage mLogo;
    QString mLogoFormat;
    QByteArray mSound;
    QString mPictureAttachmentName;
    QString mLogoAttachmentName;
    QString mSoundAttachmentName;
    QString mChildren;
    QString mGender;
    QString mLanguage;
    QList<PhoneNumber> mPhoneNumbers;
    QList<Email> mEmails;
    QString mFullEmail;
    QList<Address> mAddresses;
    QString mPreferredAddress;
    float mLatitude;
    float mLongitude;
    bool mHasGeo = false;
    struct Custom {
        QString app;
        QString name;
        QString value;
    };
    QList<Custom> mCustomList;
};
}
