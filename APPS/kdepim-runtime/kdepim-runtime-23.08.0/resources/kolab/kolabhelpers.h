/*
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "pimkolab/kolabformat/formathelpers.h" //libkolab
#include "pimkolab/kolabformat/kolabdefinitions.h" //libkolab
#include <Akonadi/Item>

#define KOLAB_COLOR_ANNOTATION "/vendor/kolab/color"

class QColor;

class KolabHelpers
{
public:
    static bool checkForErrors(const Akonadi::Item &affectedItem);
    static Akonadi::Item translateFromImap(Kolab::FolderType folderType, const Akonadi::Item &item, bool &ok);
    static Akonadi::Item::List translateToImap(const Akonadi::Item::List &items, bool &ok);
    static Akonadi::Item translateToImap(const Akonadi::Item &item, bool &ok);
    static Kolab::FolderType folderTypeFromString(const QByteArray &folderTypeName);
    static QByteArray getFolderTypeAnnotation(const QMap<QByteArray, QByteArray> &annotations);
    static void setFolderTypeAnnotation(QMap<QByteArray, QByteArray> &annotations, const QByteArray &value);
    static QColor getFolderColor(const QMap<QByteArray, QByteArray> &annotations);
    static void setFolderColor(QMap<QByteArray, QByteArray> &annotations, const QColor &color);
    static Kolab::ObjectType getKolabTypeFromMimeType(const QString &type);
    static QByteArray kolabTypeForMimeType(const QStringList &contentMimeTypes);
    static QStringList getContentMimeTypes(Kolab::FolderType type);
    static QString getIcon(Kolab::FolderType type);
    // Returns true if the folder type shouldn't be ignored
    static bool isHandledType(Kolab::FolderType type);
    static QString getMimeType(Kolab::FolderType type);
    static QList<QByteArray> ancestorChain(const Akonadi::Collection &col);
    static QString createMemberUrl(const Akonadi::Item &item, const QString &user);
};
