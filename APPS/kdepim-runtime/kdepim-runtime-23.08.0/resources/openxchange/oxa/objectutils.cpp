/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectutils.h"

#include "contactutils.h"
#include "davutils.h"
#include "incidenceutils.h"
#include "oxutils.h"

#include <QDomElement>

using namespace OXA;

Object OXA::ObjectUtils::parseObject(const QDomElement &propElement, Folder::Module module)
{
    Object object;

    QDomElement element = propElement.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName() == QLatin1String("last_modified")) {
            object.setLastModified(OXUtils::readString(element.text()));
        } else if (element.tagName() == QLatin1String("object_id")) {
            object.setObjectId(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("folder_id")) {
            object.setFolderId(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("object_status")) {
            const QString content = OXUtils::readString(element.text());
            if (content == QLatin1String("CREATE")) {
                object.setObjectStatus(Object::Created);
            } else if (content == QLatin1String("DELETE")) {
                object.setObjectStatus(Object::Deleted);
            } else {
                Q_ASSERT(false);
            }
        }

        element = element.nextSiblingElement();
    }

    switch (module) {
    case Folder::Contacts:
        ContactUtils::parseContact(propElement, object);
        break;
    case Folder::Calendar:
        IncidenceUtils::parseEvent(propElement, object);
        break;
    case Folder::Tasks:
        IncidenceUtils::parseTask(propElement, object);
        break;
    case Folder::Unbound:
        Q_ASSERT(false);
        break;
    }

    return object;
}

void OXA::ObjectUtils::addObjectElements(QDomDocument &document, QDomElement &propElement, const Object &object, void *preloadedData)
{
    if (object.objectId() != -1) {
        DAVUtils::addOxElement(document, propElement, QStringLiteral("object_id"), OXUtils::writeNumber(object.objectId()));
    }
    if (object.folderId() != -1) {
        DAVUtils::addOxElement(document, propElement, QStringLiteral("folder_id"), OXUtils::writeNumber(object.folderId()));
    }
    if (!object.lastModified().isEmpty()) {
        DAVUtils::addOxElement(document, propElement, QStringLiteral("last_modified"), OXUtils::writeString(object.lastModified()));
    }

    switch (object.module()) {
    case Folder::Contacts:
        ContactUtils::addContactElements(document, propElement, object, preloadedData);
        break;
    case Folder::Calendar:
        IncidenceUtils::addEventElements(document, propElement, object);
        break;
    case Folder::Tasks:
        IncidenceUtils::addTaskElements(document, propElement, object);
        break;
    case Folder::Unbound:
        Q_ASSERT(false);
        break;
    }
}

bool OXA::ObjectUtils::needsPreloading(const Object &object)
{
    if (object.module() == Folder::Contacts) {
        if (object.contactGroup().contactReferenceCount() != 0) { // we have to resolve these entries first
            return true;
        }
    }

    return false;
}

KJob *OXA::ObjectUtils::preloadJob(const Object &object)
{
    if (object.module() == Folder::Contacts) {
        if (object.contactGroup().contactReferenceCount() != 0) {
            return ContactUtils::preloadJob(object);
        }
    }

    return nullptr;
}

void *OXA::ObjectUtils::preloadData(const Object &object, KJob *job)
{
    if (object.module() == Folder::Contacts) {
        if (object.contactGroup().contactReferenceCount() != 0) {
            return ContactUtils::preloadData(object, job);
        }
    }

    return nullptr;
}

QString OXA::ObjectUtils::davPath(Folder::Module module)
{
    switch (module) {
    case Folder::Contacts:
        return QStringLiteral("/servlet/webdav.contacts");
    case Folder::Calendar:
        return QStringLiteral("/servlet/webdav.calendar");
    case Folder::Tasks:
        return QStringLiteral("/servlet/webdav.tasks");
    case Folder::Unbound:
        Q_ASSERT(false);
        return {};
    }

    return {};
}
