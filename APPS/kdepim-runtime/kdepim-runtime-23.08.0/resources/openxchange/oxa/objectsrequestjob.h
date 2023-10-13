/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>

#include "folder.h"
#include "object.h"

namespace OXA
{
class ObjectsRequestJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Describes the mode of the request job.
     */
    enum Mode {
        Modified, ///< Fetches all new and modified objects
        Deleted ///< Fetches all deleted objects
    };

    /**
     * Creates a new objects request job.
     *
     * @param folder The folder the objects shall be request from.
     * @param lastSync The timestamp of the last sync. Only added, modified or deleted objects
     *                 after this date will be requested. 0 will request all available objects.
     * @param mode The mode of objects to request.
     * @param parent The parent object.
     */
    explicit ObjectsRequestJob(const Folder &folder, qulonglong lastSync = 0, Mode mode = Modified, QObject *parent = nullptr);

    void start() override;

    Q_REQUIRED_RESULT Object::List objects() const;

private:
    void davJobFinished(KJob *);
    Folder mFolder;
    qulonglong mLastSync;
    const Mode mMode;
    Object::List mObjects;
};
}
