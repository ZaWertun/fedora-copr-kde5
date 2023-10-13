/*
 * SPDX-FileCopyrightText: 2012 Sofia Balicka <balicka@kolabsys.com>
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "kolab_export.h"

#include "kolabdefinitions.h"
#include <kolabformat.h>

#include <memory>

namespace Kolab
{
class MIMEObjectPrivate;

class KOLAB_EXPORT MIMEObject
{
public:
    MIMEObject();
    ~MIMEObject();

    ObjectType parseMessage(const std::string &msg);

    /**
     * Set to override the autodetected object type, before parsing the message.
     */
    void setObjectType(ObjectType);

    /**
     * Set to override the autodetected version, before parsing the message.
     */
    void setVersion(Version);

    /**
     * Returns the Object type of the parsed kolab object.
     */
    ObjectType getType() const;

    /**
     * Returns the kolab-format version of the parsed kolab object.
     */
    Version getVersion() const;

    Kolab::Event getEvent() const;
    Kolab::Todo getTodo() const;
    Kolab::Journal getJournal() const;
    Kolab::Note getNote() const;
    Kolab::Contact getContact() const;
    Kolab::DistList getDistlist() const;
    Kolab::Freebusy getFreebusy() const;
    Kolab::Configuration getConfiguration() const;

    std::string writeEvent(const Kolab::Event &event, Version version, const std::string &productId = std::string());
    Kolab::Event readEvent(const std::string &s);

    std::string writeTodo(const Kolab::Todo &todo, Version version, const std::string &productId = std::string());
    Kolab::Todo readTodo(const std::string &s);

    std::string writeJournal(const Kolab::Journal &journal, Version version, const std::string &productId = std::string());
    Kolab::Journal readJournal(const std::string &s);

    std::string writeNote(const Kolab::Note &note, Version version, const std::string &productId = std::string());
    Kolab::Note readNote(const std::string &s);

    std::string writeContact(const Kolab::Contact &contact, Version version, const std::string &productId = std::string());
    Kolab::Contact readContact(const std::string &s);

    std::string writeDistlist(const Kolab::DistList &distlist, Version version, const std::string &productId = std::string());
    Kolab::DistList readDistlist(const std::string &s);

    std::string writeFreebusy(const Kolab::Freebusy &freebusy, Version version, const std::string &productId = std::string());
    Kolab::Freebusy readFreebusy(const std::string &s);

    std::string writeConfiguration(const Kolab::Configuration &freebusy, Version version, const std::string &productId = std::string());
    Kolab::Configuration readConfiguration(const std::string &s);

private:
    //@cond PRIVATE
    MIMEObject(const MIMEObject &other) = delete;
    MIMEObject &operator=(const MIMEObject &rhs) = delete;

private:
    std::unique_ptr<MIMEObjectPrivate> const d;
    //@endcond
};
}
