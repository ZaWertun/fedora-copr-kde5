/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

#include "folder.h"

namespace OXA
{
/**
 * @short A job that requests the delta for folders changes from the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FoldersRequestDeltaJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new folders request delta job.
     *
     * @param lastSync The timestamp of the last sync. Only added, modified and deleted folders
     *                 after this date will be requested. 0 will request all available folders.
     * @param parent The parent object.
     */
    explicit FoldersRequestDeltaJob(qulonglong lastSync, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the list of all added and modified folders.
     */
    Q_REQUIRED_RESULT Folder::List modifiedFolders() const;

    /**
     * Returns the list of all deleted folders.
     */
    Q_REQUIRED_RESULT Folder::List deletedFolders() const;

private:
    void fetchModifiedJobFinished(KJob *);
    void fetchDeletedJobFinished(KJob *);
    const qulonglong mLastSync;
    Folder::List mModifiedFolders;
    Folder::List mDeletedFolders;
    int mJobFinishedCount = 0;
};
}
