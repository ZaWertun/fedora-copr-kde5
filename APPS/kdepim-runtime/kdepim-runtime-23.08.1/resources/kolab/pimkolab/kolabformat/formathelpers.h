/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <kolab_export.h>
#include <string>

namespace Kolab
{
enum FolderType {
    MailType = 0,
    ContactType,
    EventType,
    TaskType,
    JournalType,
    NoteType,
    ConfigurationType,
    FreebusyType,
    FileType,
    LastType,
};

/**
 * Returns the FolderType from a KOLAB_FOLDER_TYPE_* folder type string
 */
KOLAB_EXPORT FolderType folderTypeFromString(const std::string &folderTypeName);
/**
 * Returns the annotation string for a folder
 */
KOLAB_EXPORT std::string folderAnnotation(FolderType type, bool isDefault = false);
/**
 * Guesses the folder type from a user visible string
 */
KOLAB_EXPORT FolderType guessFolderTypeFromName(const std::string &name);

/**
 * Returns a folder name for a type
 */
KOLAB_EXPORT std::string nameForFolderType(FolderType type);
}
