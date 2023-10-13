/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "foldercreatejob.h"

#include "davmanager.h"
#include "davutils.h"
#include "folderutils.h"
#include "oxutils.h"

#include <KIO/DavJob>

using namespace OXA;

FolderCreateJob::FolderCreateJob(const Folder &folder, QObject *parent)
    : KJob(parent)
    , mFolder(folder)
{
}

void FolderCreateJob::start()
{
    QDomDocument document;
    QDomElement propertyupdate = DAVUtils::addDavElement(document, document, QStringLiteral("propertyupdate"));
    QDomElement set = DAVUtils::addDavElement(document, propertyupdate, QStringLiteral("set"));
    QDomElement prop = DAVUtils::addDavElement(document, set, QStringLiteral("prop"));

    FolderUtils::addFolderElements(document, prop, mFolder);

    const QString path = QStringLiteral("/servlet/webdav.folders");

    KIO::DavJob *job = DavManager::self()->createPatchJob(path, document);
    connect(job, &KIO::DavJob::result, this, &FolderCreateJob::davJobFinished);
}

Folder FolderCreateJob::folder() const
{
    return mFolder;
}

void FolderCreateJob::davJobFinished(KJob *job)
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

    QDomElement element = prop.firstChildElement();
    while (!element.isNull()) {
        if (element.tagName() == QLatin1String("object_id")) {
            mFolder.setObjectId(OXUtils::readNumber(element.text()));
        } else if (element.tagName() == QLatin1String("last_modified")) {
            mFolder.setLastModified(OXUtils::readString(element.text()));
        }

        element = element.nextSiblingElement();
    }

    emitResult();
}
