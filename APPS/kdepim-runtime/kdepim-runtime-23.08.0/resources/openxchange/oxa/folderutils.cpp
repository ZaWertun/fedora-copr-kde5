/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "folderutils.h"

#include "davutils.h"
#include "folder.h"
#include "oxutils.h"

#include <QDomElement>

using namespace OXA;

static void createFolderPermissions(const Folder &folder, QDomDocument &document, QDomElement &permissions)
{
    {
        const Folder::UserPermissions userPermissions = folder.userPermissions();
        Folder::UserPermissions::ConstIterator it = userPermissions.constBegin();
        while (it != userPermissions.constEnd()) {
            QDomElement user = DAVUtils::addOxElement(document, permissions, QStringLiteral("user"), OXUtils::writeNumber(it.key()));
            DAVUtils::setOxAttribute(user, QStringLiteral("folderpermission"), OXUtils::writeNumber(it.value().folderPermission()));
            DAVUtils::setOxAttribute(user, QStringLiteral("objectreadpermission"), OXUtils::writeNumber(it.value().objectReadPermission()));
            DAVUtils::setOxAttribute(user, QStringLiteral("objectwritepermission"), OXUtils::writeNumber(it.value().objectWritePermission()));
            DAVUtils::setOxAttribute(user, QStringLiteral("objectdeletepermission"), OXUtils::writeNumber(it.value().objectDeletePermission()));
            DAVUtils::setOxAttribute(user, QStringLiteral("admin_flag"), OXUtils::writeBoolean(it.value().adminFlag()));

            ++it;
        }
    }

    {
        const Folder::GroupPermissions groupPermissions = folder.groupPermissions();
        Folder::GroupPermissions::ConstIterator it = groupPermissions.constBegin();
        while (it != groupPermissions.constEnd()) {
            QDomElement group = DAVUtils::addOxElement(document, permissions, QStringLiteral("group"), OXUtils::writeNumber(it.key()));
            DAVUtils::setOxAttribute(group, QStringLiteral("folderpermission"), OXUtils::writeNumber(it.value().folderPermission()));
            DAVUtils::setOxAttribute(group, QStringLiteral("objectreadpermission"), OXUtils::writeNumber(it.value().objectReadPermission()));
            DAVUtils::setOxAttribute(group, QStringLiteral("objectwritepermission"), OXUtils::writeNumber(it.value().objectWritePermission()));
            DAVUtils::setOxAttribute(group, QStringLiteral("objectdeletepermission"), OXUtils::writeNumber(it.value().objectDeletePermission()));
            DAVUtils::setOxAttribute(group, QStringLiteral("admin_flag"), OXUtils::writeBoolean(it.value().adminFlag()));

            ++it;
        }
    }
}

static void parseFolderPermissions(const QDomElement &permissions, Folder &folder)
{
    Folder::UserPermissions userPermissions;
    Folder::GroupPermissions groupPermissions;

    QDomElement element = permissions.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName() == QLatin1String("user")) {
            Folder::Permissions permissions;
            permissions.setFolderPermission(
                (Folder::Permissions::FolderPermission)OXUtils::readNumber(element.attribute(QStringLiteral("folderpermission"), QStringLiteral("0"))));
            permissions.setObjectReadPermission(
                (Folder::Permissions::ObjectReadPermission)OXUtils::readNumber(element.attribute(QStringLiteral("objectreadpermission"), QStringLiteral("0"))));
            permissions.setObjectWritePermission((Folder::Permissions::ObjectWritePermission)OXUtils::readNumber(
                element.attribute(QStringLiteral("objectwritepermission"), QStringLiteral("0"))));
            permissions.setObjectDeletePermission((Folder::Permissions::ObjectDeletePermission)OXUtils::readNumber(
                element.attribute(QStringLiteral("objectdeletepermission"), QStringLiteral("0"))));
            permissions.setAdminFlag(OXUtils::readBoolean(element.attribute(QStringLiteral("admin_flag"), QStringLiteral("false"))));

            userPermissions.insert(OXUtils::readNumber(element.text()), permissions);
        } else if (element.tagName() == QLatin1String("group")) {
            Folder::Permissions permissions;
            permissions.setFolderPermission(
                (Folder::Permissions::FolderPermission)OXUtils::readNumber(element.attribute(QStringLiteral("folderpermission"), QStringLiteral("0"))));
            permissions.setObjectReadPermission(
                (Folder::Permissions::ObjectReadPermission)OXUtils::readNumber(element.attribute(QStringLiteral("objectreadpermission"), QStringLiteral("0"))));
            permissions.setObjectWritePermission((Folder::Permissions::ObjectWritePermission)OXUtils::readNumber(
                element.attribute(QStringLiteral("objectwritepermission"), QStringLiteral("0"))));
            permissions.setObjectDeletePermission((Folder::Permissions::ObjectDeletePermission)OXUtils::readNumber(
                element.attribute(QStringLiteral("objectdeletepermission"), QStringLiteral("0"))));
            permissions.setAdminFlag(OXUtils::readBoolean(element.attribute(QStringLiteral("admin_flag"), QStringLiteral("false"))));

            groupPermissions.insert(OXUtils::readNumber(element.text()), permissions);
        }

        element = element.nextSiblingElement();
    }

    folder.setUserPermissions(userPermissions);
    folder.setGroupPermissions(groupPermissions);
}

