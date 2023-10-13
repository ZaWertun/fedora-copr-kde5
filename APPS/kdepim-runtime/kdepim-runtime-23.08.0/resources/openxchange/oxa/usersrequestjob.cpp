/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "usersrequestjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "oxutils.h"

#include <KIO/DavJob>

#include <QDomDocument>

using namespace OXA;

UsersRequestJob::UsersRequestJob(QObject *parent)
    : KJob(parent)
{
}

void UsersRequestJob::start()
{
    QDomDocument document;
    QDomElement multistatus = DAVUtils::addDavElement(document, document, QStringLiteral("multistatus"));
    QDomElement prop = DAVUtils::addDavElement(document, multistatus, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("user"), QStringLiteral("*"));

    const QString path = QStringLiteral("/servlet/webdav.groupuser");

    KIO::DavJob *job = DavManager::self()->createFindJob(path, document);
    connect(job, &KIO::DavJob::result, this, &UsersRequestJob::davJobFinished);

    job->start();
}

User::List UsersRequestJob::users() const
{
    return mUsers;
}

void UsersRequestJob::davJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    auto davJob = qobject_cast<KIO::DavJob *>(job);

    const QByteArray ba = davJob->responseData();
    QDomDocument document;
    document.setContent(ba);

    QDomElement multistatus = document.documentElement();
    QDomElement response = multistatus.firstChildElement(QStringLiteral("response"));
    QDomElement propstat = response.firstChildElement(QStringLiteral("propstat"));
    QDomElement prop = propstat.firstChildElement(QStringLiteral("prop"));
    QDomElement users = prop.firstChildElement(QStringLiteral("users"));

    QDomElement userElement = users.firstChildElement(QStringLiteral("user"));
    while (!userElement.isNull()) {
        User user;

        QDomElement element = userElement.firstChildElement();
        while (!element.isNull()) {
            if (element.tagName() == QLatin1String("uid")) {
                user.setUid(OXUtils::readNumber(element.text()));
            } else if (element.tagName() == QLatin1String("email1")) {
                user.setEmail(OXUtils::readString(element.text()));
            } else if (element.tagName() == QLatin1String("displayname")) {
                user.setName(OXUtils::readString(element.text()));
            }

            element = element.nextSiblingElement();
        }

        mUsers.append(user);

        userElement = userElement.nextSiblingElement(QStringLiteral("user"));
    }

    emitResult();
}
