/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapaclattribute.h"

#include <QByteArray>

using namespace Akonadi;

ImapAclAttribute::ImapAclAttribute() = default;

ImapAclAttribute::ImapAclAttribute(const QMap<QByteArray, KIMAP::Acl::Rights> &rights, const QMap<QByteArray, KIMAP::Acl::Rights> &oldRights)
    : mRights(rights)
    , mOldRights(oldRights)
{
}

void ImapAclAttribute::setRights(const QMap<QByteArray, KIMAP::Acl::Rights> &rights)
{
    mOldRights = mRights;
    mRights = rights;
}

QMap<QByteArray, KIMAP::Acl::Rights> ImapAclAttribute::rights() const
{
    return mRights;
}

QMap<QByteArray, KIMAP::Acl::Rights> ImapAclAttribute::oldRights() const
{
    return mOldRights;
}

void ImapAclAttribute::setMyRights(KIMAP::Acl::Rights rights)
{
    mMyRights = rights;
}

KIMAP::Acl::Rights ImapAclAttribute::myRights() const
{
    return mMyRights;
}

QByteArray ImapAclAttribute::type() const
{
    static const QByteArray sType("imapacl");
    return sType;
}

ImapAclAttribute *ImapAclAttribute::clone() const
{
    auto attr = new ImapAclAttribute(mRights, mOldRights);
    attr->setMyRights(mMyRights);
    return attr;
}

QByteArray ImapAclAttribute::serialized() const
{
    QByteArray result = "";

    bool added = false;

    QMap<QByteArray, KIMAP::Acl::Rights>::const_iterator it = mRights.constBegin();
    const QMap<QByteArray, KIMAP::Acl::Rights>::const_iterator end = mRights.constEnd();
    for (; it != end; ++it) {
        result += it.key();
        result += ' ';
        result += KIMAP::Acl::rightsToString(it.value());
        result += " % "; // We use this separator as '%' is not allowed in keys or values
        added = true;
    }

    if (added) {
        result.chop(3);
    }

    result += " %% ";

    added = false;
    QMap<QByteArray, KIMAP::Acl::Rights>::const_iterator it2 = mOldRights.constBegin();
    const QMap<QByteArray, KIMAP::Acl::Rights>::const_iterator end2 = mOldRights.constEnd();
    for (; it2 != end2; ++it2) {
        result += it2.key();
        result += ' ';
        result += KIMAP::Acl::rightsToString(it2.value());
        result += " % "; // We use this separator as '%' is not allowed in keys or values
        added = true;
    }

    if (added) {
        result.chop(3);
    }

    if (mMyRights) {
        result += " %% ";
        result += KIMAP::Acl::rightsToString(mMyRights);
    }

    return result;
}

static void fillRightsMap(const QList<QByteArray> &rights, QMap<QByteArray, KIMAP::Acl::Rights> &map)
{
    for (const QByteArray &right : rights) {
        const QByteArray trimmed = right.trimmed();
        const int wsIndex = trimmed.indexOf(' ');
        const QByteArray id = trimmed.mid(0, wsIndex).trimmed();
        if (!id.isEmpty()) {
            const bool noValue = (wsIndex == -1);
            if (noValue) {
                map[id] = KIMAP::Acl::None;
            } else {
                const QByteArray value = trimmed.mid(wsIndex + 1, right.length() - wsIndex).trimmed();
                map[id] = KIMAP::Acl::rightsFromString(value);
            }
        }
    }
}

void ImapAclAttribute::deserialize(const QByteArray &data)
{
    mRights.clear();
    mOldRights.clear();
    mMyRights = KIMAP::Acl::None;

    QList<QByteArray> parts;
    int lastPos = 0;
    int pos;
    while ((pos = data.indexOf(" %% ", lastPos)) != -1) {
        parts << data.mid(lastPos, pos - lastPos);
        lastPos = pos + 4;
    }
    parts << data.mid(lastPos);

    if (parts.size() < 2) {
        return;
    }
    fillRightsMap(parts.at(0).split('%'), mRights);
    fillRightsMap(parts.at(1).split('%'), mOldRights);
    if (parts.size() >= 3) {
        mMyRights = KIMAP::Acl::rightsFromString(parts.at(2));
    }
}

bool ImapAclAttribute::operator==(const ImapAclAttribute &other) const
{
    return (oldRights() == other.oldRights()) && (rights() == other.rights()) && (myRights() == other.myRights());
}
