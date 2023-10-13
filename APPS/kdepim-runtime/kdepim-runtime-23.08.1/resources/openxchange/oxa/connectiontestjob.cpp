/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "connectiontestjob.h"

#include <KIO/Job>

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

using namespace OXA;

ConnectionTestJob::ConnectionTestJob(const QString &url, const QString &user, const QString &password, QObject *parent)
    : KJob(parent)
    , mUrl(url)
    , mUser(user)
    , mPassword(password)
{
}

void ConnectionTestJob::start()
{
    if (!mUrl.startsWith(QLatin1String("https://"))) {
        mUrl = QLatin1String("https://") + mUrl;
    }
    const QUrl url(mUrl + QStringLiteral("/ajax/login?action=login&name=%1&password=%2").arg(mUser, mPassword));

    KJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    connect(job, &KJob::result, this, &ConnectionTestJob::httpJobFinished);
}

void ConnectionTestJob::httpJobFinished(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    auto transferJob = qobject_cast<KIO::StoredTransferJob *>(job);
    Q_ASSERT(transferJob);

    const QJsonObject data = QJsonDocument::fromJson(transferJob->data()).object();

    // on success data contains something like: {"session":"e530578bca504aa89738fadde9e44b3d","random":"ac9090d2cc284fed926fa3c7e316c43b"}
    // on failure data contains something like: {"category":1,"error_params":[],"error":"Invalid credentials.","error_id":"-1529642166-37","code":"LGI-0006"}
    if (data.contains(QLatin1String("error"))) {
        const QString errorText = data[QStringLiteral("error")].toString();

        setError(UserDefinedError);
        setErrorText(errorText);
        emitResult();
        return;
    } else { // success case
        const QString sessionId = data[QStringLiteral("session")].toString();

        // logout correctly...
        const QUrl url(mUrl + QStringLiteral("/ajax/login?action=logout&session=%1").arg(sessionId));
        KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    }

    emitResult();
}
