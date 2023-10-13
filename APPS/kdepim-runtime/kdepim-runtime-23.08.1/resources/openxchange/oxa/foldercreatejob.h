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
 * @short A job that creates a new folder on the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class FolderCreateJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new folder create job.
     *
     * @param folder The folder to create.
     * @param parent The parent object.
     */
    explicit FolderCreateJob(const Folder &folder, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the updated folder that has been created.
     */
    Folder folder() const;

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    Folder mFolder;
};
}
