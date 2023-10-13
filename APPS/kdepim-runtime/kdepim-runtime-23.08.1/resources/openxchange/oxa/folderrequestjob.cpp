/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "folderrequestjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "folderutils.h"
#include "oxutils.h"

#include <KIO/DavJob>
#include <QDomDocument>

using namespace OXA;

FolderRequestJob::FolderRequestJob(const Folder &folder, QObject *parent)
    : KJob(parent)
    , mFolder(folder)
{
}

void FolderRequestJob::start()
{
    QDomDocument document;
    QDomElement multistatus = DAVUtils::addDavElement(document, document, QStringLiteral("multistatus"));
    QDomElement prop = DAVUtils::addDavElement(document, multistatus, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("object_id"), OXUtils::writeNumber(mFolder.objectId()));

    const QString path = QStringLiteral("/servlet/webdav.folders");

    KIO::DavJob *job = DavManager::self()->createFindJob(path, document);
    connect(job, &KIO::DavJob::result, this, &FolderRequestJob::davJobFinished);
}

Folder FolderRequestJob::folder() const
{
    return mFolder;
}

void FolderRequestJob::davJobFinished(KJob *job)
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
    mFolder = FolderUtils::parseFolder(prop);

    emitResult();
}
