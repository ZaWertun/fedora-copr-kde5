/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "faketransferjob.h"

#include <QUrl>

#include <KIO/TransferJob>

QQueue<FakeTransferJob::Verifier> FakeTransferJob::mVerifierQueue;

FakeTransferJob::FakeTransferJob(const QByteArray &postData, const VerifierFn &fn, QObject *parent)
    : KIO::SpecialJob(QUrl(QStringLiteral("file:///tmp/")), QByteArray())
    , mPostData(postData)
    , mVerifier(fn)
{
    Q_UNUSED(parent)

    metaObject()->invokeMethod(this, "callVerifier", Qt::QueuedConnection);
}

FakeTransferJob::~FakeTransferJob() = default;

void FakeTransferJob::callVerifier()
{
    mVerifier(this, mPostData);
}

void FakeTransferJob::postResponse(const QByteArray &resp)
{
    mResponse = resp;
    qRegisterMetaType<KIO::Job *>();
    metaObject()->invokeMethod(this, "doData", Qt::QueuedConnection, Q_ARG(QByteArray, mResponse));
    metaObject()->invokeMethod(this, "doEmitResult", Qt::QueuedConnection);
}

void FakeTransferJob::doData(const QByteArray &resp)
{
    Q_EMIT data(this, resp);
}

void FakeTransferJob::doEmitResult()
{
    emitResult();
}

void FakeTransferJob::addVerifier(QObject *obj, const VerifierFn &fn)
{
    Verifier vfy = {obj, fn};
    mVerifierQueue.enqueue(vfy);
}

FakeTransferJob::Verifier FakeTransferJob::getVerifier()
{
    return mVerifierQueue.dequeue();
}
