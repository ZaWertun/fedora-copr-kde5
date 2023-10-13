/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "object.h"

using namespace OXA;

Object::Object() = default;

void Object::setObjectStatus(ObjectStatus status)
{
    mObjectStatus = status;
}

Object::ObjectStatus Object::objectStatus() const
{
    return mObjectStatus;
}

void Object::setObjectId(qlonglong id)
{
    mObjectId = id;
}

qlonglong Object::objectId() const
{
    return mObjectId;
}

void Object::setFolderId(qlonglong id)
{
    mFolderId = id;
}

qlonglong Object::folderId() const
{
    return mFolderId;
}

void Object::setLastModified(const QString &timeStamp)
{
    mLastModified = timeStamp;
}

QString Object::lastModified() const
{
    return mLastModified;
}

void Object::setModule(Folder::Module module)
{
    mModule = module;
}

Folder::Module Object::module() const
{
    return mModule;
}

void Object::setContact(const KContacts::Addressee &contact)
{
    mModule = Folder::Contacts;
    mContact = contact;
}

KContacts::Addressee Object::contact() const
{
    return mContact;
}

void Object::setContactGroup(const KContacts::ContactGroup &group)
{
    mModule = Folder::Contacts;
    mContactGroup = group;
}

KContacts::ContactGroup Object::contactGroup() const
{
    return mContactGroup;
}

void Object::setEvent(const KCalendarCore::Incidence::Ptr &event)
{
    mModule = Folder::Calendar;
    mEvent = event;
}

KCalendarCore::Incidence::Ptr Object::event() const
{
    return mEvent;
}

void Object::setTask(const KCalendarCore::Incidence::Ptr &task)
{
    mModule = Folder::Tasks;
    mTask = task;
}

KCalendarCore::Incidence::Ptr Object::task() const
{
    return mTask;
}
