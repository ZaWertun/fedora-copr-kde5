/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectrequestjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "objectutils.h"
#include "oxutils.h"

#include <KIO/DavJob>
#include <QDomDocument>

using namespace OXA;

ObjectRequestJob::ObjectRequestJob(const Object &object, QObject *parent)
    : KJob(parent)
    , mObject(object)
{
}

void ObjectRequestJob::start()
{
    QDomDocument document;
    QDomElement multistatus = DAVUtils::addDavElement(document, document, QStringLiteral("multistatus"));
    QDomElement prop = DAVUtils::addDavElement(document, multistatus, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("object_id"), OXUtils::writeNumber(mObject.objectId()));

    const QString path = ObjectUtils::davPath(mObject.module());

    KIO::DavJob *job = DavManager::self()->createFindJob(path, document);
    connect(job, &KIO::DavJob::result, this, &ObjectRequestJob::davJobFinished);
}

Object ObjectRequestJob::object() const
{
    return mObject;
}

void ObjectRequestJob::davJobFinished(KJob *job)
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

    QDomElement multistatus = document.documentElement();
    QDomElement response = multistatus.firstChildElement(QStringLiteral("response"));
    const QDomNodeList props = response.elementsByTagName(QStringLiteral("prop"));
    const QDomElement prop = props.at(0).toElement();
    mObject = ObjectUtils::parseObject(prop, mObject.module());

    emitResult();
}
