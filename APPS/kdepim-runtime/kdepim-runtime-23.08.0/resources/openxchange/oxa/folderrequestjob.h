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
 * @short A job that requests a folder from the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FolderRequestJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new folder request job.
     *
     * @param folder The folder to request.
     * @param parent The parent object.
     *
     * @note The folder needs the objectId property set.
     */
    explicit FolderRequestJob(const Folder &folder, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the requested folder.
     */
    Q_REQUIRED_RESULT Folder folder() const;

private:
    void davJobFinished(KJob *);
    Folder mFolder;
};
}
