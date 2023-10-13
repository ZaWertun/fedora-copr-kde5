/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "folder.h"

#include <KCalendarCore/Incidence>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

#include <QString>
#include <QVector>

namespace OXA
{
class Object
{
public:
    /**
     * Describes a list of objects.
     */
    using List = QVector<Object>;

    /**
     * Describes the status of the object.
     */
    enum ObjectStatus {
        Created, ///< The object has been created or modified.
        Deleted ///< The object has been deleted.
    };

    Object();

    void setObjectStatus(ObjectStatus status);
    Q_REQUIRED_RESULT ObjectStatus objectStatus() const;

    void setObjectId(qlonglong id);
    Q_REQUIRED_RESULT qlonglong objectId() const;

    void setFolderId(qlonglong id);
    Q_REQUIRED_RESULT qlonglong folderId() const;

    void setLastModified(const QString &timeStamp);
    Q_REQUIRED_RESULT QString lastModified() const;

    void setModule(Folder::Module module);
    Q_REQUIRED_RESULT Folder::Module module() const;

    void setContact(const KContacts::Addressee &contact);
    Q_REQUIRED_RESULT KContacts::Addressee contact() const;

    void setContactGroup(const KContacts::ContactGroup &group);
    Q_REQUIRED_RESULT KContacts::ContactGroup contactGroup() const;

    void setEvent(const KCalendarCore::Incidence::Ptr &event);
    Q_REQUIRED_RESULT KCalendarCore::Incidence::Ptr event() const;

    void setTask(const KCalendarCore::Incidence::Ptr &task);
    Q_REQUIRED_RESULT KCalendarCore::Incidence::Ptr task() const;

private:
    ObjectStatus mObjectStatus = Created;
    qlonglong mObjectId = -1;
    qlonglong mFolderId = -1;
    QString mLastModified;
    Folder::Module mModule;
    KContacts::Addressee mContact;
    KContacts::ContactGroup mContactGroup;
    KCalendarCore::Incidence::Ptr mEvent;
    KCalendarCore::Incidence::Ptr mTask;
};
}

Q_DECLARE_TYPEINFO(OXA::Object, Q_MOVABLE_TYPE);
