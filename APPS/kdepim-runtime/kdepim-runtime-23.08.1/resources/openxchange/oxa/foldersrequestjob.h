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
 * @short A job that requests all folders from the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FoldersRequestJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Describes the mode of the request job.
     */
    enum Mode {
        Modified, ///< Fetches all new and modified folders
        Deleted ///< Fetches all deleted folders
    };

    /**
     * Creates a new folders request job.
     *
     * @param lastSync The timestamp of the last sync. Only added, modified or deleted folders
     *                 after this date will be requested. 0 will request all available folders.
     * @param mode The mode of folders to request.
     * @param parent The parent object.
     */
    explicit FoldersRequestJob(qulonglong lastSync = 0, Mode mode = Modified, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the list of all requested folders.
     */
    Q_REQUIRED_RESULT Folder::List folders() const;

private:
    void davJobFinished(KJob *);
    const qulonglong mLastSync;
    const Mode mMode;
    Folder::List mFolders;
};
}
