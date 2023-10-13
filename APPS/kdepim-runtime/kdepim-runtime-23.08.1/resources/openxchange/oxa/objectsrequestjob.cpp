/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "objectsrequestjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "objectutils.h"
#include "oxutils.h"

#include <KIO/DavJob>
#include <QDomDocument>

using namespace OXA;

ObjectsRequestJob::ObjectsRequestJob(const Folder &folder, qulonglong lastSync, Mode mode, QObject *parent)
    : KJob(parent)
    , mFolder(folder)
    , mLastSync(lastSync)
    , mMode(mode)
{
}

void ObjectsRequestJob::start()
{
    QDomDocument document;
    QDomElement multistatus = DAVUtils::addDavElement(document, document, QStringLiteral("multistatus"));
    QDomElement prop = DAVUtils::addDavElement(document, multistatus, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("folder_id"), OXUtils::writeNumber(mFolder.objectId()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("lastsync"), OXUtils::writeNumber(mLastSync));
    if (mMode == Modified) {
        DAVUtils::addOxElement(document, prop, QStringLiteral("objectmode"), QStringLiteral("MODIFIED"));
    } else {
        DAVUtils::addOxElement(document, prop, QStringLiteral("objectmode"), QStringLiteral("DELETED"));
    }

    const QString path = ObjectUtils::davPath(mFolder.module());

    KIO::DavJob *job = DavManager::self()->createFindJob(path, document);
    connect(job, &KIO::DavJob::result, this, &ObjectsRequestJob::davJobFinished);
}

Object::List ObjectsRequestJob::objects() const
{
    return mObjects;
}

void ObjectsRequestJob::davJobFinished(KJob *job)
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
    while (!response.isNull()) {
        const QDomNodeList props = response.elementsByTagName(QStringLiteral("prop"));
        const QDomElement prop = props.at(0).toElement();
        mObjects.append(ObjectUtils::parseObject(prop, mFolder.module()));
        response = response.nextSiblingElement();
    }

    emitResult();
}
