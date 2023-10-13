/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "formathelpers.h"
#include "kolabdefinitions.h"
#include <KLazyLocalizedString>
#include <KLocalizedString>
namespace Kolab
{
static const struct {
    const char *name;
    const KLazyLocalizedString label;
} folderTypeData[] = {{KOLAB_FOLDER_TYPE_MAIL, KLazyLocalizedString()},
                      {KOLAB_FOLDER_TYPE_CONTACT, kli18n("Contacts")},
                      {KOLAB_FOLDER_TYPE_EVENT, kli18n("Calendar")},
                      {KOLAB_FOLDER_TYPE_TASK, kli18n("Tasks")},
                      {KOLAB_FOLDER_TYPE_JOURNAL, kli18n("Journal")},
                      {KOLAB_FOLDER_TYPE_NOTE, kli18n("Notes")},
                      {KOLAB_FOLDER_TYPE_CONFIGURATION, kli18n("Configuration")},
                      {KOLAB_FOLDER_TYPE_FREEBUSY, kli18n("Freebusy")},
                      {KOLAB_FOLDER_TYPE_FILE, kli18n("Files")}};
static const int numFolderTypeData = sizeof folderTypeData / sizeof *folderTypeData;

std::string folderAnnotation(FolderType type, bool isDefault)
{
    Q_ASSERT(type >= 0 && type < LastType);
    std::string result = folderTypeData[type].name;
    if (isDefault) {
        result += KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX;
    }
    return result;
}

FolderType folderTypeFromString(const std::string &folderTypeName)
{
    if (folderTypeName == KOLAB_FOLDER_TYPE_CONTACT || folderTypeName == KOLAB_FOLDER_TYPE_CONTACT KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return ContactType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_EVENT || folderTypeName == KOLAB_FOLDER_TYPE_EVENT KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return EventType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_TASK || folderTypeName == KOLAB_FOLDER_TYPE_TASK KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return TaskType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_JOURNAL || folderTypeName == KOLAB_FOLDER_TYPE_JOURNAL KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return JournalType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_NOTE || folderTypeName == KOLAB_FOLDER_TYPE_NOTE KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return NoteType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_CONFIGURATION || folderTypeName == KOLAB_FOLDER_TYPE_CONFIGURATION KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return ConfigurationType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_FREEBUSY || folderTypeName == KOLAB_FOLDER_TYPE_FREEBUSY KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return FreebusyType;
    }

    if (folderTypeName == KOLAB_FOLDER_TYPE_FILE || folderTypeName == KOLAB_FOLDER_TYPE_FILE KOLAB_FOLDER_TYPE_DEFAULT_SUFFIX) {
        return FileType;
    }

    return MailType;
}

FolderType guessFolderTypeFromName(const std::string &name)
{
    for (int i = 0; i < numFolderTypeData; ++i) {
        if (name == folderTypeData[i].label.toString().toStdString() || name == folderTypeData[i].label.untranslatedText()) {
            return static_cast<FolderType>(i);
        }
    }
    return MailType;
}

std::string nameForFolderType(FolderType type)
{
    Q_ASSERT(type >= 0 && type < LastType);
    return folderTypeData[type].label.toString().toStdString();
}
}
