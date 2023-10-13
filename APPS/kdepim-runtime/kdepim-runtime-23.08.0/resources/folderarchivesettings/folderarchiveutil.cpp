/*
   SPDX-FileCopyrightText: 2013-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "folderarchiveutil.h"

using namespace FolderArchive;

QString FolderArchiveUtil::groupConfigPattern()
{
    return QStringLiteral("FolderArchiveAccount ");
}

QString FolderArchiveUtil::configFileName()
{
    return QStringLiteral("foldermailarchiverc");
}
