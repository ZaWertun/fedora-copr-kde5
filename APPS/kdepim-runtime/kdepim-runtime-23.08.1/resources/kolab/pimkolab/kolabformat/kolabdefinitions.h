/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

namespace Kolab
{
#define KOLAB_FOLDER_TYPE_MAIL "mail"
#define KOLAB_FOLDER_TYPE_CONTACT "contact"
#define KOLAB_FOLDER_TYPE_EVENT "event"
#define KOLAB_FOLDER_TYPE_TASK "task"
#define KOLAB_FOLDER_TYPE_JOURNAL "journal"
#define KOLAB_FOLDER_TYPE_NOTE "note"
#define KOLAB_FOLDER_TYPE_CONFIGURATION "configuration"
#define KOLAB_FOLDER_TYPE_FREEBUSY "freebusy"
#define KOLAB_FOLDER_TYPE_FILE "file"

#define KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX ".default"
#define KOLAB_FOLDER_TYPE_DRAFT_SUFFIX ".drafts"
#define KOLAB_FOLDER_TYPE_SENT_SUFFIX ".sentitems"
#define KOLAB_FOLDER_TYPE_OUTBOX_SUFFIX ".outbox"
#define KOLAB_FOLDER_TYPE_TRASH_SUFFIX ".wastebasket"
#define KOLAB_FOLDER_TYPE_JUNK_SUFFIX ".junkemail"
#define KOLAB_FOLDER_TYPE_INBOX_SUFFIX ".inbox"

#define KOLAB_FOLDER_TYPE_ANNOTATION "/vendor/kolab/folder-type"

#define X_KOLAB_TYPE_HEADER "X-Kolab-Type"
#define X_KOLAB_MIME_VERSION_HEADER "X-Kolab-Mime-Version"
#define X_KOLAB_MIME_VERSION_HEADER_COMPAT "X-Kolab-Version"
#define KOLAB_VERSION_V2 QStringLiteral("2.0")
#define KOLAB_VERSION_V3 QStringLiteral("3.0")

#define KOLAB_OBJECT_FILENAME QStringLiteral("kolab.xml")

#define MIME_TYPE_XCAL "application/calendar+xml"
#define MIME_TYPE_XCARD "application/vcard+xml"
#define MIME_TYPE_KOLAB "application/vnd.kolab+xml"

#define KOLAB_TYPE_EVENT "application/x-vnd.kolab.event"
#define KOLAB_TYPE_TASK "application/x-vnd.kolab.task"
#define KOLAB_TYPE_JOURNAL "application/x-vnd.kolab.journal"
#define KOLAB_TYPE_CONTACT "application/x-vnd.kolab.contact"
#define KOLAB_TYPE_DISTLIST_V2 "application/x-vnd.kolab.contact.distlist"
#define KOLAB_TYPE_DISTLIST "application/x-vnd.kolab.distribution-list"
#define KOLAB_TYPE_NOTE "application/x-vnd.kolab.note"
#define KOLAB_TYPE_CONFIGURATION "application/x-vnd.kolab.configuration"
#define KOLAB_TYPE_DICT "application/x-vnd.kolab.configuration.dictionary"
#define KOLAB_TYPE_FREEBUSY "application/x-vnd.kolab.freebusy"
#define KOLAB_TYPE_FILE "application/x-vnd.kolab.file"
#define KOLAB_TYPE_RELATION "application/x-vnd.kolab.configuration.relation"

enum Version {
    KolabV2,
    KolabV3,
};

enum ObjectType {
    InvalidObject,
    EventObject,
    TodoObject,
    JournalObject,
    ContactObject,
    DistlistObject,
    NoteObject,
    DictionaryConfigurationObject,
    FreebusyObject,
    RelationConfigurationObject
};
}
