/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "foldersrequestjob.h"

#include "davmanager.h"
#include "davutils.h"
#include "folderutils.h"
#include "oxutils.h"
#include <KIO/DavJob>

using namespace OXA;

FoldersRequestJob::FoldersRequestJob(qulonglong lastSync, Mode mode, QObject *parent)
    : KJob(parent)
    , mLastSync(lastSync)
    , mMode(mode)
{
}

void FoldersRequestJob::start()
{
    QDomDocument document;
    QDomElement multistatus = DAVUtils::addDavElement(document, document, QStringLiteral("multistatus"));
    QDomElement prop = DAVUtils::addDavElement(document, multistatus, QStringLiteral("prop"));
    DAVUtils::addOxElement(document, prop, QStringLiteral("lastsync"), OXUtils::writeNumber(mLastSync));
    if (mMode == Modified) {
        DAVUtils::addOxElement(document, prop, QStringLiteral("objectmode"), QStringLiteral("MODIFIED"));
    } else {
        DAVUtils::addOxElement(document, prop, QStringLiteral("objectmode"), QStringLiteral("DELETED"));
    }

    const QString path = QStringLiteral("/servlet/webdav.folders");

    KIO::DavJob *job = DavManager::self()->createFindJob(path, document);
    connect(job, &KIO::DavJob::result, this, &FoldersRequestJob::davJobFinished);
}

Folder::List FoldersRequestJob::folders() const
{
    return mFolders;
}

void FoldersRequestJob::davJobFinished(KJob *job)
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
        mFolders.append(FolderUtils::parseFolder(prop));
        response = response.nextSiblingElement();
    }

    emitResult();
}
