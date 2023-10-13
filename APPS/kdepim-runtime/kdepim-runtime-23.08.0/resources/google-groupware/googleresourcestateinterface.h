/*
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "resourcestateinterface.h"

namespace KGAPI2
{
class Job;
}

/**
 * This is a ResourceStateInterface with some specific for Google Resource bits
 */
class GoogleResourceStateInterface : public ResourceStateInterface
{
public:
    /**
     * Returns whether the resource is operational (i.e. account is configured)
     */
    virtual bool canPerformTask() = 0;

    /**
     * Handles an error (if any) for a job. It includes cancelling a task
     * (if there was an error), or retrying to authenticate (if necessary)
     */
    virtual bool handleError(KGAPI2::Job *job, bool _cancelTask = true) = 0;

    /**
     * Each handler use this to report that it has finished collection fetching
     */
    virtual void collectionsRetrievedFromHandler(const Akonadi::Collection::List &collections) = 0;

    /**
     * FreeBusy
     */
    virtual void freeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText = QString()) = 0;
    virtual void handlesFreeBusy(const QString &email, bool handles) = 0;

    virtual void scheduleCustomTask(QObject *receiver, const char *method, const QVariant &argument) = 0;
};
