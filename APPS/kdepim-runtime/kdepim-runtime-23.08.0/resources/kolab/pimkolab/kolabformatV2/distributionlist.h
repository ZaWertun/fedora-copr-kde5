/*
    This file is part of Akonadi KolabProxy
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kolabbase.h"

namespace KContacts
{
class ContactGroup;
}

namespace KolabV2
{
class DistributionList : public KolabBase
{
public:
    explicit DistributionList(const KContacts::ContactGroup *contactGroup);
    DistributionList(const QString &xml);
    ~DistributionList() override;

    void saveTo(KContacts::ContactGroup *contactGroup);

    QString type() const override;

    void setName(const QString &name);
    QString name() const;

    // Load the attributes of this class
    bool loadAttribute(QDomElement &) override;

    // Save the attributes of this class
    bool saveAttributes(QDomElement &) const override;

    // Load this note by reading the XML file
    bool loadXML(const QDomDocument &xml) override;

    // Serialize this note to an XML string
    QString saveXML() const override;

    QString productID() const override;

protected:
    void setFields(const KContacts::ContactGroup *);

private:
    void loadDistrListMember(const QDomElement &element);
    void saveDistrListMembers(QDomElement &element) const;

    QString mName;

    struct Custom {
        QString app;
        QString name;
        QString value;
    };
    QList<Custom> mCustomList;

    struct Member {
        QString displayName;
        QString email;
        QString uid;
    };
    QList<Member> mDistrListMembers;
};
}

// kate: space-indent on; indent-width 2; replace-tabs on;
