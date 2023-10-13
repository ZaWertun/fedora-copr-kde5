/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QUrl>

namespace KIO
{
class DavJob;
}

class QDomDocument;

namespace OXA
{
/**
 * @short A class that manages DAV specific information.
 *
 * The DavManager stores the base url of the DAV service that
 * shall be accessed and provides factory methods for creating
 * DAV find and patch jobs.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class DavManager
{
public:
    /**
     * Destroys the DAV manager.
     */
    ~DavManager();

    /**
     * Returns the global instance of the DAV manager.
     */
    static DavManager *self();

    /**
     * Sets the base @p url the DAV manager should use.
     */
    void setBaseUrl(const QUrl &url);

    /**
     * Returns the base url the DAV manager uses.
     */
    Q_REQUIRED_RESULT QUrl baseUrl() const;

    /**
     * Returns a new DAV find job.
     *
     * @param path The path that is appended to the base url.
     * @param document The request XML document.
     */
    KIO::DavJob *createFindJob(const QString &path, const QDomDocument &document) const;

    /**
     * Returns a new DAV patch job.
     *
     * @param path The path that is appended to the base url.
     * @param document The request XML document.
     */
    KIO::DavJob *createPatchJob(const QString &path, const QDomDocument &document) const;

private:
    /**
     * Creates a new DAV manager.
     */
    DavManager();

    QUrl mBaseUrl;
    static DavManager *mSelf;
};
}
