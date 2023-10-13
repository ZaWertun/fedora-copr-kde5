/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

#include "object.h"

namespace OXA
{
/**
 * @short A job that requests the delta for objects changes from the OX server.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class ObjectsRequestDeltaJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new objects request delta job.
     *
     * @param folder The folder the objects shall be request from.
     * @param lastSync The timestamp of the last sync. Only added, modified and deleted objects
     *                 after this date will be requested. 0 will request all available objects.
     * @param parent The parent object.
     */
    ObjectsRequestDeltaJob(const Folder &folder, qulonglong lastSync, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Returns the list of all added and modified objects.
     */
    Q_REQUIRED_RESULT Object::List modifiedObjects() const;

    /**
     * Returns the list of all deleted objects.
     */
    Q_REQUIRED_RESULT Object::List deletedObjects() const;

private:
    void fetchModifiedJobFinished(KJob *);
    void fetchDeletedJobFinished(KJob *);
    Folder mFolder;
    qulonglong mLastSync;
    Object::List mModifiedObjects;
    Object::List mDeletedObjects;
    int mJobFinishedCount = 0;
};
}