Folder OXA::FolderUtils::parseFolder(const QDomElement &propElement)
{
    Folder folder;

    QDomElement element = propElement.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName() == QLatin1String("object_status")) {
            const QString content = OXUtils::readString(element.text());
            if (content == QLatin1String("CREATE")) {
                folder.setObjectStatus(Folder::Created);
            } else if (content == QLatin1String("DELETE")) {
                folder.setObjectStatus(Folder::Deleted);
            } else {
                Q_ASSERT(false);
            }
        } else if (element.tagName() == QLatin1String("title")) {
            folder.setTitle(OXUtils::readString(element.text()));
        } else if (element.tagName() == QLatin1String("owner")) {
            folder.setOwner(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("module")) {
            const QString content = OXUtils::readString(element.text());
            if (content == QLatin1String("calendar")) {
                folder.setModule(Folder::Calendar);
            } else if (content == QLatin1String("contact")) {
                folder.setModule(Folder::Contacts);
            } else if (content == QLatin1String("task")) {
                folder.setModule(Folder::Tasks);
            } else {
                folder.setModule(Folder::Unbound);
            }
        } else if (element.tagName() == QLatin1String("type")) {
            const QString content = OXUtils::readString(element.text());
            if (content == QLatin1String("public")) {
                folder.setType(Folder::Public);
            } else if (content == QLatin1String("private")) {
                folder.setType(Folder::Private);
            } else {
                Q_ASSERT(false);
            }
        } else if (element.tagName() == QLatin1String("defaultfolder")) {
            folder.setIsDefaultFolder(OXUtils::readBoolean(element.text()));
        } else if (element.tagName() == QLatin1String("last_modified")) {
            folder.setLastModified(OXUtils::readString(element.text()));
        } else if (element.tagName() == QLatin1String("object_id")) {
            folder.setObjectId(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("folder_id")) {
            folder.setFolderId(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("permissions")) {
            parseFolderPermissions(element, folder);
        }

        element = element.nextSiblingElement();
    }

    return folder;
}

void OXA::FolderUtils::addFolderElements(QDomDocument &document, QDomElement &propElement, const Folder &folder)
{
    DAVUtils::addOxElement(document, propElement, QStringLiteral("title"), OXUtils::writeString(folder.title()));
    DAVUtils::addOxElement(document, propElement, QStringLiteral("folder_id"), OXUtils::writeNumber(folder.folderId()));

    const QString type = (folder.type() == Folder::Public ? QStringLiteral("public") : QStringLiteral("private"));
    DAVUtils::addOxElement(document, propElement, QStringLiteral("type"), OXUtils::writeString(type));

    QString module;
    switch (folder.module()) {
    case Folder::Calendar:
        module = QStringLiteral("calendar");
        break;
    case Folder::Contacts:
        module = QStringLiteral("contact");
        break;
    case Folder::Tasks:
        module = QStringLiteral("task");
        break;
    default:
        break;
    }
    DAVUtils::addOxElement(document, propElement, QStringLiteral("module"), OXUtils::writeString(module));

    QDomElement permissions = DAVUtils::addOxElement(document, propElement, QStringLiteral("permissions"));
    createFolderPermissions(folder, document, permissions);
}
