/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "foldermodifyjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "oxutils.h"

#include <KIO/DavJob>
#include <QDomDocument>

using namespace OXA;

FolderModifyJob::FolderModifyJob(const Folder &folder, QObject *parent)
    : KJob(parent)
    , mFolder(folder)
{
}

void FolderModifyJob::start()
{
    QDomDocument document;
    QDomElement propertyupdate = DAVUtils::addDavElement(document, document, QStringLiteral("propertyupdate"));
    QDomElement set = DAVUtils::addDavElement(document, propertyupdate, QStringLiteral("set"));
    QDomElement prop = DAVUtils::addDavElement(document, set, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("title"), OXUtils::writeString(mFolder.title()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("object_id"), OXUtils::writeNumber(mFolder.objectId()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("folder_id"), OXUtils::writeNumber(mFolder.folderId()));
    DAVUtils::addOxElement(document, prop, QStringLiteral("last_modified"), OXUtils::writeString(mFolder.lastModified()));

    const QString path = QStringLiteral("/servlet/webdav.folders");

    KIO::DavJob *job = DavManager::self()->createPatchJob(path, document);
    connect(job, &KIO::DavJob::result, this, &FolderModifyJob::davJobFinished);
}

Folder FolderModifyJob::folder() const
{
    return mFolder;
}

void FolderModifyJob::davJobFinished(KJob *job)
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
        if (element.tagName() == QLatin1String("last_modified")) {
            mFolder.setLastModified(OXUtils::readString(element.text()));
        }

        element = element.nextSiblingElement();
    }

    emitResult();
}
