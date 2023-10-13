/*
    This file is part of Akonadi KolabProxy.
    SPDX-FileCopyrightText: 2009 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "distributionlist.h"
#include "pimkolab_debug.h"

#include <KContacts/ContactGroup>

using namespace KolabV2;

namespace
{
inline QString unhandledTagAppName()
{
    return QStringLiteral("KOLABUNHANDLED");
} // no hyphens in appnames!
}
// saving (contactgroup->xml)
DistributionList::DistributionList(const KContacts::ContactGroup *contactGroup)
{
    setFields(contactGroup);
}

// loading (xml->contactgroup)
DistributionList::DistributionList(const QString &xml)
{
    load(xml);
}

DistributionList::~DistributionList() = default;

void DistributionList::setName(const QString &name)
{
    mName = name;
}

QString DistributionList::name() const
{
    return mName;
}

void KolabV2::DistributionList::loadDistrListMember(const QDomElement &element)
{
    Member member;
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isComment()) {
            continue;
        }
        if (n.isElement()) {
            QDomElement e = n.toElement();
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("display-name")) {
                member.displayName = e.text();
            } else if (tagName == QLatin1String("smtp-address")) {
                member.email = e.text();
            } else if (tagName == QLatin1String("uid")) {
                member.uid = e.text();
            }
        }
    }
    mDistrListMembers.append(member);
}

void DistributionList::saveDistrListMembers(QDomElement &element) const
{
    QList<Member>::ConstIterator it = mDistrListMembers.constBegin();
    const QList<Member>::ConstIterator end = mDistrListMembers.constEnd();
    for (; it != end; ++it) {
        QDomElement e = element.ownerDocument().createElement(QStringLiteral("member"));
        element.appendChild(e);
        const Member &m = *it;
        if (!m.uid.isEmpty()) {
            writeString(e, QStringLiteral("uid"), m.uid);
        } else {
            writeString(e, QStringLiteral("display-name"), m.displayName);
            writeString(e, QStringLiteral("smtp-address"), m.email);
        }
    }
}

bool DistributionList::loadAttribute(QDomElement &element)
{
    const QString tagName = element.tagName();
    switch (tagName[0].toLatin1()) {
    case 'd':
        if (tagName == QLatin1String("display-name")) {
            setName(element.text());
            return true;
        }
        break;
    case 'm':
        if (tagName == QLatin1String("member")) {
            loadDistrListMember(element);
            return true;
        }
        break;
    default:
        break;
    }
    return KolabBase::loadAttribute(element);
}

bool DistributionList::saveAttributes(QDomElement &element) const
{
    // Save the base class elements
    KolabBase::saveAttributes(element);
    writeString(element, QStringLiteral("display-name"), name());
    saveDistrListMembers(element);

    return true;
}

bool DistributionList::loadXML(const QDomDocument &document)
{
    QDomElement top = document.documentElement();

    if (top.tagName() != QLatin1String("distribution-list")) {
        qCWarning(PIMKOLAB_LOG) << QStringLiteral("XML error: Top tag was %1 instead of the expected distribution-list").arg(top.tagName());
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

QString DistributionList::saveXML() const
{
    QDomDocument document = domTree();
    QDomElement element = document.createElement(QStringLiteral("distribution-list"));
    element.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    saveAttributes(element);
    document.appendChild(element);
    return document.toString();
}

QString DistributionList::productID() const
{
    // TODO should we get name/version from desktop file?
    return QStringLiteral("Akonadi Kolab Proxy");
}

// The saving is contactgroup -> DistributionList -> xml, this is the first part
void DistributionList::setFields(const KContacts::ContactGroup *contactGroup)
{
    KolabBase::setFields(contactGroup);

    setName(contactGroup->name());

    // explicit contact data
    for (int index = 0; index < contactGroup->dataCount(); ++index) {
        const KContacts::ContactGroup::Data &data = contactGroup->data(index);

        Member m;
        m.displayName = data.name();
        m.email = data.email();

        mDistrListMembers.append(m);
    }
    for (int index = 0; index < contactGroup->contactReferenceCount(); ++index) {
        const KContacts::ContactGroup::ContactReference &data = contactGroup->contactReference(index);

        Member m;
        m.uid = data.uid();

        mDistrListMembers.append(m);
    }
    if (contactGroup->contactGroupReferenceCount() > 0) {
        qCWarning(PIMKOLAB_LOG) << "Tried to save contact group references, which should have been resolved already";
    }
}

// The loading is: xml -> DistributionList -> contactgroup, this is the second part
void DistributionList::saveTo(KContacts::ContactGroup *contactGroup)
{
    KolabBase::saveTo(contactGroup);

    contactGroup->setName(name());

    QList<Member>::ConstIterator mit = mDistrListMembers.constBegin();
    const QList<Member>::ConstIterator mEnd = mDistrListMembers.constEnd();
    for (; mit != mEnd; ++mit) {
        if (!(*mit).uid.isEmpty()) {
            contactGroup->append(KContacts::ContactGroup::ContactReference((*mit).uid));
        } else {
            contactGroup->append(KContacts::ContactGroup::Data((*mit).displayName, (*mit).email));
        }
    }
}

QString DistributionList::type() const
{
    return QStringLiteral("DistributionList");
}

// kate: space-indent on; indent-width 2; replace-tabs on;
