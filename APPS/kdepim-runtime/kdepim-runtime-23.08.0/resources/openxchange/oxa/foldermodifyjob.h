/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

#include "folder.h"

namespace OXA
{
/**
 * @short A job that modifies a folder on the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FolderModifyJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new folder modify job.
     *
     * @param folder The folder to modify.
     * @param parent The parent object.
     *
     * @note The folder needs at least the objectId and lastModified property set.
     */
    explicit FolderModifyJob(const Folder &folder, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the updated folder that has been modified.
     */
    Q_REQUIRED_RESULT Folder folder() const;

private:
    void davJobFinished(KJob *);
    Folder mFolder;
};
}
