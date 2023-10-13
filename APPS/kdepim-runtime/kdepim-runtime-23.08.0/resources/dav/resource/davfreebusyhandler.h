/*
    SPDX-FileCopyrightText: 2011 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/FreeBusy>

#include <QMap>
#include <QObject>
#include <QString>

class QDateTime;
class KJob;

/**
 * @short The class that will manage DAV free-busy requests
 */
class DavFreeBusyHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a new DavFreeBusyHandler
     */
    explicit DavFreeBusyHandler(QObject *parent = nullptr);

    /**
     * Checks if the free-busy info for @p email can be handled
     *
     * @param email The email address of the contact.
     */
    void canHandleFreeBusy(const QString &email);

    /**
     * Retrieve the free-busy info for @p email between @p start and @p end
     *
     * @param email The email address to retrieve the free-busy for
     * @param start The start of the free-busy period to report
     * @param end The end of the free-busy period to report
     */
    void retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end);

Q_SIGNALS:
    /**
     * Emitted once we know if the free-busy info for @p email
     * can be handled or not.
     */
    void handlesFreeBusy(const QString &email, bool handles);

    /**
     * Emitted once the free-busy has been retrieved
     */
    void freeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText);

private:
    void onPrincipalSearchJobFinished(KJob *job);
    void onRetrieveFreeBusyJobFinished(KJob *job);
    /**
     * Simple struct to track the state of requests
     */
    struct RequestTracker {
        RequestTracker() = default;

        int handlingJobCount = 0;
        bool handlingJobSuccessful = false;
        int retrievalJobCount = 0;
        bool retrievalJobSuccessful = false;
        QMap<uint, KCalendarCore::FreeBusy::Ptr> resultingFreeBusy;
    };

    QMap<QString, RequestTracker> mRequestsTracker;
    QMap<QString, QStringList> mPrincipalScheduleOutbox;
    uint mNextRequestId = 0;
};
