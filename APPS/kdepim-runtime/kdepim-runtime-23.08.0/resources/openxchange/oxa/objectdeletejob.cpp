/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectdeletejob.h"

#include "davmanager.h"
#include "davutils.h"
#include "objectutils.h"
#include "oxutils.h"

#include <KIO/DavJob>
#include <QDomDocument>
#include <QDomElement>

using namespace OXA;

ObjectDeleteJob::ObjectDeleteJob(const Object &object, QObject *parent)
    : KJob(parent)
    , mObject(object)
{
}

void ObjectDeleteJob::start()
{
    QDomDocument document;
    QDomElement propertyupdate = DAVUtils::addDavElement(document, document, QStringLiteral("propertyupdate"));
    QDomElement set = DAVUtils::addDavElement(document, propertyupdate, QStringLiteral("set"));
    QDomElement prop = DAVUtils::addDavElement(document, set, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("object_id"), OXUtils::writeNumber(mObject.objectId()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("folder_id"), OXUtils::writeNumber(mObject.folderId()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("method"), OXUtils::writeString(QStringLiteral("DELETE")));
    DAVUtils::addOxElement(document, prop, QStringLiteral("last_modified"), OXUtils::writeString(mObject.lastModified()));

    const QString path = ObjectUtils::davPath(mObject.module());

    KIO::DavJob *job = DavManager::self()->createPatchJob(path, document);
    connect(job, &KIO::DavJob::result, this, &ObjectDeleteJob::davJobFinished);
}

void ObjectDeleteJob::davJobFinished(KJob *job)
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

    QString errorText, errorStatus;
    if (DAVUtils::davErrorOccurred(document, errorText, errorStatus)) {
        setError(UserDefinedError);
        setErrorText(errorText);
        emitResult();
        return;
    }

    emitResult();
}
