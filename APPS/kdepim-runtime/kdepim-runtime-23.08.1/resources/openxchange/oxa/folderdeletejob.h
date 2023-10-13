/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "folder.h"

#include <KJob>

namespace OXA
{
/**
 * @short A job that deletes a folder on the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FolderDeleteJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new folder delete job.
     *
     * @param folder The folder to delete.
     * @param parent The parent object.
     *
     * @note The folder needs the objectId, folderId and lastModified property set.
     */
    explicit FolderDeleteJob(const Folder &folder, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

private:
    void davJobFinished(KJob *);
    const Folder mFolder;
};
}
