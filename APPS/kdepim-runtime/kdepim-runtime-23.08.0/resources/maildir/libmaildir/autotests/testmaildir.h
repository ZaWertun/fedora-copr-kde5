/*
  This file is part of the kpimutils library.

  SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class QTemporaryDir;

class MaildirTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void testMaildirInstantiation();
    void testMaildirCreation();
    void testMaildirListing();
    void testMaildirAccess();
    void testMaildirReadHeaders();
    void testMaildirWrite();
    void testMaildirAppend();
    void testMaildirRemoveEntry();
    void testMaildirListSubfolders();
    void testMaildirCreateSubfolder();
    void testMaildirRemoveSubfolder();
    void testMaildirRename();
    void testMaildirMoveTo();
    void testMaildirFlagsReading();
    void testMaildirFlagsWriting_data();
    void testMaildirFlagsWriting();
    void cleanup();

private:
    void fillDirectory(const QString &name, int limit);
    void fillNewDirectory();
    void fillCurrentDirectory();
    void createSubFolders();
    QTemporaryDir *m_temp = nullptr;
};
